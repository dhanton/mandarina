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

void UnitUI::updateStatus(const Status& status)
{
	//reset m_status values
	m_status.preUpdate();

	for (auto it = m_statusBar.begin(); it != m_statusBar.end();) {
		if (!status[*it]) {
			//remove any status that is not in the unit anymore
			it = m_statusBar.erase(it);
		} else {
			//keep track of which status were already in the bar
			m_status[*it] = true;
			++it;
		}
	}

	for (int i = 0; i < STATUS_MAX_TYPES; ++i) {
		//if a status wasn't already in the bar (but it's in the unit)
		if (status[i] && !m_status[i]) {
			m_status[i] = true;

			//add it
			if (Status::getInStatusBar(i)) {
				m_statusBar.push_back(i);
			}
		}
	}
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
	const Vector2 initialPos = m_unit->getPosition() - barSize/2.f + Vector2(0.f, yOffset - 30.f - m_unit->getCollisionRadius());

    sf::Sprite statusSprite;
    float statusXOffset = 5.f;

	//render status in StatusBar
	for (int i = 0; i < m_statusBar.size(); ++i) {
		statusSprite.setTexture(m_textures->getResource(Status::getTextureId(m_statusBar[i])));
        statusSprite.setPosition(initialPos + Vector2(i * (statusSprite.getLocalBounds().width + statusXOffset), 0.f));
        statusSprite.setScale(Status::getScale(i), Status::getScale(i));
		target.draw(statusSprite, states);
	}

	//render the rest
	for (int i = 0; i < STATUS_MAX_TYPES; ++i) {
		if (m_status[i] && !Status::getInStatusBar(i)) {
			statusSprite.setTexture(m_textures->getResource(Status::getTextureId(i)));
            statusSprite.setOrigin(Vector2(statusSprite.getLocalBounds().width/2.f, statusSprite.getLocalBounds().height/2.f));
            statusSprite.setPosition(m_unit->getPosition() + Status::getOffset(i));
            statusSprite.setScale(Status::getScale(i), Status::getScale(i));
            target.draw(statusSprite, states);
		}
	}

    Vector2 backgroundPos = m_unit->getPosition() - HealthUI::barSize/2.f + Vector2(0.f, yOffset - m_unit->getCollisionRadius());

    //draw primary fire (only the controlled unit has a valid m_clientCaster)
    if (m_clientCaster) {
        const CooldownAbility* primaryFire = m_clientCaster->getPrimaryFire();
        constexpr float xDistance = 2.f;
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
