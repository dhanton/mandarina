#include "buff_holder_component.hpp"

#include "unit.hpp"
#include "ability.hpp"

//all buffs have to be included for loadBuffData to work
#include "buffs/root_buff.hpp"
#include "buffs/reveal_buff.hpp"
#include "buffs/recharge_ability_buff.hpp"
#include "buffs/storm_buff.hpp"
#include "buffs/invis_buff.hpp"
#include "buffs/stun_buff.hpp"
#include "buffs/slow_buff.hpp"
#include "buffs/phased_buff.hpp"
#include "buffs/lifesteal_buff.hpp"

bool BuffHolderComponent::m_buffsLoaded = false;
std::unique_ptr<Buff> BuffHolderComponent::m_buffData[BUFF_MAX_TYPES];

void BuffHolderComponent::loadBuffData(const JsonParser* jsonParser)
{
    if (m_buffsLoaded) return;

    #define DoBuff(class_name, type, json_id) \
        m_buffData[BUFF_##type] = std::unique_ptr<Buff>(new class_name()); \
        m_buffData[BUFF_##type]->loadFromJson(*jsonParser->getDocument(json_id)); \
        m_buffData[BUFF_##type]->setBuffType(BUFF_##type);
    #include "buffs.inc"
    #undef DoBuff

    m_buffsLoaded = true;
}

BuffHolderComponent::BuffHolderComponent(BuffHolderComponent const& other)
{

}

BuffHolderComponent& BuffHolderComponent::operator=(BuffHolderComponent const& other)
{
    return *this;
}

Buff* BuffHolderComponent::addBuff(u8 buffType)
{
    if (!m_buffsLoaded) return nullptr;
    if (buffType == BUFF_NONE || buffType >= BUFF_MAX_TYPES) return nullptr;

    m_buffs.emplace_back(m_buffData[buffType]->clone());

    //safe as long as this class is only inherited by Unit (must be the case)
    m_buffs.back()->setUnit(static_cast<Unit*>(this));

    return m_buffs.back().get();
}

Buff* BuffHolderComponent::addUniqueBuff(u8 buffType)
{
    if (!m_buffsLoaded) return nullptr;
    if (buffType == BUFF_NONE || buffType >= BUFF_MAX_TYPES) return nullptr;
    
    for (const auto& buff : m_buffs) {
        if (buff->getType() == buffType) {
            return nullptr;
        }
    }

    return addBuff(buffType);
}

void BuffHolderComponent::removeUniqueBuff(u8 buffType)
{
    for (const auto& buff : m_buffs) {
        if (buff->getType() == buffType) {
            buff->kill();
            break;
        }
    }
}

#define FOR_ALL_BUFFS(function_string) \
    for (auto& buff : m_buffs) { \
        buff->function_string; \
    }

void BuffHolderComponent::onTakeDamage(u16 damage, Entity* source, u32 uniqueId, u8 teamId)
{
    FOR_ALL_BUFFS(onTakeDamage(damage, source, uniqueId, teamId))
}

void BuffHolderComponent::onDealDamage(u16 damage, Entity* target)
{
    FOR_ALL_BUFFS(onDealDamage(damage, target))
}

void BuffHolderComponent::onBeHealed(u16 amount, Entity* source)
{
    FOR_ALL_BUFFS(onBeHealed(amount, source))
}

void BuffHolderComponent::onHeal(u16 amount, Entity* target)
{
    FOR_ALL_BUFFS(onHeal(amount, target))
}

void BuffHolderComponent::onEntityKill(Entity* target)
{
    FOR_ALL_BUFFS(onEntityKill(target))
}

void BuffHolderComponent::onPreUpdate(sf::Time eTime)
{
    auto it = m_buffs.begin();

    while (it != m_buffs.end()) {
        //update timer
        (*it)->update(eTime);

        if ((*it)->isDead()) {
            it = m_buffs.erase(it);
        } else {
            (*it)->onPreUpdate(eTime);
            it++;
        }
    }
}

void BuffHolderComponent::onUpdate(sf::Time eTime)
{
    FOR_ALL_BUFFS(onUpdate(eTime))
}

void BuffHolderComponent::onDeath(bool& dead)
{
    FOR_ALL_BUFFS(onDeath(dead))
}

void BuffHolderComponent::onGetDamageMultiplier(float& multiplier) const
{
    FOR_ALL_BUFFS(onGetDamageMultiplier(multiplier))
}
void BuffHolderComponent::onMovement()
{
    FOR_ALL_BUFFS(onMovement())
}

void BuffHolderComponent::onPrimaryFireCasted()
{
    FOR_ALL_BUFFS(onPrimaryFireCasted())
}

void BuffHolderComponent::onSecondaryFireCasted()
{
    FOR_ALL_BUFFS(onSecondaryFireCasted())
}

void BuffHolderComponent::onAltAbilityCasted()
{
    FOR_ALL_BUFFS(onAltAbilityCasted())
}

void BuffHolderComponent::onUltimateCasted()
{
    FOR_ALL_BUFFS(onUltimateCasted())
}


// void BuffHolderComponent::onAbilityCasted(Ability* ability)
// {
//     FOR_ALL_BUFFS(onAbilityCasted(ability))
// }
