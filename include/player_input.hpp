#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/System/Time.hpp>

#include "defines.hpp"
#include "crcpacket.hpp"

class Status;

struct PlayerInput
{
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    bool primaryFire = false;
    bool secondaryFire = false;
    bool altAbility = false;
    bool ultimate = false;

    float aimAngle = 0.f;

    u32 id = 0;

    //amount of time this input has been applied
    sf::Time timeApplied;
};

void PlayerInput_packData(PlayerInput& playerInput, CRCPacket& outPacket, const Status& status);
void PlayerInput_loadFromData(PlayerInput& playerInput, CRCPacket& inPacket);

void PlayerInput_handleInput(PlayerInput& playerInput, const sf::Event& event);
void PlayerInput_clearKeys(PlayerInput& playerInput);

std::string PlayerInput_toString(const PlayerInput& input);

void PlayerInput_updateAimAngle(PlayerInput& input, const Vector2& unitPos, const Vector2& mousePos);

bool PlayerInput_applyInput(PlayerInput& input, Vector2& pos, float movementSpeed, sf::Time dt);

//repeat the input using dt = timeApplied and without modyfing it
bool PlayerInput_repeatAppliedInput(const PlayerInput& input, Vector2& pos, float movementSpeed);
