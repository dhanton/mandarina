#include "unit_ui.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "unit.hpp"
#include "client_caster.hpp"
#include "context.hpp"
#include "texture_ids.hpp"

const Vector2 UnitUI::m_barSize = Vector2(80.f, 16.f);

void UnitUI::setUnit(const C_Unit* unit)
{
    m_unit = unit;
}

const C_Unit* UnitUI::getUnit() const
{
    return m_unit;
}

void UnitUI::setClientCaster(const ClientCaster* clientCaster)
{
    m_clientCaster = clientCaster;
}

const ClientCaster* UnitUI::getClientCaster() const
{
    return m_clientCaster;
}

void UnitUI::setIsAlly(bool isAlly)
{
    m_isAlly = isAlly;
}

void UnitUI::setFonts(const FontLoader* fonts)
{
    m_fonts = fonts;
}

void UnitUI::setTextureLoader(const TextureLoader* textures)
{
    m_textures = textures;
}

void UnitUI::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (!m_unit) return;

    const float yOffset = m_clientCaster ? -50.f : -30.f;

    //draw status icons
    sf::Sprite statusSprite;
    float statusXOffset = 5.f;
    statusSprite.setPosition(m_unit->getPosition() - m_barSize/2.f + Vector2(0.f, yOffset - 30.f - m_unit->getCollisionRadius()));

    //@TODO: This code is not flexible (hard to add new status)
    //It's also not done in chronological order, which could be confusing
    if (m_unit->getStatus().stunned) {
        statusSprite.setTexture(m_textures->getResource(TextureId::STUNNED));
        target.draw(statusSprite, states);
    }

    if (m_unit->getStatus().silenced) {
        statusSprite.setPosition(statusSprite.getPosition() + Vector2(statusSprite.getLocalBounds().width  + statusXOffset, 0.f));
        statusSprite.setTexture(m_textures->getResource(TextureId::SILENCED));
        target.draw(statusSprite, states);
    }

    if (m_unit->getStatus().disarmed) {
        statusSprite.setPosition(statusSprite.getPosition() + Vector2(statusSprite.getLocalBounds().width  + statusXOffset, 0.f));
        statusSprite.setTexture(m_textures->getResource(TextureId::DISARMED));
        target.draw(statusSprite, states);
    }

    if (m_unit->getStatus().rooted) {
        statusSprite.setPosition(statusSprite.getPosition() + Vector2(statusSprite.getLocalBounds().width  + statusXOffset, 0.f));
        statusSprite.setTexture(m_textures->getResource(TextureId::ROOTED));
        target.draw(statusSprite, states);
    }

    if (m_unit->getStatus().slowed) {
        statusSprite.setPosition(statusSprite.getPosition() + Vector2(statusSprite.getLocalBounds().width  + statusXOffset, 0.f));
        statusSprite.setTexture(m_textures->getResource(TextureId::SLOWED));
        target.draw(statusSprite, states);
    }

    //draw health bar
    sf::RectangleShape background;
    background.setSize(m_barSize);
    background.setPosition(m_unit->getPosition() - background.getSize()/2.f + Vector2(0.f, yOffset - m_unit->getCollisionRadius()));
    background.setFillColor(sf::Color::Black);
    target.draw(background, states);

    const float percentage = (float)m_unit->getHealth()/(float)m_unit->getMaxHealth();

    if (percentage != 0.f) {
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
    healthText.setString(std::to_string(m_unit->getMaxHealth()));
    healthText.setFillColor(sf::Color::White);
    healthText.setOrigin(healthText.getLocalBounds().width/2.f + healthText.getLocalBounds().left, 
                         healthText.getLocalBounds().height/2.f + healthText.getLocalBounds().top);
    healthText.setOutlineColor(sf::Color::Black);
    healthText.setOutlineThickness(1.f);
    healthText.setPosition(background.getPosition() + Vector2(background.getSize().x/2.f, 0.f));
    target.draw(healthText, states);

    //draw primary fire (only the controlled unit has a valid m_clientCaster)
    if (m_clientCaster) {
        const CooldownAbility* primaryFire = m_clientCaster->getPrimaryFire();
        const float xDistance = 2.f;
        const u8 maxCharges = primaryFire->getMaxCharges();
        const u8 currentCharges = primaryFire->getCurrentCharges();
        const float chargeSize = (m_barSize.x + xDistance)/maxCharges - xDistance;

        for (int i = 0; i < maxCharges; ++i) {
            sf::RectangleShape chargeBackground;
            chargeBackground.setSize({chargeSize, m_barSize.y});
            chargeBackground.setPosition(background.getPosition() + Vector2(i * (chargeSize + xDistance), 20.f));
            chargeBackground.setFillColor(sf::Color::Black);
            target.draw(chargeBackground, states);

            if (i <= currentCharges) {
                const float chargePercentage = (i == currentCharges) ? primaryFire->getPercentage() : 1.f;

                sf::RectangleShape chargeForeground;
                chargeForeground.setSize({chargePercentage * chargeSize, m_barSize.y});
                chargeForeground.setPosition(background.getPosition() + Vector2(i * (chargeSize + xDistance), 20.f));
                
                chargeForeground.setFillColor((i == currentCharges) ? AbilityUI::cooldownColor : AbilityUI::readyColor);

                target.draw(chargeForeground, states);
            }
        }
    }
}
