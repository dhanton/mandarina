#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include "defines.hpp"
#include "context.hpp"
#include "status.hpp"

class C_Unit;
class ClientCaster;

class UnitUI : public sf::Drawable
{
public:
    UnitUI();

	void updateStatus(const Status& status);

    void setUnit(const C_Unit* unit);
    const C_Unit* getUnit() const;

    void setClientCaster(const ClientCaster* clientCaster);
    const ClientCaster* getClientCaster() const;
    
    void setFonts(const FontLoader* fonts);
    void setTextureLoader(const TextureLoader* textures);

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    const C_Unit* m_unit;
    const ClientCaster* m_clientCaster;
    const FontLoader* m_fonts;
    const TextureLoader* m_textures;

	std::vector<u8> m_statusBar;
	Status m_status;
};
