#include "hero.hpp"

#include "bit_stream.hpp"
#include "game_mode.hpp"
#include "client_entity_manager.hpp"
#include "entities/food.hpp"

const u32 maxPower = 255000;

std::string _HeroBase::getDisplayName() const
{
    return m_displayName;
}

void _HeroBase::setDisplayName(std::string displayName)
{
    if (displayName.size() > 0xff) {
        displayName.resize(0xff);
    }

    m_displayName = displayName;
}

u32 _HeroBase::getPower() const
{
    return m_power;
}

u8 _HeroBase::getPowerLevel() const
{
    return static_cast<u8>(static_cast<float>(m_power)/1000.f);
}

void _HeroBase::loadFromJson(const rapidjson::Document& doc)
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
    _HeroBase::loadFromJson(doc);
}

void Hero::packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const
{
    Unit::packData(prevEntity, teamId, controlledEntityUniqueId, outPacket);

    BitStream bits;
    const Hero* prevHero = static_cast<const Hero*>(prevEntity);

    bool displayNameChanged = !prevHero || prevHero->getDisplayName() != m_displayName;
    bits.pushBit(displayNameChanged);

    bool powerLevelChanged = !prevHero || prevHero->getPowerLevel() != getPowerLevel();
    bits.pushBit(powerLevelChanged);

    outPacket << bits.popByte();

    if (displayNameChanged) {
        outPacket << (u8) m_displayName.size();

        for (const char& c : m_displayName) {
            outPacket << c;
        }
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

float Hero::getPowerDamageMultiplier() const
{
    //Each power level (1000 power) ~ 10% more = 1.10 multiplier
    return (1.f + static_cast<float>(getPower()) * 0.0001);
}

void Hero::increasePower(u32 amount)
{
    m_power = std::min(maxPower, m_power + amount);

    //Each power level (1000 power) ~ 300 more health
    increaseMaxHealth(static_cast<float>(amount) * 0.3);
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
    _HeroBase::loadFromJson(doc);
}

void C_Hero::loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot)
{
    C_Unit::loadFromData(controlledEntityUniqueId, inPacket, casterSnapshot);

    BitStream bits;

    u8 byte;
    inPacket >> byte;
    bits.pushByte(byte);

    bool displayNameChanged = bits.popBit();
    bool powerLevelChanged = bits.popBit();

    if (displayNameChanged) {
        m_displayName.clear();
        
        u8 size;
        inPacket >> size;

        for (int i = 0; i < size; ++i) {
            char c;
            c >> size;

            m_displayName.push_back(c);
        }
    }

    if (powerLevelChanged) {
        u8 powerLevel;
        inPacket >> powerLevel;

        m_power = static_cast<u32>(powerLevel) * 1000;
    }
}

void C_Hero::insertRenderNode(const C_ManagersContext& managersContext, const Context& context)
{
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
