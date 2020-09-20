#include "res_loader.hpp"

template <typename Res, typename Id>
void ResLoader<Res, Id>::loadResource(const std::string& filename, Id id)
{
    std::unique_ptr<Res> resource{new Res{}};

    if (!resource->loadFromFile(filename))
        throw std::runtime_error("ResourceHandler::loadResource - Failed to load " + filename + '\n');

    auto inserted = m_resourceMap.emplace(id, std::move(resource));

    assert(inserted.second);
}

template <typename Res, typename Id>
Res& ResLoader<Res, Id>::getResource(Id id)
{
    auto found = m_resourceMap.find(id);

    assert(found != m_resourceMap.end());

    return *found->second;
}

template <typename Res, typename Id>
const Res& ResLoader<Res, Id>::getResource(Id id) const
{
    auto found = m_resourceMap.find(id);

    assert(found != m_resourceMap.end());

    return *found->second;
}

template<typename Res, typename Id>
template<typename Param>
void ResLoader<Res, Id>::loadResource(const std::string& filename, Id id, const Param &param)
{
    std::unique_ptr<Res> resource{new Res{}};

    if (!resource->loadFromFile(filename, param))
        throw std::runtime_error("ResourceHandler::loadResource - Failed to load " + filename + '\n');

    auto inserted = m_resourceMap.emplace(id, std::move(resource));

    assert(inserted.second);
}
