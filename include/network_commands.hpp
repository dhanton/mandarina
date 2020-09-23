//Commands that can be received by the client
enum class ClientCommand {
    Null,
    Snapshot,
    RequestInitialInfo,
	HeroCreated,
    PlayerCoords,
    GameModeType,
    GameStarted,
    ChangeSpectator,
    TeamEliminated,
    GameEnded
};

//Commands that can be received by the server
enum class ServerCommand {
    Null,
    LatestSnapshotId,
    PlayerInput,
    ChangeInputRate,
    ChangeSnapshotRate,
    DisplayName,
	SelectedHero
};
