#include "client_caster.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include "unit.hpp"
#include "server_entity_manager.hpp"
#include "game_mode.hpp"

ClientCaster::ClientCaster(const Context& context):
    InContext(context),
    m_secondaryUI(context),
    m_altAbilityUI(context),
    m_ultimateUI(context)
{
    m_caster = nullptr;
}

void ClientCaster::update(sf::Time eTime, GameMode* gameMode)
{
    m_casterComponent.update(eTime, gameMode);

    if (m_casterComponent.isValid()) {
        m_secondaryUI.setPercentage(m_casterComponent.getSecondaryFire()->getPercentage());
        m_altAbilityUI.setPercentage(m_casterComponent.getAltAbility()->getPercentage());
        m_ultimateUI.setPercentage(m_casterComponent.getUltimate()->getPercentage());
    }
}

void ClientCaster::applyInputs(const PlayerInput& input, Vector2& casterPos, const C_ManagersContext& context)
{
    if (!m_caster) return;
    
    m_casterComponent.C_applyInput(m_caster, casterPos, input, context, false, getExtraFlags());
}

void ClientCaster::reapplyInputs(const PlayerInput& input, Vector2& casterPos, const C_ManagersContext& context)
{
    if (!m_caster) return;
    
    //call C_applyInput with repeating = true
    m_casterComponent.C_applyInput(m_caster, casterPos, input, context, true, getExtraFlags());
}

CasterSnapshot ClientCaster::takeSnapshot() const
{
    CasterSnapshot snapshot;

    if (m_casterComponent.isValid()) {
        snapshot.primaryPercentage = m_casterComponent.getPrimaryFire()->getTotalPercentage();
        snapshot.secondaryPercentage = m_casterComponent.getSecondaryFire()->getTotalPercentage();
        snapshot.altPercentage = m_casterComponent.getAltAbility()->getTotalPercentage();
        snapshot.ultimatePercentage = m_casterComponent.getUltimate()->getTotalPercentage();

    } else {
        snapshot.valid = false;
    }

    return snapshot;
}

void ClientCaster::applyServerCorrection(const CasterSnapshot& diffSnapshot)
{
    if (m_casterComponent.isValid()) {
        if (diffSnapshot.primaryPercentage != 0.f) {
            m_casterComponent.getPrimaryFire()->applyServerCorrection(diffSnapshot.primaryPercentage);
        }

        if (diffSnapshot.secondaryPercentage != 0.f) {
            m_casterComponent.getSecondaryFire()->applyServerCorrection(diffSnapshot.secondaryPercentage);
        }

        if (diffSnapshot.altPercentage != 0.f) {
            m_casterComponent.getAltAbility()->applyServerCorrection(diffSnapshot.altPercentage);
        }

        if (diffSnapshot.ultimatePercentage != 0.f) {
            m_casterComponent.getUltimate()->applyServerCorrection(diffSnapshot.ultimatePercentage);
        }
    }
}

void ClientCaster::setCaster(C_Unit* caster, GameMode* gameMode)
{
    if (!caster) return;

    //@TODO: If we want to implement a hero that changes abilities dynamically, we have
    //to send ability type from Server instead of using EntityType like we do here
    //BitStream with (primaryFireTypeChanged, ..., primaryFireCasted)
    //If primaryFireCasted == false we reset the cooldown in Client or something like this
    //probably with better accuracy; not just casted, but also some info about charges remaining, etc

    //This is needed for non-local connections (maybe there's a better way?)
    EntityManager::loadEntityData(m_context.jsonParser);

    const Unit* unitData = static_cast<const Unit*>(EntityManager::getEntityData(caster->getEntityType()));

    //copy all abilities from a global EntityData
    m_casterComponent.setPrimaryFire(unitData->getPrimaryFire() ? unitData->getPrimaryFire()->clone() : nullptr);
    m_casterComponent.setSecondaryFire(unitData->getSecondaryFire() ? unitData->getSecondaryFire()->clone() : nullptr);
    m_casterComponent.setAltAbility(unitData->getAltAbility() ? unitData->getAltAbility()->clone() : nullptr);
    m_casterComponent.setUltimate(unitData->getUltimate() ? unitData->getUltimate()->clone() : nullptr);

    m_secondaryUI.setTexture(m_casterComponent.getSecondaryFire()->getIconTextureId());
    m_altAbilityUI.setTexture(m_casterComponent.getAltAbility()->getIconTextureId());
    m_ultimateUI.setTexture(m_casterComponent.getUltimate()->getIconTextureId());

    float timeMultiplier = 1.f;

    if (gameMode) {
        timeMultiplier = gameMode->getAbilityTimeMultiplier();
    }

    m_secondaryUI.setMaxTime(m_casterComponent.getSecondaryFire()->getMaxTime() / timeMultiplier);
    m_altAbilityUI.setMaxTime(m_casterComponent.getAltAbility()->getMaxTime() / timeMultiplier);

    //@TODO: Shape Type should depend on the type of the ability
    m_secondaryUI.setShapeType(AbilityUI::BOX);
    m_altAbilityUI.setShapeType(AbilityUI::BOX);
    m_ultimateUI.setShapeType(AbilityUI::CIRCLE);

    //@TODO: Update position when window size changes
    const Vector2u windowSize = m_context.window->getSize();
    const float xPadding = 10.f;
    const float yPadding = 20.f;

    m_secondaryUI.setPosition({15.f * xPadding, windowSize.y - m_secondaryUI.getBoundingSize().y - 2.5f*yPadding});
    m_altAbilityUI.setPosition({m_secondaryUI.getPosition().x + m_secondaryUI.getBoundingSize().x + 2.f*xPadding, m_secondaryUI.getPosition().y});
    m_ultimateUI.setPosition({windowSize.x/2.f - m_ultimateUI.getBoundingSize().x/2.f, windowSize.y - m_ultimateUI.getBoundingSize().y - 2.5f*yPadding});

    //@TODO: Use user-defined hotkeys
    m_secondaryUI.setHotkey("RMOUSE");
    m_altAbilityUI.setHotkey("LSHIFT");
    m_ultimateUI.setHotkey("Q");

    m_caster = caster;
}

void ClientCaster::forceCasterUpdate()
{
    m_caster = nullptr;
}

C_Unit* ClientCaster::getCaster() const
{
    return m_caster;
}

const CooldownAbility* ClientCaster::getPrimaryFire() const
{
    return m_casterComponent.getPrimaryFire();
}

void ClientCaster::setSpectating(bool spectating)
{
    m_spectating = spectating;
}

bool ClientCaster::getSpectating() const
{
    return m_spectating;
}

void ClientCaster::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (!m_caster) return;

    sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    if (m_spectating) {
        //draw SPECTATING text
        const Vector2u windowSize = m_context.window->getSize();

        sf::Text spectatingText;
        spectatingText.setFont(m_context.fonts->getResource("keep_calm_font"));
        spectatingText.setCharacterSize(50);
        spectatingText.setString("SPECTATING");
        spectatingText.setFillColor(sf::Color::White);
        spectatingText.setOutlineColor(sf::Color::Black);
        spectatingText.setOutlineThickness(2.f);
        spectatingText.setOrigin(spectatingText.getLocalBounds().width/2.f + spectatingText.getLocalBounds().left, 
                                spectatingText.getLocalBounds().height/2.f + spectatingText.getLocalBounds().top);

        float yOffset = 1.5f * spectatingText.getLocalBounds().height;
        spectatingText.setPosition({windowSize.x/2.f, windowSize.y - spectatingText.getLocalBounds().height/2.f - yOffset});
        
        target.draw(spectatingText, states);

    } else {
        target.draw(m_secondaryUI, states);
        target.draw(m_altAbilityUI, states);
        target.draw(m_ultimateUI, states);
    }

    target.setView(previousView);
}

CanCast_ExtraFlags ClientCaster::getExtraFlags() const
{
    CanCast_ExtraFlags extraFlags;
    
    if (m_caster) {
        extraFlags.primaryFire = m_caster->getStatus().canAttack();
        extraFlags.secondaryFire = m_caster->getStatus().canCast();
        extraFlags.altAbility = m_caster->getStatus().canCast();
        extraFlags.ultimate = m_caster->getStatus().canCast();
    }

    return extraFlags;
}

void ClientCaster::DummyCaster::setPrimaryFire(CooldownAbility* ability)
{
    m_primaryFire = std::unique_ptr<CooldownAbility>(ability);
}

void ClientCaster::DummyCaster::setSecondaryFire(Ability* ability)
{
    m_secondaryFire = std::unique_ptr<Ability>(ability);
}

void ClientCaster::DummyCaster::setAltAbility(Ability* ability)
{
    m_altAbility = std::unique_ptr<Ability>(ability);
}

void ClientCaster::DummyCaster::setUltimate(Ability* ability)
{
    m_ultimate = std::unique_ptr<Ability>(ability);
}

bool ClientCaster::DummyCaster::isValid() const
{
    return m_primaryFire && m_secondaryFire && m_altAbility && m_ultimate;
}

u8 ClientCaster::DummyCaster::_casterComponent_weaponId() const
{
    return 0;
}
