#include "entities.hpp"

#include "entity_manager.hpp"

TestCharacter::TestCharacter()
{
    defaultComponentFlags.render = true;
}

void TestCharacter::asignDefaultValues(EntityManager* manager, int entityId)
{
    RenderComponent& renderComp = manager->m_renderComponent[manager->mIds_render[entityId]];

    renderComp.height = 100;
    renderComp.width = 100;

    //do something like this for components that require us to modify callbacks
    // manager->m_onDeathComponent[id].callback = TestCharacter::onDeath;
}
