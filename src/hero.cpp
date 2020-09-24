#include "hero.hpp"

#include "bit_stream.hpp"
#include "game_mode.hpp"
#include "client_entity_manager.hpp"
#include "entities/food.hpp"

constexpr u32 maxPower = 255000;
const size_t HeroBase::maxDisplayNameSize = 32;

std::string HeroBase::getDisplayName() const
{
    return m_displayName;
}

void HeroBase::setDisplayName(std::string displayName)
{
    if (displayName.size() > maxDisplayNameSize) {
        displayName.resize(maxDisplayNameSize);
    }

    m_displayName = displayName;
}

u32 HeroBase::getPower() const
{
    return m_power;
}

u8 HeroBase::getPowerLevel() const
{
    return static_cast<u8>(static_cast<float>(m_power)/1000.f);
}

void HeroBase::loadFromJson(const rapidjson::Document& doc)
{
    m_power = 0;
}

Hero* Hero::clone() const
{
    return new Hero(*this);
}

void Hero::loadFromJson(const rapidjson::Document& doc)
{
    Unit::loadFromJson(doc);
    HeroBase::loadFromJson(doc);

    m_powerHealthMultiplier = doc["health_per_level"].GetFloat()/1000.f;
    m_powerDamageMultiplier = doc["damage_multiplier_per_level"].GetFloat()/1000.f;
}

void Hero::packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const
{
    Unit::packData(prevEntity, teamId, controlledEntityUniqueId, outPacket);

    const Hero* prevHero = static_cast<const Hero*>(prevEntity);

    bool displayNameChanged = !prevHero || prevHero->getDisplayName() != m_displayName;
    outPacket << displayNameChanged;

    bool powerLevelChanged = !prevHero || prevHero->getPowerLevel() != getPowerLevel();
    outPacket << powerLevelChanged;

    if (displayNameChanged) {
        outPacket << m_displayName;
    }

    if (powerLevelChanged) {
        outPacket << getPowerLevel();
    }
}

void Hero::onDeath(bool& dead, const ManagersContext& context)
{
    Unit::onDeath(dead, context);

    if (dead) {
        if (context.gameMode) {
            context.gameMode->onHeroDeath(this, m_dead);
        }

        FoodBase::scatterFood(getPosition(), m_consumedFood, context);
    }
}

float Hero::getDamageMultiplier() const
{
    float damageMultiplier = Unit::getDamageMultiplier();

    return (damageMultiplier + static_cast<float>(getPower()) * m_powerDamageMultiplier);
}

void Hero::increasePower(u32 amount)
{
    m_power = std::min(maxPower, m_power + amount);

    increaseMaxHealth(static_cast<float>(amount) * m_powerHealthMultiplier);
}

void Hero::consumeFood(u8 foodType)
{
    m_consumedFood.push_back(foodType);

    increasePower(FoodBase::getPowerGiven(foodType));
}

C_Hero* C_Hero::clone() const
{
    return new C_Hero(*this);
}

void C_Hero::loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context)
{
    C_Unit::loadFromJson(doc, textureId, context);
    HeroBase::loadFromJson(doc);
}

void C_Hero::loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot)
{
    C_Unit::loadFromData(controlledEntityUniqueId, inPacket, casterSnapshot);

    bool displayNameChanged;
    bool powerLevelChanged;

    inPacket >> displayNameChanged;
    inPacket >> powerLevelChanged;

    if (displayNameChanged) {
        m_displayName.clear();

        inPacket >> m_displayName;
    }

    if (powerLevelChanged) {
        u8 powerLevel;
        inPacket >> powerLevel;

        m_power = static_cast<u32>(powerLevel) * 1000;
    }
}

void C_Hero::insertRenderNode(const C_ManagersContext& managersContext, const Context& context)
{
#ifdef MANDARINA_DEBUG
    bool renderingLocallyHidden = managersContext.entityManager->renderingLocallyHidden;
    if (!renderingLocallyHidden && isLocallyHidden() && !isServerRevealed()) return;
#else
    if (isLocallyHidden() && !isServerRevealed()) return;
#endif

    C_Unit::insertRenderNode(managersContext, context);

    std::vector<RenderNode>& uiRenderNodes = managersContext.entityManager->getUIRenderNodes();

    //add unit UI
    if (!m_ui.getHero()) {
        m_ui.setHero(this);
        m_ui.setFonts(context.fonts);
        m_ui.setTextureLoader(context.textures);
    }

    m_ui.setIsControlledEntity(m_uniqueId == managersContext.entityManager->getControlledEntityUniqueId());

    uiRenderNodes.emplace_back(m_uniqueId);
    uiRenderNodes.back().usingSprite = false;
    uiRenderNodes.back().drawable = &m_ui;
    uiRenderNodes.back().height = getPosition().y;
    uiRenderNodes.back().manualFilter = 10;
}

void C_Hero::_doSnapshotCopy(const C_Entity* snapshotEntity)
{
    HeroUI heroUI = m_ui;

    //this method is needed to cast to C_Hero here (and copy data that is in Hero but not in Unit)
    *this = *(static_cast<const C_Hero*>(snapshotEntity));

    m_ui = heroUI;
}
