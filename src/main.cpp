#include <iostream>
#include <thread>

#include <SFML/Graphics.hpp>

#include "paths.hpp"
#include "game_server.hpp"
#include "game_client.hpp"
#include "res_loader.hpp"
#include "texture_ids.hpp"

#include "json_parser.hpp"
#include "weapon.hpp"
#include "ability.hpp"

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
    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Recv, 25);
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

    srand(time(nullptr));

    //can be shared between client and server (in case of local connection)
    bool running = true;

    Context context;
    context.local = (execMode == ExecMode::LocalConnection);
    context.localCon1 = localCon1;
    context.localCon2 = localCon2;

    std::unique_ptr<TextureLoader> textures;
    
    //only load textures in client
    if (execMode == ExecMode::Client || execMode == ExecMode::LocalConnection) {
        textures = std::unique_ptr<TextureLoader>(new TextureLoader());
        
        textures->loadResource(TEXTURES_PATH + "test_tileset.png", TextureId::TEST_TILESET);
        textures->loadResource(TEXTURES_PATH + "diablo.png", TextureId::RED_DEMON);
        textures->loadResource(TEXTURES_PATH + "devils_bow.png", TextureId::DEVILS_BOW);
        textures->loadResource(TEXTURES_PATH + "hells_bubble.png", TextureId::HELLS_BUBBLE);

        context.textures = textures.get();
    }

    JsonParser jsonParser;
    jsonParser.loadAll(JSON_PATH);

    context.jsonParser = &jsonParser;

    loadWeaponsFromJson(&jsonParser);
    loadAbilitiesFromJson(&jsonParser);

    SteamNetworkingIPAddr serverAddr;
    serverAddr.ParseString("127.0.0.1:7000");

    //client and server are not created in the main context so that
    //they call their deleter before GameNetworkingSockets is killed

    switch (execMode) {
        case ExecMode::Client:
        {
            context.CLIENT = true;

            GameClient client(context, serverAddr);
            client.mainLoop(running);

            break;
        }

        case ExecMode::Server:
        {
            context.SERVER = true;

            GameServer server(context, 1);
            server.mainLoop(running);

            break;
        }

        case ExecMode::LocalConnection:
        {
            context.CLIENT = true;
            context.SERVER = true;

            GameServer server(context, 1);

            std::thread thread(&GameServer::mainLoop, &server, std::ref(running));

            GameClient client(context, serverAddr);
            client.mainLoop(running);

            thread.join();
            break;
        }
    }

    GameNetworkingSockets_Kill();

    return 0;
}