#include "main_menu.hpp"

#include "projectiles.hpp"

MainMenu::MainMenu(const Context& context):
	InContext(context)
{
	m_stopRunning = false;
	C_loadProjectilesFromJson(context.jsonParser);

	m_selectedHero = 0;

	const rapidjson::Document& doc = *context.jsonParser->getDocument("client_config");
	loadFromJson(doc);

	readDisplayName();

	m_window = std::unique_ptr<sf::RenderWindow>(new sf::RenderWindow({m_screenSize.x, m_screenSize.y}, "Mandarina v0.0.3", m_screenStyle));
	m_view = m_window->getDefaultView();

	m_context.window = m_window.get();
	m_context.view = &m_view;
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
	data.selectedHero = m_selectedHero;

	m_gameClient = std::unique_ptr<GameClient>(new GameClient(m_context, data));
	m_window->setMouseCursorVisible(false);
}

void MainMenu::stopGame()
{
	if (!m_gameClient) return;

	m_gameClient.reset();
	m_window->setMouseCursorVisible(true);
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
}

void MainMenu::_doUpdate(sf::Time eTime)
{
	//Update hero aimAngle using the mouse
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
}

void MainMenu::_doDraw(sf::RenderTarget& target, sf::RenderStates states) const
{
	//Render each hero
}
