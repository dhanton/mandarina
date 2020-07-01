#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include "defines.hpp"
#include "context.hpp"

class C_Unit;
class ClientCaster;

class UnitUI : public sf::Drawable
{
public:
    void setUnit(const C_Unit* unit);
    const C_Unit* getUnit() const;

    void setClientCaster(const ClientCaster* clientCaster);
    const ClientCaster* getClientCaster() const;
    
    void setIsAlly(bool isAlly);
    bool getIsAlly() const;

    void setFonts(const FontLoader* fonts);
    void setTextureLoader(const TextureLoader* textures);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    const C_Unit* m_unit;
    const ClientCaster* m_clientCaster;
    const FontLoader* m_fonts;
    const TextureLoader* m_textures;
    
    bool m_isAlly;

    static const Vector2 m_barSize;
};
