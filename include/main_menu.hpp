#pragma once

#include <SFML/Graphics/Drawable.hpp>

#include "context.hpp"
#include "game_client.hpp"
#include "hero.hpp"

class MainMenu : public InContext, public sf::Drawable
{
public:
    MainMenu(const Context& context);

    void mainLoop(bool& running);

    void startGame();
    void stopGame();

    void update(sf::Time eTime);
    void handleInput(const sf::Event& event, bool focused);

    void loadFromJson(const rapidjson::Document& doc);

    void readDisplayName();

private:
    void draw(sf::RenderTarget& target, sf::RenderStates tates) const;

    void _doUpdate(sf::Time eTime);
    void _doHandleInput(const sf::Event& event);
    void _doDraw(sf::RenderTarget& target, sf::RenderStates states) const;

    void resetHeroSelection();

    std::unique_ptr<GameClient> m_gameClient;

    std::unique_ptr<sf::RenderWindow> m_window;
    sf::View m_view;

    bool m_stopRunning;

    SteamNetworkingIPAddr m_endpoint;

    sf::Time m_updateRate;
    sf::Time m_renderRate;
    sf::Time m_inputRate;
    Vector2u m_screenSize;
    u32      m_screenStyle;

    std::string m_displayName;

    bool m_selectedHeroes[g_numberOfHeroes];
    sf::FloatRect m_heroButtons[g_numberOfHeroes];
    sf::FloatRect m_playButton;

    static const sf::Color m_backgroundColor;
    static const sf::Color m_buttonColor;
    static const sf::Color m_hoveringColor;
    static const sf::Color m_selectedColor;
};
