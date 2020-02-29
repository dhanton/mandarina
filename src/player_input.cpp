#include "player_input.hpp"

#include <sstream>

#include "bit_stream.hpp"
#include "helper.hpp"
#include "defines.hpp"

void PlayerInput_packData(PlayerInput& playerInput, CRCPacket& outPacket)
{
    BitStream stream;

    stream.pushBit(playerInput.left);
    stream.pushBit(playerInput.right);
    stream.pushBit(playerInput.up);
    stream.pushBit(playerInput.down);

    stream.pushBit(playerInput.primaryFire);
    stream.pushBit(playerInput.secondaryFire);
    stream.pushBit(playerInput.altAbility);
    stream.pushBit(playerInput.ultimate);

    outPacket << playerInput.id;
    outPacket << stream.popByte();
    outPacket << Helper_angleTo16bit(playerInput.aimAngle);
}

void PlayerInput_loadFromData(PlayerInput& playerInput, CRCPacket& inPacket)
{
    inPacket >> playerInput.id;

    BitStream stream;

    u8 byte;
    inPacket >> byte;
    stream.pushByte(byte);

    playerInput.left = stream.popBit();
    playerInput.right = stream.popBit();
    playerInput.up = stream.popBit();
    playerInput.down = stream.popBit();

    playerInput.primaryFire = stream.popBit();
    playerInput.secondaryFire = stream.popBit();
    playerInput.altAbility = stream.popBit();
    playerInput.ultimate = stream.popBit();

    u16 angle16bit;
    inPacket >> angle16bit;

    playerInput.aimAngle = Helper_angleFrom16bit(angle16bit);
}

void PlayerInput_handleInput(PlayerInput& playerInput, const sf::Event& event)
{
    //playerInput contains the result of the previous chain of inputs

    //@TODO: Use player configurable hotkeys (store in a map)

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::A) {
            playerInput.left = true;
        }

        if (event.key.code == sf::Keyboard::D) {
            playerInput.right = true;
        }

        if (event.key.code == sf::Keyboard::W) {
            playerInput.up = true;
        }

        if (event.key.code == sf::Keyboard::S) {
            playerInput.down = true;
        }

        if (event.key.code == sf::Keyboard::LShift) {
            playerInput.altAbility = true;
        }

        if (event.key.code == sf::Keyboard::Q) {
            playerInput.ultimate = true;
        }
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            playerInput.primaryFire = true;
        }

        if (event.mouseButton.button == sf::Mouse::Right) {
            playerInput.secondaryFire = true;
        }
    }

    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::A) {
            playerInput.left = false;
        }

        if (event.key.code == sf::Keyboard::D) {
            playerInput.right = false;
        }
        
        if (event.key.code == sf::Keyboard::W) {
            playerInput.up = false;
        }

        if (event.key.code == sf::Keyboard::S) {
            playerInput.down = false;
        }

        if (event.key.code == sf::Keyboard::LShift) {
            playerInput.altAbility = false;
        }

        if (event.key.code == sf::Keyboard::Q) {
            playerInput.ultimate = false;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            playerInput.primaryFire = false;
        }

        if (event.mouseButton.button == sf::Mouse::Right) {
            playerInput.secondaryFire = false;
        }
    }
}

void PlayerInput_clearKeys(PlayerInput& playerInput)
{
    playerInput.left = false;
    playerInput.right = false;
    playerInput.up = false;
    playerInput.down = false;
}

std::string PlayerInput_toString(const PlayerInput& input)
{
    std::stringstream ss;
    ss << input.left << ' ' << input.right << ' ' << input.up << ' ' << input.down
       << input.primaryFire << ' ' << input.secondaryFire << ' ' << input.altAbility << ' ' << input.ultimate;
   
    return ss.str();
}

void PlayerInput_updateAimAngle(PlayerInput& input, const Vector2& unitPos, const Vector2& mousePos)
{
    Vector2 dir = mousePos - unitPos;
    input.aimAngle = Helper_radToDeg(std::atan2(dir.x, dir.y));
}

bool _applyInput_impl(const PlayerInput& input, Vector2& pos, float movementSpeed, sf::Time dt)
{
    float xSign = 0;
    float ySign = 0;
    float distance = movementSpeed * dt.asSeconds();

    //there is no movement if player is pressing up & down at the same time (or left & right)

    if (!input.left || !input.right) {
        if (input.left) xSign = -1;
        if (input.right) xSign = 1;
    }

    if (!input.up || !input.down) {
        if (input.up) ySign = -1;
        if (input.down) ySign = 1;
    }

    if (xSign != 0 && ySign != 0) {
        distance *= SQRT2_INV;
    }
    
    pos.x += xSign * distance;
    pos.y += ySign * distance;

    //return true if there was movement
    return xSign != 0 || ySign != 0;
}

bool PlayerInput_applyInput(PlayerInput& input, Vector2& pos, float movementSpeed, sf::Time dt)
{
    input.timeApplied += dt;

    return _applyInput_impl(input, pos, movementSpeed, dt);
}

bool PlayerInput_repeatAppliedInput(const PlayerInput& input, Vector2& pos, float movementSpeed)
{
    return _applyInput_impl(input, pos, movementSpeed, input.timeApplied);
}
