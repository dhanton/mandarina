#include "buff_holder_component.hpp"

#include "unit.hpp"
#include "ability.hpp"

//all buffs have to be included for loadBuffData to work
#include "buffs/root_buff.hpp"
#include "buffs/reveal_buff.hpp"

bool BuffHolderComponent::m_buffsLoaded = false;
std::unique_ptr<Buff> BuffHolderComponent::m_buffData[BUFF_MAX_TYPES];

void BuffHolderComponent::loadBuffData(const JsonParser* jsonParser)
{
    if (m_buffsLoaded) return;

    #define DoBuff(class_name, type, json_id) \
        m_buffData[BUFF_##type] = std::unique_ptr<Buff>(new class_name()); \
        m_buffData[BUFF_##type]->loadFromJson(*jsonParser->getDocument(json_id));
    #include "buffs.inc"
    #undef DoBuff

    m_buffsLoaded = true;
}

//@WIP: We should copy the buffs in some way or another ?????
//since this is used only the server, and since buffs are not used by snapshot units, 
//are they not needed>???????????

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
    if (buffType >= BUFF_MAX_TYPES) return nullptr;

    //@TODO: We need some sort of setting to remove old buffs with the same name in some cases
    //(look at platano for reference)

    m_buffs.emplace_back(m_buffData[buffType]->clone());

    //safe as long as this class is only inherited by Unit (must be the case)
    m_buffs.back()->setUnit(static_cast<Unit*>(this));

    return m_buffs.back().get();
}

#define FOR_ALL_BUFFS(function_string) \
    for (auto& buff : m_buffs) { \
        buff->function_string; \
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

void BuffHolderComponent::onDeath()
{
    FOR_ALL_BUFFS(onDeath())
}

void BuffHolderComponent::onTakeDamage(u16 damage, Entity* source)
{
    FOR_ALL_BUFFS(onTakeDamage(damage, source))
}

void BuffHolderComponent::onDealDamage(u16 damage, Entity* receiver)
{
    FOR_ALL_BUFFS(onDealDamage(damage, receiver))
}

void BuffHolderComponent::onBeHealed(u16 amount, Entity* source)
{
    FOR_ALL_BUFFS(onBeHealed(amount, source))
}

void BuffHolderComponent::onHeal(u16 amount, Entity* receiver)
{
    FOR_ALL_BUFFS(onHeal(amount, receiver))
}

// void BuffHolderComponent::onAbilityCasted(Ability* ability)
// {
//     FOR_ALL_BUFFS(onAbilityCasted(ability))
// }
