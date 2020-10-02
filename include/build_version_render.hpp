#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include "defines.hpp"
#include "context.hpp"

class BuildVersionRender : public sf::Drawable, public InContext
{
public:
    BuildVersionRender(const Context& context);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    std::string m_buildVersion;
};
