#include "entities.hpp"

#include "texture_ids.hpp"
#include "helper.hpp"

void TestCharacter_init(TestCharacter& entity)
{
    //these should get initialized depending on the sub-type

    entity.vel = Vector2(50.f, 50.f);
    entity.flyingHeight = 0;
}

void C_TestCharacter_init(C_TestCharacter& entity)
{
    entity.textureId = TextureId::DIABLO;
    entity.scale = 4.f;
}

void TestCharacter_packData(const TestCharacter& entity, const TestCharacter* prevEntity, CRCPacket& outPacket)
{
    if (prevEntity && entity.pos == prevEntity->pos) {
        outPacket << false;
    } else {
        outPacket << true;
        outPacket << entity.pos.x << entity.pos.y;
    }

    outPacket << entity.flyingHeight;
    outPacket << entity.flags;
}

void C_TestCharacter_loadFromData(C_TestCharacter& entity, CRCPacket& inPacket)
{
    bool posChanged;
    inPacket >> posChanged;

    if (posChanged) {
        inPacket >> entity.pos.x >> entity.pos.y;
    }

    inPacket >> entity.flyingHeight;
    inPacket >> entity.flags;
}

void C_TestCharacter_interpolate(C_TestCharacter& entity, const C_TestCharacter* prev, const C_TestCharacter* next, double t, double d)
{
    entity.pos = dh::lerpVec2(prev->pos, next->pos, t, d);
}

void TestCharacter_update(TestCharacter& entity, sf::Time eTime)
{
    entity.pos += entity.vel * eTime.asSeconds();
}