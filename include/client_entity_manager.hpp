#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

#include "bucket.hpp"
#include "entities.hpp"
#include "context.hpp"

class C_EntityManager : public InContext, public sf::Drawable
{
public:
    C_EntityManager(const Context& context, sf::Time worldTime = sf::Time::Zero);

    //used by snapshots
    C_EntityManager();
    
    void update(sf::Time eTime);

    void performInterpolation(const C_EntityManager* prevSnapshot, const C_EntityManager* nextSnapshot, double elapsedTime, double totalTime);
    void copySnapshotData(const C_EntityManager* snapshot);

    void loadFromData(C_EntityManager* prevSnapshot, CRCPacket& inPacket);

    void allocateAll();

public:
    Bucket<C_TestCharacter> m_characters;
    u32 m_controlledEntityUniqueId;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;


};
