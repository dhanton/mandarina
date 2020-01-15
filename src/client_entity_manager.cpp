#include "client_entity_manager.hpp"

#include <algorithm>
#include <SFML/Graphics.hpp>

#include "helper.hpp"

C_EntityManager::C_EntityManager(const Context& context, sf::Time worldTime):
    InContext(context)
{
    m_controlledEntityUniqueId = 0;
}

C_EntityManager::C_EntityManager():
    //dummy context used if this instance is a snapshot
    InContext(Context())
{
    m_controlledEntityUniqueId = 0;    
}

void C_EntityManager::update(sf::Time eTime)
{    

}

void C_EntityManager::performInterpolation(const C_EntityManager* prevSnapshot, const C_EntityManager* nextSnapshot, double elapsedTime, double totalTime)
{
    if (!prevSnapshot) {
        std::cout << "performInterpolation error - Previous snapshot doesn't exist" << std::endl;
        return;
    }

    if (!nextSnapshot) {
        std::cout << "performInterpolation error - Next snapshot doesn't exist" << std::endl;
        return;
    }

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        C_Unit& unit = units[i];

        const C_Unit* prevUnit = prevSnapshot->units.atUniqueId(unit.uniqueId);
        const C_Unit* nextUnit = nextSnapshot->units.atUniqueId(unit.uniqueId);

        bool controlled = unit.uniqueId == m_controlledEntityUniqueId;

        C_Unit_interpolate(unit, prevUnit, nextUnit, elapsedTime, totalTime, controlled);
    }

    //rest of the interpolations
}

void C_EntityManager::copySnapshotData(const C_EntityManager* snapshot)
{
    //controlledEntity might change
    m_controlledEntityUniqueId = snapshot->m_controlledEntityUniqueId;

    int index = units.getIndexByUniqueId(m_controlledEntityUniqueId);

    //copy only entities that don't exist locally
    for (int i = 0; i < snapshot->units.firstInvalidIndex(); ++i) {
        const C_Unit& snapshotUnit = snapshot->units[i];

        int index = units.getIndexByUniqueId(snapshotUnit.uniqueId);

        if (index == -1) {
            int thisIndex = units.addElement(snapshotUnit.uniqueId);
            units[thisIndex] = snapshotUnit;
        }
    }
}

//@TODO: Implementation is very similar to method above
//Generalize in one single _copyFromSnapshot_impl method 
//that can take a packet or just the snapshot
void C_EntityManager::loadFromData(C_EntityManager* prevSnapshot, CRCPacket& inPacket)
{
    if (prevSnapshot) {
        m_controlledEntityUniqueId = prevSnapshot->m_controlledEntityUniqueId;
    }

    //number of characters
    u16 unitNumber;
    inPacket >> unitNumber;

    for (int i = 0; i < unitNumber; ++i) {
        u32 uniqueId;
        inPacket >> uniqueId;

        C_Unit* prevUnit = nullptr;
        
        if (prevSnapshot) {
            prevUnit = prevSnapshot->units.atUniqueId(uniqueId);
        }

        //allocate the unit
        int index = units.addElement(uniqueId);

        if (prevUnit) {
            //if the unit existed in previous snapshot, copy it
            units[index] = *prevUnit;

        } else {
            u8 unitType;
            inPacket >> unitType;

            //otherwise initialize it
            C_Unit_init(units[index], (UnitType) unitType);
            units[index].uniqueId = uniqueId;

            //Entity creation callbacks might go here?
            //Or is it better to have them when the entity is rendered for the first time?
        }

        //in both cases it has to be loaded from packet
        C_Unit_loadFromData(units[index], inPacket);     
    }
}

void C_EntityManager::allocateAll()
{
    units.resize(MAX_UNITS);
}

void C_EntityManager::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    //We have to sort all objects together by their height (accounting for flying objects)
    //(this has to be done every frame, otherwise the result might not look super good)
    //**vector sorting is faster because of the O(1) access operation**

    //pair of (Sprite, flyingHeight)
    using Node = std::pair<sf::Sprite, float>;

    std::vector<Node> spriteNodes;

    for (int i = 0; i < units.firstInvalidIndex(); ++i) {
        spriteNodes.emplace_back(sf::Sprite(), static_cast<float>(units[i].flyingHeight));

        sf::Sprite& sprite = spriteNodes.back().first;

        sprite.setTexture(m_context.textures->getResource(units[i].textureId));
        sprite.setScale(units[i].scale, units[i].scale);
        // sprite.setRotation(units[i].rotation);
        sprite.setPosition(units[i].pos);
    }

    //include the rest of the entities in the vector

    //@WIP: Mirror the sprite (x) if the unit is looking in the other direction
    //@WIP: Render weapons as well

    std::sort(spriteNodes.begin(), spriteNodes.end(),
        [] (const Node& lhs, const Node& rhs) {
            bool sameHeight = lhs.first.getPosition().y + lhs.first.getLocalBounds().height - lhs.second < 
                              rhs.first.getPosition().y + rhs.first.getLocalBounds().height - rhs.second;

            //@WIP: Compare uniqueId if both heights are the same
            //to avoid flickering (use struct instead of pair and include u32 uniqueId)

            // if (sameHeight) {
                // return lhs.uniqueId < rhs.uniqueId;
            // }

            return sameHeight;
        });

    for (const auto& node : spriteNodes) {
        target.draw(node.first, states);
    }
}
