#ifndef CONTEXT_HPP
#define CONTEXT_HPP

struct Context {
    // sf::RenderTarget* renderTarget = nullptr;
    // sf::Window* window = nullptr; //Not the same as above if we're using a RenderTexture
    // sf::View* view = nullptr;

    bool local = false;
    uint32_t localCon1 = 0;
    uint32_t localCon2 = 0;
};

class InContext {
public:
    InContext(const Context& context):
        m_context(context) {}

protected:
    Context m_context;
};

#endif // CONTEXT_HPP
