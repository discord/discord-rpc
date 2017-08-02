using System.Runtime.InteropServices;

public class DiscordRpc
{
    [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
    public delegate void ReadyCallback();

    [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
    public delegate void DisconnectedCallback(int errorCode, string message);

    [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
    public delegate void ErrorCallback(int errorCode, string message);

    [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
    public delegate void JoinCallback(string secret);

    [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
    public delegate void SpectateCallback(string secret);

    [System.Serializable]
    public struct EventHandlers
    {
        public ReadyCallback readyCallback;
        public DisconnectedCallback disconnectedCallback;
        public ErrorCallback errorCallback;
        public JoinCallback joinCallback;
        public SpectateCallback spectateCallback;
    }

    [System.Serializable]
    public struct RichPresence
    {
        public string state; /* max 128 bytes */
        public string details; /* max 128 bytes */
        public long startTimestamp;
        public long endTimestamp;
        public string largeImageKey; /* max 32 bytes */
        public string largeImageText; /* max 128 bytes */
        public string smallImageKey; /* max 32 bytes */
        public string smallImageText; /* max 128 bytes */
        public string partyId; /* max 128 bytes */
        public int partySize;
        public int partyMax;
        public string matchSecret; /* max 128 bytes */
        public string joinSecret; /* max 128 bytes */
        public string spectateSecret; /* max 128 bytes */
        public bool instance;
    }

    [DllImport("discord-rpc", EntryPoint = "Discord_Initialize")]
    public static extern void Initialize(string applicationId, ref EventHandlers handlers, bool autoRegister);

    [DllImport("discord-rpc", EntryPoint = "Discord_Shutdown")]
    public static extern void Shutdown();

    [DllImport("discord-rpc", EntryPoint = "Discord_RunCallbacks")]
    public static extern void RunCallbacks();

    [DllImport("discord-rpc", EntryPoint = "Discord_UpdatePresence")]
    public static extern void UpdatePresence(ref RichPresence presence);
}

