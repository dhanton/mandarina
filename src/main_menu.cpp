#include "main_menu.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include "projectiles.hpp"
#include "weapon.hpp"

const sf::Color MainMenu::m_backgroundColor = sf::Color(104, 109, 224);
const sf::Color MainMenu::m_buttonColor = sf::Color(72, 52, 212);
const sf::Color MainMenu::m_hoveringColor = sf::Color(34, 166, 179);
//const sf::Color MainMenu::m_selectedColor = sf::Color(126, 214, 223);
const sf::Color MainMenu::m_selectedColor = sf::Color(149, 175, 192);

MainMenu::MainMenu(const Context& context):
    InContext(context),
    m_buildVersionRender(context)
{
    m_stopRunning = false;
    C_loadProjectilesFromJson(context.jsonParser);
    C_EntityManager::loadEntityData(context);

    const rapidjson::Document& doc = *context.jsonParser->getDocument("client_config");
    loadFromJson(doc);

    readDisplayName();

    m_window = std::unique_ptr<sf::RenderWindow>(new sf::RenderWindow({m_screenSize.x, m_screenSize.y}, "Mandarina", m_screenStyle));
    m_view = m_window->getDefaultView();

    m_context.window = m_window.get();
    m_context.view = &m_view;

    resetHeroSelection();

    constexpr float offsetX = 300.f;
    constexpr float width = 200.f;
    constexpr float height = 150.f;
    const Vector2u windowSize = m_window->getSize(); 
    
    for (int i = 0; i < g_numberOfHeroes; ++i) {
        m_heroButtons[i].left = windowSize.x/2.f + (i - 1) * offsetX - width/2.f;
        m_heroButtons[i].top = windowSize.y/2.f - height/2.f; 
        m_heroButtons[i].width = width;
        m_heroButtons[i].height = height;
    }

    constexpr float playWidth = 200.f;
    constexpr float playHeight = 50.f;

    m_playButton.left = windowSize.x/2.f - playWidth/2.f;
    m_playButton.top = windowSize.y - playHeight/2.f - 200.f;
    m_playButton.width = playWidth;
    m_playButton.height = playHeight;
}

void MainMenu::mainLoop(bool& running)
{
    sf::Clock clock;

    //weird artifacts happen if eTime is 0 during the first frame
    sf::Time eTime = sf::milliseconds(5);

    sf::Time updateTimer;
    sf::Time inputTimer;
    sf::Time renderTimer;

    bool focused = true;

    while (running) {
        if (m_gameClient) {
            m_gameClient->updateWorldTime(eTime);
        }

        updateTimer += eTime;
        inputTimer += eTime;
        renderTimer += eTime;

        if (m_gameClient) {
            m_gameClient->receiveLoop();
        }
        
        while (inputTimer >= m_inputRate) {
            sf::Event event;

            while (m_window->pollEvent(event)) {
                if (event.type == sf::Event::GainedFocus) {
                    focused = true;
                }

                else if (event.type == sf::Event::LostFocus) {
                    focused = false;
                }

                else if (event.type == sf::Event::Closed) {
                    running = false;
                }
                
                else {
                    handleInput(event, focused);
                }
            }

            if (m_gameClient) {
                m_gameClient->saveCurrentInput();
            }

            inputTimer -= m_inputRate;
        }

        while (updateTimer >= m_updateRate) {
            update(m_inputRate);
            updateTimer -= m_inputRate;
        }

        while (renderTimer >= m_renderRate) {
            if (m_gameClient) {
                m_gameClient->renderUpdate(m_renderRate == sf::Time::Zero ? eTime : m_renderRate);
            }

            m_window->clear();
            m_window->setView(m_view);

            m_window->draw(*this);

            m_window->display();

            renderTimer -= m_renderRate;
        }

        eTime = clock.restart();

        if (m_stopRunning) running = false;
    }
}

void MainMenu::startGame()
{
    if (m_gameClient) return;

    //camera zoom needs to be reset
    m_view = m_window->getDefaultView();

    GameClient::ConfigData data;
    data.endpoint = m_endpoint;
    data.inputRate = m_inputRate;
    data.displayName = m_displayName;
    
    std::vector<u8> available;

    for (int i = 0; i < g_numberOfHeroes; ++i) {
        if (m_selectedHeroes[i]) {
            available.push_back(i);
        }
    }
    
    if (available.empty()) {
        //for the server, 0 means random
        data.pickedHero = 0;
    } else {
        std::uniform_int_distribution<int> distr(0, available.size() - 1);

        //so we have to +1 the picked hero 
        data.pickedHero = available[distr(Helper_Random::gen())] + 1;
    }

    m_gameClient = std::unique_ptr<GameClient>(new GameClient(m_context, data));
    m_window->setMouseCursorVisible(false);
}

void MainMenu::stopGame()
{
    if (!m_gameClient) return;

    m_view = m_window->getDefaultView();

    m_gameClient.reset();
    m_window->setMouseCursorVisible(true);

    //it's better to remember what the player selected 
    //resetHeroSelection();
}

void MainMenu::update(sf::Time eTime)
{
    if (m_gameClient) {
        m_gameClient->update(eTime);
    } else {
        _doUpdate(eTime);
    }
}

void MainMenu::handleInput(const sf::Event& event, bool focused)
{
    _doHandleInput(event);

    if (m_gameClient) {
        m_gameClient->handleInput(event, focused);
    }
}

void MainMenu::loadFromJson(const rapidjson::Document& doc)
{
    m_inputRate = sf::seconds(1.f/30.f);

    if (doc.HasMember("update_rate")) {
        m_updateRate = sf::seconds(1.f/doc["update_rate"].GetFloat());
    } else {
        m_updateRate = sf::seconds(1.f/30.f);
    }

    if (doc.HasMember("frames_per_second")) {
        m_renderRate = sf::seconds(1.f/doc["frames_per_second"].GetFloat());
    } else {
        m_renderRate = sf::seconds(1.f/120.f);
    }

    if (doc.HasMember("server_ip_address")) {
        m_endpoint.ParseString(doc["server_ip_address"].GetString());
    } else {
        m_endpoint.ParseString("127.0.0.1");
    }

    if (doc.HasMember("server_port")) {
        m_endpoint.m_port = doc["server_port"].GetUint();
    } else {
        m_endpoint.m_port = 7000;
    }

    if (doc.HasMember("resolution")) {
        m_screenSize.x = doc["resolution"][0].GetUint();
        m_screenSize.y = doc["resolution"][1].GetUint();
    } else {
        m_screenSize.x = 1080;
        m_screenSize.y = 720;
    }

    if (doc.HasMember("fullscreen")) {
        if (doc["fullscreen"].GetBool()) {
            m_screenStyle = sf::Style::Fullscreen;
        } else {
            m_screenStyle = sf::Style::Titlebar | sf::Style::Close;
        }
    } else {
        m_screenStyle = sf::Style::Titlebar | sf::Style::Close;
    }
}

void MainMenu::readDisplayName()
{
#ifndef MANDARINA_DEBUG
    const char* itchioAPIKey = std::getenv("ITCHIO_API_KEY");

    if (itchioAPIKey) {
        std::string authString = "Bearer " + std::string(itchioAPIKey);

        httplib::SSLClient itchioClient("itch.io");
        httplib::Headers authHeader = {
            { "Authorization", authString}
        };

        auto res = itchioClient.Get("/api/1/jwt/me", authHeader);

        if (res && res->status == 200) {
            rapidjson::Document doc;
            doc.Parse(res->body.c_str());

            if (doc.HasMember("user") && doc["user"].HasMember("display_name")) {
                m_displayName = doc["user"]["display_name"].GetString();
            }
        }

    } else {
        std::fstream nameFile(DATA_PATH + "name.txt");

        if (nameFile.is_open()) {
            std::getline(nameFile, m_displayName);
        }
    }
    
    if (m_displayName.size() > HeroBase::maxDisplayNameSize) {
        m_displayName.resize(HeroBase::maxDisplayNameSize);
    }
#else
    m_displayName = "Debug";
#endif
}

void MainMenu::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (m_gameClient) {
        target.draw(*m_gameClient, states);
    } else {
        _doDraw(target, states);
    }

    target.draw(m_buildVersionRender, states);
}

void MainMenu::_doUpdate(sf::Time eTime)
{
    
}

void MainMenu::_doHandleInput(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            if (m_gameClient && !m_context.local) {
                stopGame();
            } else {
                m_stopRunning = true;
            }
        }

        else if (event.key.code == sf::Keyboard::Return) {
            if (!m_gameClient) {
                startGame();    
            }
        }
    }

    if (m_gameClient) return;

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        const Vector2 mousePos = static_cast<Vector2>(sf::Mouse::getPosition(*m_window));

        if (m_playButton.contains(mousePos)) {
            startGame();
            return;
        }

        for (int i = 0; i < g_numberOfHeroes; ++i) {
            if (m_heroButtons[i].contains(mousePos)) {
                m_selectedHeroes[i] = !m_selectedHeroes[i];
                return;
            }
        }   
    }
}

void MainMenu::_doDraw(sf::RenderTarget& target, sf::RenderStates states) const
{
    const Vector2u windowSize = m_window->getSize(); 

    constexpr float backgroundOffset = 200.f;

    sf::RectangleShape background;
    background.setSize(static_cast<Vector2>(windowSize) - Vector2(backgroundOffset, backgroundOffset));
    background.setOrigin(background.getSize().x/2.f, background.getSize().y/2.f);
    background.setPosition(windowSize.x/2.f, windowSize.y/2.f);
    background.setFillColor(m_backgroundColor);

    target.draw(background, states);

    //no need to do any transformations since the view is not moving 
    //and there is no zoom in the menu
    const Vector2 mousePos = static_cast<Vector2>(sf::Mouse::getPosition(*m_window));

    for (int i = 0; i < g_numberOfHeroes; ++i) {
        const sf::FloatRect& button = m_heroButtons[i];

        sf::RectangleShape shape;

        shape.setFillColor(m_selectedHeroes[i] ? m_selectedColor : m_buttonColor);
        shape.setSize({button.width, button.height});
        shape.setOrigin(button.width/2.f, button.height/2.f);
        shape.setPosition(button.left + button.width/2.f, button.top + button.height/2.f);

        if (button.contains(mousePos)) {
            shape.setOutlineThickness(3.f);
            shape.setOutlineColor(m_hoveringColor);
        }
        
        target.draw(shape, states);

        const C_Unit* unit = static_cast<C_Unit*>(C_EntityManager::getEntityData(g_heroTypes[i]));  

        sf::Sprite sprite;
        sprite.setTexture(m_context.textures->getResource(unit->getTextureId()));
        sprite.setScale(unit->getScale(), unit->getScale());
        sprite.setOrigin(sprite.getLocalBounds().width/2.f, sprite.getLocalBounds().height/2.f);
        sprite.setPosition(shape.getPosition());

        target.draw(sprite, states);

        u8 weaponId = unit->getWeaponId();

        if (weaponId != WEAPON_NONE) {
            const Weapon& weapon = g_weaponData[weaponId];

            sf::Sprite weaponSprite;
            weaponSprite.setTexture(m_context.textures->getResource(weapon.textureId));
            weaponSprite.setScale(weapon.scale, weapon.scale);
            weaponSprite.setOrigin(Vector2(weaponSprite.getLocalBounds().width/2.f, weaponSprite.getLocalBounds().height/2.f) + weapon.originOffset);
            weaponSprite.setPosition(sprite.getPosition());

            Vector2 dir = mousePos - weaponSprite.getPosition();
            float aimAngle = Helper_radToDeg(std::atan2(dir.x, dir.y));
            weaponSprite.setRotation(-aimAngle - weapon.angleOffset);

            target.draw(weaponSprite, states);
        }
    }

    constexpr float titleBgWidth = 550.f;
    constexpr float titleBgHeight = 100.f;

    sf::RectangleShape titleBg;
    titleBg.setSize({titleBgWidth, titleBgHeight});
    titleBg.setOrigin(titleBgWidth/2.f, titleBgHeight/2.f);
    titleBg.setPosition(windowSize.x/2.f, 250.f);
    titleBg.setFillColor(m_buttonColor);

    target.draw(titleBg, states);

    sf::Text titleText;
    titleText.setFont(m_context.fonts->getResource("keep_calm_font"));
    titleText.setCharacterSize(70);
    titleText.setString("MANDARINA");
    titleText.setOrigin(titleText.getLocalBounds().width/2.f + titleText.getLocalBounds().left,
                        titleText.getLocalBounds().height/2.f + titleText.getLocalBounds().top);
    titleText.setPosition(titleBg.getPosition());

    target.draw(titleText, states);

    sf::RectangleShape playShape;
    playShape.setSize({m_playButton.width, m_playButton.height});
    playShape.setOrigin(m_playButton.width/2.f, m_playButton.height/2.f);
    playShape.setPosition(m_playButton.left + m_playButton.width/2.f, m_playButton.top + m_playButton.height/2.f);
    playShape.setFillColor(m_buttonColor);

    if (m_playButton.contains(mousePos)) {
        playShape.setOutlineThickness(3.f);
        playShape.setOutlineColor(m_hoveringColor);
    }

    target.draw(playShape, states);

    sf::Text playText;
    playText.setFont(m_context.fonts->getResource("keep_calm_font"));
    playText.setCharacterSize(30);
    playText.setString("PLAY");
    playText.setOrigin(playText.getLocalBounds().width/2.f + playText.getLocalBounds().left,
                        playText.getLocalBounds().height/2.f + playText.getLocalBounds().top);
    playText.setPosition(playShape.getPosition());

    target.draw(playText, states);
}

void MainMenu::resetHeroSelection()
{
    for (int i = 0; i < g_numberOfHeroes; ++i) {
        m_selectedHeroes[i] = false;
    }
}
