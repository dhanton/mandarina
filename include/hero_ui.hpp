#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include "defines.hpp"
#include "context.hpp"

class C_Hero;

class HeroUI : public sf::Drawable
{
public:
    HeroUI();

    void setHero(const C_Hero* hero);
    const C_Hero* getHero() const;

    void setFonts(const FontLoader* fonts);
    void setTextureLoader(const TextureLoader* textures);

    void setIsControlledEntity(bool isControlledEntity);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    const C_Hero* m_hero;
    const FontLoader* m_fonts;
    const TextureLoader* m_textures;
    
    bool m_isControlledEntity;
};
