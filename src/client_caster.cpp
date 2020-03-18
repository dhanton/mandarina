#include "client_caster.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include "unit.hpp"
#include "server_entity_manager.hpp"

ClientCaster::ClientCaster(const Context& context):
    InContext(context),
    m_secondaryUI(context),
    m_altAbilityUI(context),
    m_ultimateUI(context)
{
    m_caster = nullptr;
}

void ClientCaster::update(sf::Time eTime)
{
    m_casterComponent.update(eTime);

    if (m_casterComponent.isValid()) {
        m_secondaryUI.setPercentage(m_casterComponent.getSecondaryFire()->getPercentage());
        m_altAbilityUI.setPercentage(m_casterComponent.getAltAbility()->getPercentage());
        m_ultimateUI.setPercentage(m_casterComponent.getUltimate()->getPercentage());
    }
}

void ClientCaster::applyInputs(const PlayerInput& input, Vector2& casterPos, const C_ManagersContext& context)
{
    if (!m_caster) return;
    
    //@WIP: Correct the cooldown in client if casting fails on server

    m_casterComponent.C_applyInput(m_caster, casterPos, input, context, false);
}

void ClientCaster::reapplyInputs(const PlayerInput& input, Vector2& casterPos, const C_ManagersContext& context)
{
    if (!m_caster) return;
    
    //call C_applyInput with repeating = true
    m_casterComponent.C_applyInput(m_caster, casterPos, input, context, true);
}

void ClientCaster::setCaster(C_Unit* caster)
{
    if (!caster) return;

    //@WIP: If we want to implement a hero that changes abilities dynamically, we have
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

    m_secondaryUI.setMaxTime(m_casterComponent.getSecondaryFire()->getMaxTime());
    m_altAbilityUI.setMaxTime(m_casterComponent.getAltAbility()->getMaxTime());

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
    m_secondaryUI.setHotkey("LMOUSE");
    m_altAbilityUI.setHotkey("LSHIFT");
    m_ultimateUI.setHotkey("Q");

    m_caster = caster;
}

C_Unit* ClientCaster::getCaster() const
{
    return m_caster;
}

void ClientCaster::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (!m_caster) return;

    sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    target.draw(m_secondaryUI, states);
    target.draw(m_altAbilityUI, states);
    target.draw(m_ultimateUI, states);

    target.setView(previousView);
}

void ClientCaster::DummyCaster::setPrimaryFire(Ability* ability)
{
    m_primaryFire = std::unique_ptr<Ability>(ability);
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
