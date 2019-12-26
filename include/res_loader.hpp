#pragma once

#include <SFML/Graphics/Texture.hpp>

#include <map>
#include <memory>
#include <cassert>
#include <stdexcept>

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

using TextureLoader = ResLoader<sf::Texture, std::string>;

#include "res_loader.inl"
