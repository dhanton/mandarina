#include "unit_ui.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "unit.hpp"
#include "client_caster.hpp"
#include "context.hpp"
#include "texture_ids.hpp"
#include "health_ui.hpp"

UnitUI::UnitUI()
{
    m_unit = nullptr;
    m_clientCaster = nullptr;
    m_fonts = nullptr;
    m_textures = nullptr;
}

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

    const float yOffset = HealthUI::getYOffset(m_clientCaster);
    const Vector2 barSize = HealthUI::barSize;

    //draw status icons
    sf::Sprite statusSprite;
    float statusXOffset = 5.f;
    statusSprite.setPosition(m_unit->getPosition() - barSize/2.f + Vector2(0.f, yOffset - 30.f - m_unit->getCollisionRadius()));

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

    Vector2 backgroundPos = m_unit->getPosition() - HealthUI::barSize/2.f + Vector2(0.f, yOffset - m_unit->getCollisionRadius());

    //draw primary fire (only the controlled unit has a valid m_clientCaster)
    if (m_clientCaster) {
        const CooldownAbility* primaryFire = m_clientCaster->getPrimaryFire();
        const float xDistance = 2.f;
        const u8 maxCharges = primaryFire->getMaxCharges();
        const u8 currentCharges = primaryFire->getCurrentCharges();
        const float chargeSize = (barSize.x + xDistance)/maxCharges - xDistance;

        for (int i = 0; i < maxCharges; ++i) {
            sf::RectangleShape chargeBackground;
            chargeBackground.setSize({chargeSize, barSize.y});
            chargeBackground.setPosition(backgroundPos + Vector2(i * (chargeSize + xDistance), 20.f));
            chargeBackground.setFillColor(sf::Color::Black);
            target.draw(chargeBackground, states);

            if (i <= currentCharges) {
                const float chargePercentage = (i == currentCharges) ? primaryFire->getPercentage() : 1.f;

                sf::RectangleShape chargeForeground;
                chargeForeground.setSize({chargePercentage * chargeSize, barSize.y});
                chargeForeground.setPosition(backgroundPos + Vector2(i * (chargeSize + xDistance), 20.f));
                
                chargeForeground.setFillColor((i == currentCharges) ? AbilityUI::cooldownColor : AbilityUI::readyColor);

                target.draw(chargeForeground, states);
            }
        }
    }
}
