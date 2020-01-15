#pragma once

#include "res_loader.hpp"
#include "json_parser.hpp"

struct Context {
    // sf::RenderTarget* renderTarget = nullptr;
    // sf::Window* window = nullptr; //Not the same as above if we're using a RenderTexture
    // sf::View* view = nullptr;

    bool SERVER = false;
    bool CLIENT = false;

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
