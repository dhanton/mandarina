#include <iostream>
#include <thread>

#include <SFML/Graphics.hpp>

#include "game_server.hpp"
#include "game_client.hpp"
#include "res_loader.hpp"

enum class ExecMode {
    Client,
    Server,
    LocalConnection
};

int main(int argc, char* argv[])
{
    ExecMode execMode = ExecMode::Client;

    SteamDatagramErrMsg error;
    if (!GameNetworkingSockets_Init(nullptr, error)) {
        std::cout << "Failed to initialize GameNetworkingSockets. Error:" << error << std::endl;
        return -1;
    }

    ISteamNetworkingSockets* pInterface = SteamNetworkingSockets();
    HSteamNetConnection localCon1 = k_HSteamNetConnection_Invalid;
    HSteamNetConnection localCon2 = k_HSteamNetConnection_Invalid;

    if (argc > 1) {
        std::string option = argv[1];

        if (option == "--server") {
            execMode = ExecMode::Server;

        } else if (option == "--local-connection") {
            execMode = ExecMode::LocalConnection;

            pInterface->CreateSocketPair(&localCon1, &localCon2, true, nullptr, nullptr);
        }
    }

    //can be shared between client and server (in case of local connection)
    bool running = true;

    auto clientFunc = [&] () {
        TextureLoader textures;
        textures.loadResource("../../data/muscles.png", "test_character");

        sf::RenderWindow window{{960, 640}, "Mandarina Prototype", sf::Style::Titlebar | sf::Style::Close};

        SteamNetworkingIPAddr serverAddr;
        serverAddr.ParseString("127.0.0.1:7000");

        Context context;
        context.local = (execMode == ExecMode::LocalConnection);
        context.localCon1 = localCon1;
        context.localCon2 = localCon2;
        context.textures = &textures;
        context.CLIENT = true;

        GameClient client(context, serverAddr);

        while (running) {
            sf::Event event;

            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    running = false;
                }

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                    running = false;
                }
            }

            client.update(sf::Time::Zero);
            client.receiveLoop();

            window.clear();
            window.display();
        }
    };

    auto serverFunc = [&] () {
        Context context;
        context.local = (execMode == ExecMode::LocalConnection);
        context.localCon1 = localCon1;
        context.localCon2 = localCon2;
        context.SERVER = true;

        GameServer server(context, 1);

        while (running) {
            server.receiveLoop();
            server.update(sf::Time::Zero, running);

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

    };

    switch (execMode) {
        case ExecMode::Client:
        {
            clientFunc();
            break;
        }

        case ExecMode::Server:
        {
            serverFunc();
            break;
        }

        case ExecMode::LocalConnection:
        {
            std::thread thread(serverFunc);
            clientFunc();
            thread.join();
            break;
        }
    }

    GameNetworkingSockets_Kill();

    return 0;
}