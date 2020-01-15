#include "entities.hpp"

#include "texture_ids.hpp"
#include "helper.hpp"
#include "bit_stream.hpp"

bool UnitStatus_equals(const UnitStatus& lhs, const UnitStatus& rhs)
{
    return lhs.stunTime == rhs.stunTime &&
           lhs.rootTime == rhs.rootTime &&
           lhs.disarmTime == rhs.disarmTime &&
           lhs.invisTime == rhs.invisTime &&
           lhs.solid == rhs.solid &&
           lhs.illusion == rhs.illusion;
}

void UnitStatus_packData(const UnitStatus& status, CRCPacket& packet)
{
    BitStream stream;

    stream.pushBit(status.stunTime > sf::Time::Zero);
    stream.pushBit(status.rootTime > sf::Time::Zero);
    stream.pushBit(status.disarmTime > sf::Time::Zero);
    stream.pushBit(status.invisTime > sf::Time::Zero);
    stream.pushBit(status.solid);
    stream.pushBit(status.illusion);

    packet << stream.popByte();
}

void C_UnitStatus_loadFromData(C_UnitStatus& status, CRCPacket& packet)
{
    BitStream stream;

    u8 byte;
    packet >> byte;
    stream.pushByte(byte);

    status.stunned = stream.popBit();
    status.rooted = stream.popBit();
    status.disarmed = stream.popBit();
    status.invisible = stream.popBit();
    status.solid = stream.popBit();
    status.illusion = stream.popBit();
}

void TestCharacter_init(TestCharacter& entity)
{
    //these should get initialized depending on the sub-type

    entity.flyingHeight = 0;
    entity.movementSpeed = 350;
}

void C_TestCharacter_init(C_TestCharacter& entity)
{
    entity.textureId = TextureId::DIABLO;
    entity.scale = 4.f;
    entity.movementSpeed = 350;
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
    outPacket << entity.teamId;
}

void C_TestCharacter_loadFromData(C_TestCharacter& entity, CRCPacket& inPacket)
{
    //@TODO (for actual entities): Use delta encoding with bitflags

    bool posChanged;
    inPacket >> posChanged;

    if (posChanged) {
        inPacket >> entity.pos.x >> entity.pos.y;
    }

    inPacket >> entity.flyingHeight;
    inPacket >> entity.flags;
    inPacket >> entity.teamId;
}

void C_TestCharacter_interpolate(C_TestCharacter& entity, const C_TestCharacter* prev, const C_TestCharacter* next, double t, double d)
{
    entity.pos = Helper_lerpVec2(prev->pos, next->pos, t, d);
}

void TestCharacter_update(TestCharacter& entity, sf::Time eTime)
{
    entity.pos += entity.vel * eTime.asSeconds();
}

void TestCharacter_applyInput(TestCharacter& entity, PlayerInput& input)
{
    //@TODO: Check flags to see if we can actually move

    bool moved = PlayerInput_repeatAppliedInput(input, entity.pos, entity.movementSpeed);

    if (moved) {
        //@TODO: Check collision using EntityManager's QuadTree
    }
}

void C_TestCharacter_applyInput(C_TestCharacter& entity, PlayerInput& input, sf::Time dt)
{
    //@TODO: Check flags to see if we can actually move

    bool moved = PlayerInput_applyInput(input, entity.pos, entity.movementSpeed, dt);    

    if (moved) {
        //@TODO: Check collision using C_EntityManager's bucket
    }
}
