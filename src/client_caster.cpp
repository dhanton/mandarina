#include "client_caster.hpp"

#include "unit.hpp"
#include "server_entity_manager.hpp"

ClientCaster::ClientCaster(const Context& context):
    InContext(context)
{
    m_caster = nullptr;
}

void ClientCaster::update(sf::Time eTime)
{
    m_casterComponent.update(eTime);
}

void ClientCaster::applyInputs(const PlayerInput& input, const C_ManagersContext& context)
{
    if (!m_caster) return;
    
    //@WIP: Correct the cooldown in client if casting fails on server

    m_casterComponent.C_applyInput(m_caster, input, context);
}

void ClientCaster::setCaster(C_Unit* caster)
{
    if (!caster) return;

    //@WIP: If we want to implement a hero that changes abilities dynamically, we have
    //to send ability type from Server instead of using EntityType like we do here
    //BitStream with (primaryFireTypeChanged, ..., primaryFireCasted)
    //If primaryFireCasted == false we reset the cooldown in Client or something like this
    //probably with better accuracy; not just casted, but also some info about charges remaining, etc

    const Unit* unitData = static_cast<const Unit*>(EntityManager::getEntityData(caster->getEntityType()));

    //copy all abilities from a global EntityData
    m_casterComponent.setPrimaryFire(unitData->getPrimaryFire() ? unitData->getPrimaryFire()->clone() : nullptr);
    m_casterComponent.setSecondaryFire(unitData->getSecondaryFire() ? unitData->getSecondaryFire()->clone() : nullptr);
    m_casterComponent.setAltAbility(unitData->getAltAbility() ? unitData->getAltAbility()->clone() : nullptr);
    m_casterComponent.setUltimate(unitData->getUltimate() ? unitData->getUltimate()->clone() : nullptr);

    m_caster = caster;
}

C_Unit* ClientCaster::getCaster() const
{
    return m_caster;
}

void ClientCaster::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (!m_caster) return;

    //@WIP: Draw a nice cooldown/charges UI
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

u8 ClientCaster::DummyCaster::_casterComponent_weaponId() const
{
    return 0;
}
