#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/System/Time.hpp>

#include "defines.hpp"
#include "crcpacket.hpp"

struct PlayerInput
{
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    float aimAngle = 0.f;

    u16 id = 0;

    //amount of time this input has been applied
    sf::Time timeApplied;
};

void PlayerInput_packData(PlayerInput& playerInput, CRCPacket& outPacket);
void PlayerInput_loadFromData(PlayerInput& playerInput, CRCPacket& inPacket);

void PlayerInput_handleKeyboardInput(PlayerInput& playerInput, const sf::Event& event);

std::string PlayerInput_toString(const PlayerInput& input);

bool PlayerInput_applyInput(PlayerInput& input, Vector2& pos, float movementSpeed, sf::Time dt);

//repeat the input using dt = timeApplied and without modyfing it
bool PlayerInput_repeatAppliedInput(const PlayerInput& input, Vector2& pos, float movementSpeed);
