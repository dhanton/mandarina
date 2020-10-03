#include "hero_ui.hpp"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include "health_ui.hpp"
#include "hero.hpp"

HeroUI::HeroUI()
{
    m_hero = nullptr;
    m_fonts = nullptr;
    m_textures = nullptr;
}

void HeroUI::setHero(const C_Hero* hero)
{
    m_hero = hero;
}

const C_Hero* HeroUI::getHero() const
{
    return m_hero;
}

void HeroUI::setFonts(const FontLoader* fonts)
{
    m_fonts = fonts;
}

void HeroUI::setTextureLoader(const TextureLoader* textures)
{
    m_textures = textures;
}

void HeroUI::setIsControlledEntity(bool isControlledEntity)
{
    m_isControlledEntity = isControlledEntity;
}

void HeroUI::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    const float yOffset = HealthUI::getYOffset(m_isControlledEntity);
    const int powerLevel = m_hero->getPowerLevel();

    const Vector2 backgroundPos = m_hero->getPosition() + Vector2(0.f, yOffset - m_hero->getCollisionRadius());
    const Vector2 pos = backgroundPos - Vector2(HealthUI::barSize.x/2.f + 6.f, 1.f);


    sf::CircleShape powerCircle;
    powerCircle.setRadius(10.f);
    powerCircle.setOrigin(powerCircle.getRadius(), powerCircle.getRadius());
    powerCircle.setPosition(pos);
    powerCircle.setFillColor(sf::Color::Magenta);

    sf::Text powerText;
    powerText.setFont(m_fonts->getResource("main_font"));
    powerText.setCharacterSize(14);
    powerText.setString(std::to_string(powerLevel));
    powerText.setFillColor(sf::Color::White);
    powerText.setOrigin(powerText.getLocalBounds().width/2.f + powerText.getLocalBounds().left, 
                         powerText.getLocalBounds().height/2.f + powerText.getLocalBounds().top);
    powerText.setOutlineColor(sf::Color::Black);
    powerText.setOutlineThickness(1.f);
    powerText.setPosition(powerCircle.getPosition());

    target.draw(powerCircle, states);
    target.draw(powerText, states);

    if (!m_hero->getDisplayName().empty()) {
        sf::Text nameText;
        nameText.setFont(m_fonts->getResource("main_font"));
        nameText.setCharacterSize(10);
        nameText.setString(m_hero->getDisplayName());
        nameText.setFillColor(sf::Color::White);
        nameText.setOrigin(nameText.getLocalBounds().width/2.f + nameText.getLocalBounds().left, 
                            nameText.getLocalBounds().height/2.f + nameText.getLocalBounds().top);
        nameText.setOutlineColor(sf::Color::Black);
        nameText.setOutlineThickness(1.f);
        nameText.setPosition(backgroundPos - Vector2(0.f, nameText.getLocalBounds().height + 10.f));
        
        target.draw(nameText, states);
    }
}
