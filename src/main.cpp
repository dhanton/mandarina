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

#ifdef MANDARINA_DEBUG
    //NETWORK CONDITIONS TESTING
    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Recv, 25);
    // SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLoss_Recv, 10);
    // SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketReorder_Recv, 10);
#endif

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
        
        //@TODO: Load textures automatically

        textures->loadResource(TEXTURES_PATH + "test_tileset.png", TextureId::TEST_TILESET);

        textures->loadResource(TEXTURES_PATH + "stunned.png", TextureId::STUNNED);
        textures->loadResource(TEXTURES_PATH + "silenced.png", TextureId::SILENCED);
        textures->loadResource(TEXTURES_PATH + "disarmed.png", TextureId::DISARMED);
        textures->loadResource(TEXTURES_PATH + "rooted.png", TextureId::ROOTED);
        textures->loadResource(TEXTURES_PATH + "slowed.png", TextureId::SLOWED);

        textures->loadResource(TEXTURES_PATH + "diablo.png", TextureId::RED_DEMON);
        textures->loadResource(TEXTURES_PATH + "devils_bow.png", TextureId::DEVILS_BOW);
        textures->loadResource(TEXTURES_PATH + "hells_bubble.png", TextureId::HELLS_BUBBLE);
        textures->loadResource(TEXTURES_PATH + "hells_dart.png", TextureId::HELLS_DART);
        textures->loadResource(TEXTURES_PATH + "food.png", TextureId::FOOD);
        textures->loadResource(TEXTURES_PATH + "normal_crate.png", TextureId::NORMAL_CRATE);

        textures->loadResource(TEXTURES_PATH + "storm.png", TextureId::STORM);

        textures->loadResource(ICONS_PATH + "hells_bubble.png", TextureId::ICON_HELLS_BUBBLE);
        textures->loadResource(ICONS_PATH + "hells_dart.png", TextureId::ICON_HELLS_DART);
        textures->loadResource(ICONS_PATH + "hells_dash.png", TextureId::ICON_HELLS_DASH);
        textures->loadResource(ICONS_PATH + "hells_rain.png", TextureId::ICON_HELLS_RAIN);

        context.textures = textures.get();
    }

    FontLoader fonts;
    fonts.loadResource(FONTS_PATH + "Keep Calm.ttf", "keep_calm_font");
    context.fonts = &fonts;

    JsonParser jsonParser;
    jsonParser.loadAll(JSON_PATH);

    context.jsonParser = &jsonParser;

    loadWeaponsFromJson(&jsonParser);

    CasterComponent::loadAbilityData(&jsonParser);
    BuffHolderComponent::loadBuffData(&jsonParser);

    SteamNetworkingIPAddr serverAddr;
    serverAddr.ParseString("127.0.0.1:7000");

    //client and server are not created in the main context so that
    //they call their deleter before GameNetworkingSockets is killed

    switch (execMode) {
        case ExecMode::Client:
        {
            GameClient client(context, serverAddr);
            client.mainLoop(running);

            break;
        }

        case ExecMode::Server:
        {
            GameServer server(context, GAME_MODE_BATTLE_ROYALE_FFA);
            server.mainLoop(running);

            break;
        }

        case ExecMode::LocalConnection:
        {
            GameServer server(context,GAME_MODE_BATTLE_ROYALE_FFA);

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