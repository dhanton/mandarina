#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <steam/steamnetworkingtypes.h>
#include "defines.hpp"
#include "context.hpp"

class ConnectionStatusRender : public sf::Drawable, public InContext
{
public:
    ConnectionStatusRender(const Context& context);

    void update(const SteamNetworkingQuickConnectionStatus& status);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    FontLoader* m_fonts;
    SteamNetworkingQuickConnectionStatus m_status;
};
