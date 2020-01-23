#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include "defines.hpp"
#include "player_input.hpp"

class Camera
{
public:
    struct Transition {
        Vector2 endPos;
        sf::Time time;
        bool forceFinish = false;
    };
    
public:
    Camera();

    void renderUpdate(sf::Time eTime);
    void handleInput(const sf::Event& event);
    void changeState();

    void snapInstant(const Vector2& pos);
    void snapSmooth(const Vector2& pos, const sf::Time time, bool forceFinish = false);
    
    Vector2 mapPixelToCoords(const Vector2i& pixel) const;

    void setMapSize(const Vector2u& pos);

    //view has to be set up for renderUpdate to work
    void setView(sf::View* view);

    float getZoom() const;
    bool isFreeView() const;

private:
    Vector2 _clamp(const Vector2& pos) const;

    bool m_freeView;
    PlayerInput m_freeInput;

    sf::View* m_view;
    Vector2u m_mapSize;

    float m_currentZoom;
    float m_initialZoom;

    Transition m_currentTransition;
    Vector2 m_prevPos;

    sf::Time m_timer;
};