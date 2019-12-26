#include "entities.hpp"

// #include "entity_manager.hpp"

int _lastTypeId = 0;

////////////////////////////////////////////////////////////////////////////////

int TestCharacter::typeId = _lastTypeId++;

void TestCharacter::setupComponentFlags(ComponentFlags& flags)
{
    flags.render = true;
}

void TestCharacter::assignDefaultValues(EntityManager* manager, int entityId)
{
    // RenderComponent& renderComp = manager->m_renderComponent[manager->mIds_render[entityId]];

    // renderComp.textureId = "test_character";
    // renderComp.scale = 1.5f;

    //do something like this for components that require us to modify callbacks
    // manager->m_onDeathComponent[id].callback = TestCharacter::onDeath;
}
