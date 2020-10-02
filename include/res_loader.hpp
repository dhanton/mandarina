#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <map>
#include <memory>
#include <cassert>
#include <stdexcept>

#include "defines.hpp"

template <typename Res, typename Id = std::string>
class ResLoader : private sf::NonCopyable
{
public:
    void loadResource(const std::string& filename, Id id);

    Res& getResource(Id id);
    const Res& getResource(Id id) const;

    template <typename Param> void loadResource(const std::string& filename, Id id, const Param &param);
private:
    std::map<Id, std::unique_ptr<Res>> m_resourceMap;
};

using TextureLoader = ResLoader<sf::Texture, u16>;
using FontLoader = ResLoader<sf::Font, std::string>;
using ShaderLoader = ResLoader<sf::Shader, std::string>;

#include "res_loader.inl"
