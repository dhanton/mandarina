#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/Time.hpp>
#include "defines.hpp"
#include "context.hpp"
#include "render_node.hpp"

class C_Entity;
class HealthComponent;

class HealthUI : public sf::Drawable
{
public:
    static const Vector2 barSize;
    static float getYOffset(bool isControlledEntity);

    HealthUI();

    void renderUpdate(sf::Time eTime, RenderNode& node);

    void setEntity(const C_Entity* entity);
    const C_Entity* getEntity() const;

    void setHealthComponent(const HealthComponent* healthComponent);

    void setIsAlly(bool isAlly);
    bool getIsAlly() const;

    void setFonts(const FontLoader* fonts);
    void setIsControlledEntity(bool isControlledEntity);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    const C_Entity* m_entity;
    const HealthComponent* m_health;
    const FontLoader* m_fonts;

    bool m_isAlly;
    bool m_isControlledEntity;

    bool m_takingDamage;
    sf::Time m_takingDamageTimer;
    u16 m_prevHealth;

    static const sf::Time m_takingDamageTotalTime;
};
