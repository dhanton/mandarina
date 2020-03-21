#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <string>

#include "defines.hpp"
#include "context.hpp"

class AbilityUI : public sf::Drawable, public InContext
{
public:
    enum ShapeType {
        CIRCLE,
        BOX
    };

    static const sf::Color cooldownColor;
    static const sf::Color readyColor;
    static const sf::Color dimGreyColor;
    static const sf::Color greyColor;

public:
    AbilityUI(const Context& context);

    void setTexture(u16 textureId);
    void setShapeType(u8 shapeType);
    void setMaxTime(u16 maxTime);
    void setHotkey(const std::string& hotkey);

    float getPercentage() const;
    void setPercentage(float percentage);

    void setPosition(const Vector2& pos);
    Vector2 getPosition() const;

    Vector2 getBoundingSize() const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    u8 m_shapeType;
    const sf::Texture* m_texture;

    float m_percentage;
    Vector2 m_pos;

    //used to display time for boxes
    u16 m_maxTime;

    std::string m_hotkey;

    static const float m_circleScale;
    static const float m_boxScale;
    static const float m_boxBoundingSize;
    static const float m_circleBoundingSize;
    static const u8 m_alphaColor;
};
