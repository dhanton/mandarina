#include "build_version_render.hpp"

#include <SFML/Graphics/Text.hpp>

#if __has_include("game_build_version.h")
#include "game_build_version.h"
#endif

BuildVersionRender::BuildVersionRender(const Context& context):
    InContext(context),
    m_buildVersion("Build ")
{
//game version should be defined in include/game_build_version.h (ignored by git)
//it should contain the result of the commmand 'git describe --tags --always --long'
#ifdef GAME_BUILD_VERSION
    m_buildVersion += GAME_BUILD_VERSION;
#else
    m_buildVersion += "???????";
#endif
}

void BuildVersionRender::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    sf::View previousView = target.getView();
    target.setView(target.getDefaultView());

    sf::Text buildText;
    buildText.setFont(m_context.fonts->getResource("main_font"));
    buildText.setCharacterSize(12);
    buildText.setString(m_buildVersion);
    buildText.setFillColor(sf::Color::Red);
    buildText.setOutlineColor(sf::Color::Black);
    buildText.setOutlineThickness(2.f);
    buildText.setPosition(10.f, 10.f);

    target.draw(buildText, states);
    target.setView(previousView);
}
