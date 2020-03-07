#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include "res_loader.hpp"
#include "json_parser.hpp"

struct Context {
    sf::RenderTarget* renderTarget = nullptr;
    sf::Window* window = nullptr;
    sf::View* view = nullptr;

    TextureLoader* textures = nullptr;

    bool local = false;
    uint32_t localCon1 = 0;
    uint32_t localCon2 = 0;

    JsonParser* jsonParser = nullptr;
};

class InContext {
public:
    InContext(const Context& context):
        m_context(context) {}

protected:
    Context m_context;
};
