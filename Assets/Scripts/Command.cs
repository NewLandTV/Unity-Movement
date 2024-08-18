public static class Command
{
    // Client to Server
    public const uint CMD_C2S_JOIN_GAME = 0;
    public const uint CMD_C2S_LEAVE_GAME = 1;
    public const uint CMD_C2S_PLAYER_MOVEMENT = 2;

    // Server to Client
    public const uint CMD_S2C_USER_ID = 0;

    // Server & Client
    public const uint INVALID_CMD = 4294967295;
}
