#include "connection_status_render.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <sstream>

ConnectionStatusRender::ConnectionStatusRender(const Context& context):
    InContext(context)
{
    m_status.m_nPing = 0;
    m_status.m_flInBytesPerSec = 0.f;
    m_status.m_flOutBytesPerSec = 0.f;

    //we could also show this
    // m_status.m_flConnectionQualityLocal;
    // m_status.m_flConnectionQualityRemote;
}

void ConnectionStatusRender::update(const SteamNetworkingQuickConnectionStatus& status)
{
    m_status = status;
}

void ConnectionStatusRender::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    std::ostringstream infoStream;
    infoStream.precision(2);
    infoStream << std::fixed;

    infoStream << "In: " << m_status.m_flInBytesPerSec/1000.f << " KB/s Out: " << m_status.m_flOutBytesPerSec/1000.f << " KB/s \nPing: " << m_status.m_nPing << " ms";

    sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    const Vector2u windowSize = m_context.window->getSize();

    sf::Text basicInfoText;
    basicInfoText.setFont(m_context.fonts->getResource("keep_calm_font"));
    basicInfoText.setCharacterSize(12);
    basicInfoText.setString(infoStream.str());
    basicInfoText.setFillColor(sf::Color::Red);
    basicInfoText.setOutlineColor(sf::Color::Black);
    basicInfoText.setOutlineThickness(2.f);

    Vector2 offset = {-10.f, 10.f};
    basicInfoText.setPosition(Vector2(windowSize.x - basicInfoText.getLocalBounds().width, 0.f) + offset);
    
    target.draw(basicInfoText, states);
    target.setView(previousView);
}
