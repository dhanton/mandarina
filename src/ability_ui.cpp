#include "ability_ui.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "helper.hpp"


//icons are 512x512 pixels
const float AbilityUI::m_circleScale = 0.23f;
const float AbilityUI::m_boxScale = 0.19f;
const float AbilityUI::m_boxBoundingSize = AbilityUI::m_boxScale * 512.f;
const float AbilityUI::m_circleBoundingSize = AbilityUI::m_circleScale * 512.f;
const u8 AbilityUI::m_alphaColor = 190;
const sf::Color AbilityUI::cooldownColor = sf::Color(255, 126, 0);
const sf::Color AbilityUI::readyColor = sf::Color(0, 210, 255);
const sf::Color AbilityUI::dimGreyColor = sf::Color(169, 169, 169, AbilityUI::m_alphaColor);
const sf::Color AbilityUI::greyColor = sf::Color(169, 169, 169, 255);

AbilityUI::AbilityUI(const Context& context):
    InContext(context)
{

}

void AbilityUI::setTexture(u16 textureId)
{
    m_context.textures->getResource(textureId).setSmooth(true);
    m_texture = &m_context.textures->getResource(textureId);
}

void AbilityUI::setShapeType(u8 shapeType)
{
    m_shapeType = shapeType;
}

void AbilityUI::setMaxTime(u16 maxTime)
{
    m_maxTime = maxTime;
}

void AbilityUI::setHotkey(const std::string& hotkey)
{
    m_hotkey = hotkey;
}

float AbilityUI::getPercentage() const
{
    return m_percentage;
}
void AbilityUI::setPercentage(float percentage)
{
    m_percentage = Helper_clamp(percentage, 0.f, 1.f);
}

void AbilityUI::setPosition(const Vector2& pos)
{
    m_pos = pos;
}

Vector2 AbilityUI::getPosition() const
{
    return m_pos;
}

Vector2 AbilityUI::getBoundingSize() const
{
    //we use a vector just in case we might want to use shapes that are not symmetric

    if (m_shapeType == BOX) {
        return Vector2(m_boxBoundingSize, m_boxBoundingSize);
    } else if (m_shapeType == CIRCLE) {
        return Vector2(m_circleBoundingSize, m_circleBoundingSize);
    }

    return Vector2();
}

void AbilityUI::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    switch (m_shapeType)
    {
        case BOX:
        {
            sf::Sprite sprite;
            sprite.setPosition(m_pos);
            sprite.setTexture(*m_texture);
            sprite.setScale(m_boxScale, m_boxScale);

            if (m_percentage == 1.f) {
                sf::Color color = sprite.getColor();
                color.a = m_alphaColor;
                sprite.setColor(color);
                target.draw(sprite, states);

            } else {
                const int number = std::floor(m_percentage * (float) m_maxTime);
                const Vector2 boxCenter = m_pos + Vector2(m_boxBoundingSize, m_boxBoundingSize)/2.f;

                sf::Text timeText;
                timeText.setFont(m_context.fonts->getResource("keep_calm_font"));
                timeText.setCharacterSize(45);
                timeText.setString(std::to_string(number));
                timeText.setFillColor(sf::Color::White);
                timeText.setOrigin(timeText.getLocalBounds().width/2.f + timeText.getLocalBounds().left, 
                                   timeText.getLocalBounds().height/2.f + timeText.getLocalBounds().top);
                timeText.setOutlineColor(sf::Color::Black);
                timeText.setOutlineThickness(2.f);
                timeText.setPosition(boxCenter);

                sf::Color cdColor = cooldownColor;
                cdColor.a = m_alphaColor;

                sf::RectangleShape cdShape;
                cdShape.setFillColor(cdColor);
                cdShape.setSize({m_boxBoundingSize * m_percentage, m_boxBoundingSize});
                cdShape.setPosition(sprite.getPosition());

                sprite.setColor(dimGreyColor);

                target.draw(sprite, states);
                target.draw(cdShape, states);
                target.draw(timeText, states);
            }

            //@TODO: Render available charges as well (with a little circle)
			if (!m_hotkey.empty()) {
				sf::RectangleShape hotkeyShape;
				hotkeyShape.setSize({m_boxBoundingSize, 25.f});
				hotkeyShape.setPosition(m_pos + Vector2(0.f, m_boxBoundingSize + 10.f));
				hotkeyShape.setFillColor(greyColor);

				sf::Text hotkeyText;
				hotkeyText.setFont(m_context.fonts->getResource("keep_calm_font"));
				hotkeyText.setCharacterSize(16);
				hotkeyText.setString(m_hotkey);
				hotkeyText.setFillColor(sf::Color::White);
				hotkeyText.setOutlineColor(sf::Color::Black);
				hotkeyText.setOutlineThickness(2.f);
				hotkeyText.setOrigin(hotkeyText.getLocalBounds().width/2.f + hotkeyText.getLocalBounds().left, 
									   hotkeyText.getLocalBounds().height/2.f + hotkeyText.getLocalBounds().top);
				hotkeyText.setPosition(hotkeyShape.getPosition() + hotkeyShape.getSize()/2.f);
				
				target.draw(hotkeyShape, states);
				target.draw(hotkeyText, states);
			}

            break;
        }

        case CIRCLE:
        {
            const Vector2 circleCenter = m_pos + Vector2(m_circleBoundingSize, m_circleBoundingSize)/2.f;

            sf::CircleShape circle;
            circle.setPosition(m_pos);
            circle.setTexture(m_texture);
            circle.setRadius(m_circleBoundingSize/2.f);
            
            circle.setOutlineThickness(8.f);

            if (m_percentage == 1.f) {
                sf::Color color = circle.getFillColor();
                color.a = m_alphaColor + 40;
                circle.setFillColor(color);
                circle.setOutlineColor(readyColor);

                target.draw(circle, states);

            } else {

                sf::Text percentageText;
                percentageText.setFont(m_context.fonts->getResource("keep_calm_font"));
                percentageText.setCharacterSize(60);
                percentageText.setString(std::to_string(int(m_percentage * 100.f)));
                percentageText.setFillColor(sf::Color::White);
                percentageText.setOrigin(percentageText.getLocalBounds().width/2.f + percentageText.getLocalBounds().left, 
                                         percentageText.getLocalBounds().height/2.f + percentageText.getLocalBounds().top);
                percentageText.setOutlineColor(sf::Color::Black);
                percentageText.setOutlineThickness(2.f);
                percentageText.setPosition(circleCenter);

                sf::Text totalText;
                totalText.setFont(m_context.fonts->getResource("keep_calm_font"));
                totalText.setCharacterSize(23);
                totalText.setString("%");
                totalText.setFillColor(sf::Color::White);
                totalText.setOutlineColor(sf::Color::Black);
                totalText.setOutlineThickness(2.f);
                totalText.setPosition(circleCenter.x + percentageText.getLocalBounds().width/2.f + 5.f, 
                                      circleCenter.y + percentageText.getLocalBounds().height/2.f - totalText.getLocalBounds().height - 5.f);

                circle.setOutlineColor(cooldownColor);
                circle.setFillColor(dimGreyColor);
                
                target.draw(circle, states);
                target.draw(percentageText, states);
                target.draw(totalText, states);
            }

            const Vector2 circleHotkeyPos = circleCenter + Vector2(0.f, m_circleBoundingSize - 55.f);

            sf::CircleShape hotkeyShape;
            hotkeyShape.setRadius(25.f);
            hotkeyShape.setOrigin({hotkeyShape.getRadius(), hotkeyShape.getRadius()});
            hotkeyShape.setPosition(circleHotkeyPos);
            hotkeyShape.setFillColor(greyColor);

            sf::Text hotkeyText;
            hotkeyText.setFont(m_context.fonts->getResource("keep_calm_font"));
            hotkeyText.setCharacterSize(26);
            hotkeyText.setString(m_hotkey);
            hotkeyText.setFillColor(sf::Color::White);
            hotkeyText.setOutlineColor(sf::Color::Black);
            hotkeyText.setOutlineThickness(2.f);
            hotkeyText.setOrigin(hotkeyText.getLocalBounds().width/2.f + hotkeyText.getLocalBounds().left, 
                                 hotkeyText.getLocalBounds().height/2.f + hotkeyText.getLocalBounds().top);
            hotkeyText.setPosition(circleHotkeyPos);

            target.draw(hotkeyShape);
            target.draw(hotkeyText);

            break;
        }
    }
}
