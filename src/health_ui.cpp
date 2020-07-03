#include "health_ui.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include "entity.hpp"

const Vector2 HealthUI::barSize = Vector2(80.f, 16.f);

float HealthUI::getYOffset(bool isControlledEntity)
{
    return isControlledEntity ? -50.f : -30.f;
}

HealthUI::HealthUI()
{
    m_entity = nullptr;
    m_health = nullptr;
    m_fonts = nullptr;

    m_isControlledEntity = false;
    m_isAlly = false;
}

void HealthUI::setEntity(const C_Entity* entity)
{
    m_entity = entity;
}

const C_Entity* HealthUI::getEntity() const
{
    return m_entity;
}

void HealthUI::setHealthComponent(const HealthComponent* healthComponent)
{
    m_health = healthComponent;
}

void HealthUI::setIsAlly(bool isAlly)
{
    m_isAlly = isAlly;
}

bool HealthUI::getIsAlly() const
{
    return m_isAlly;
}

void HealthUI::setFonts(const FontLoader* fonts)
{
    m_fonts = fonts;
}

void HealthUI::setIsControlledEntity(bool isControlledEntity)
{
    m_isControlledEntity = isControlledEntity;
}

void HealthUI::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    const float yOffset = getYOffset(m_isControlledEntity);

    sf::RectangleShape background;
    background.setSize(barSize);
    background.setPosition(m_entity->getPosition() - background.getSize()/2.f + Vector2(0.f, yOffset - m_entity->getCollisionRadius()));
    background.setFillColor(sf::Color::Black);
    target.draw(background, states);

    const float percentage = (float)m_health->getHealth()/(float)m_health->getMaxHealth();

    if (percentage > 0.05f) {
        const Vector2 padding = Vector2(4.f, 4.f);

        sf::RectangleShape foreground;
        foreground.setSize({percentage * background.getSize().x - padding.x, background.getSize().y - padding.y});
        foreground.setPosition(background.getPosition() + padding/2.f);
        foreground.setFillColor(m_isAlly ? sf::Color::Green : sf::Color::Red);
        target.draw(foreground, states);
    }

    sf::Text healthText;
    healthText.setFont(m_fonts->getResource("keep_calm_font"));
    healthText.setCharacterSize(11);
    healthText.setString(std::to_string(m_health->getMaxHealth()));
    healthText.setFillColor(sf::Color::White);
    healthText.setOrigin(healthText.getLocalBounds().width/2.f + healthText.getLocalBounds().left, 
                         healthText.getLocalBounds().height/2.f + healthText.getLocalBounds().top);
    healthText.setOutlineColor(sf::Color::Black);
    healthText.setOutlineThickness(1.f);
    healthText.setPosition(background.getPosition() + Vector2(background.getSize().x/2.f, 0.f));
    target.draw(healthText, states);
}
