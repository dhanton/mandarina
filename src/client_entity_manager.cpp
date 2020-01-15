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

    for (int i = 0; i < m_characters.firstInvalidIndex(); ++i) {
        C_TestCharacter& entity = m_characters[i];
        
        // don't interpolate position or aimAngle for the character we're controlling
        //@TODO: Only continue if we can move the character (not rooted or stunned or movingFixed)
        if (entity.uniqueId == m_controlledEntityUniqueId) continue;

        const C_TestCharacter* prevEntity = prevSnapshot->m_characters.atUniqueId(entity.uniqueId);
        const C_TestCharacter* nextEntity = nextSnapshot->m_characters.atUniqueId(entity.uniqueId);

        if (prevEntity && nextEntity) {
            C_TestCharacter_interpolate(entity, prevEntity, nextEntity, elapsedTime, totalTime);
        }

        if (!prevEntity && nextEntity) {
            //This is the first time the entity is being rendered
            //creation callbacks

            //@TODO: Keep track of already sent entities
            //to see which entities are being sent for the first time
            //in the entire game
            //store it as ranges: entities between [1, 130] and [136, 400] were already sent
            //this is probably the most efficient method in memory (sorted range pairs)
            //or just a simple hash table
        }

        if (prevEntity && !nextEntity) {
            //This is the last time entity is being rendered
            //destruction callbacks
        }
    }

    //rest of the interpolations
}

void C_EntityManager::copySnapshotData(const C_EntityManager* snapshot)
{
    //controlledEntity might change
    m_controlledEntityUniqueId = snapshot->m_controlledEntityUniqueId;

    int index = m_characters.getIndexByUniqueId(m_controlledEntityUniqueId);

    //copy only entities that don't exist locally
    for (int i = 0; i < snapshot->m_characters.firstInvalidIndex(); ++i) {
        const C_TestCharacter& snapshotCharacter = snapshot->m_characters[i];

        int index = m_characters.getIndexByUniqueId(snapshotCharacter.uniqueId);

        if (index == -1) {
            int thisIndex = m_characters.addElement(snapshotCharacter.uniqueId);
            m_characters[thisIndex] = snapshotCharacter;
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
    u16 testCharacterNumber;
    inPacket >> testCharacterNumber;

    for (int i = 0; i < testCharacterNumber; ++i) {
        u32 uniqueId;
        inPacket >> uniqueId;

        C_TestCharacter* prevEntity = nullptr;
        
        if (prevSnapshot) {
            prevEntity = prevSnapshot->m_characters.atUniqueId(uniqueId);
        }
        
        //allocate the entity
        int index = m_characters.addElement(uniqueId);

        if (prevEntity) {
            //if the entity existed in previous snapshot, copy it
            m_characters[index] = *prevEntity;

        } else {
            //otherwise initialize it
            C_TestCharacter_init(m_characters[index]);
            m_characters[index].uniqueId = uniqueId;

            //Entity creation callbacks might go here?
            //Or is it better to have them when the entity is rendered for the first time?
        }

        //in both cases it has to be loaded from packet
        C_TestCharacter_loadFromData(m_characters[index], inPacket);       
    }
}

void C_EntityManager::allocateAll()
{
    m_characters.resize(100);
}

void C_EntityManager::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    //We have to sort all objects together by their height (accounting for flying objects)
    //(this has to be done every frame, otherwise the result might not look super good)
    //**vector sorting is faster because of the O(1) access operation**

    //pair of (Sprite, flyingHeight)
    using Node = std::pair<sf::Sprite, float>;

    std::vector<Node> spriteNodes;

    for (int i = 0; i < m_characters.firstInvalidIndex(); ++i) {
        spriteNodes.emplace_back(sf::Sprite(), static_cast<float>(m_characters[i].flyingHeight));

        sf::Sprite& sprite = spriteNodes.back().first;

        sprite.setTexture(m_context.textures->getResource(m_characters[i].textureId));
        sprite.setScale(m_characters[i].scale, m_characters[i].scale);
        sprite.setRotation(m_characters[i].rotation);
        sprite.setPosition(m_characters[i].pos);
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
