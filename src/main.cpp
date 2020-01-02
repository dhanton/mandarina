#include <iostream>
#include <thread>

#include <SFML/Graphics.hpp>

#include "game_server.hpp"
#include "game_client.hpp"
#include "res_loader.hpp"
#include "texture_ids.hpp"

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

    //NETWORK CONDITIONS TESTING
    // SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Recv, 500);
    // SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLoss_Recv, 10);
    // SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketReorder_Recv, 10);

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
        textures.loadResource("../../data/diablo.png", TextureId::DIABLO);

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

        sf::Clock clock;

        const sf::Time updateSpeed = sf::seconds(1.f/30.f);
        const sf::Time inputSpeed = sf::seconds(1.f/30.f);

        sf::Time updateTimer;
        sf::Time inputTimer;

        while (running) {
            sf::Time eTime = clock.restart();

            updateTimer += eTime;
            inputTimer += eTime;

            client.updateWorldTime(eTime);
            client.receiveLoop();

            while (inputTimer >= inputSpeed) {
                sf::Event event;

                client.saveCurrentInput();

                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        running = false;
                    }

                    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                        running = false;
                    }

                    client.handleInput(event);
                }

                inputTimer -= inputSpeed;
            }

            while (updateTimer >= updateSpeed) {
                client.update(updateSpeed);
                updateTimer -= updateSpeed;
            }
            
            client.renderUpdate(eTime);

            window.clear();
            window.draw(client);
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

        sf::Clock clock;

        //@TODO: Load this from json config file
        //**see how valve does it for counter strike, in regards to update/snapshot/input rates config**
        const sf::Time updateSpeed = sf::seconds(1.f/30.f);
        const sf::Time snapshotSpeed = sf::seconds(1.f/20.f);

        sf::Time updateTimer;
        sf::Time snapshotTimer;

        while (running) {
            sf::Time eTime = clock.restart();

            updateTimer += eTime;
            snapshotTimer += eTime;

            server.receiveLoop();

            if (updateTimer >= updateSpeed) {
                server.update(updateSpeed, running);
                updateTimer -= updateSpeed;
            }

            if (snapshotTimer >= snapshotSpeed) {
                server.sendSnapshots();
                snapshotTimer -= snapshotSpeed;
            }

            //Remove this for maximum performance (more CPU usage)
            // std::this_thread::sleep_for(std::chrono::milliseconds(5));
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