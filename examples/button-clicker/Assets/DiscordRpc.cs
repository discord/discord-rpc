﻿using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

public class DiscordRpc
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ReadyCallback();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DisconnectedCallback(int errorCode, string message);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ErrorCallback(int errorCode, string message);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void JoinCallback(string secret);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SpectateCallback(string secret);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void RequestCallback(ref JoinRequest request);

    public struct EventHandlers
    {
        public ReadyCallback readyCallback;
        public DisconnectedCallback disconnectedCallback;
        public ErrorCallback errorCallback;
        public JoinCallback joinCallback;
        public SpectateCallback spectateCallback;
        public RequestCallback requestCallback;
    }

    [Serializable, StructLayout(LayoutKind.Sequential)]
    public struct RichPresenceStruct
    {
        public IntPtr state; /* max 128 bytes */
        public IntPtr details; /* max 128 bytes */
        public long startTimestamp;
        public long endTimestamp;
        public IntPtr largeImageKey; /* max 32 bytes */
        public IntPtr largeImageText; /* max 128 bytes */
        public IntPtr smallImageKey; /* max 32 bytes */
        public IntPtr smallImageText; /* max 128 bytes */
        public IntPtr partyId; /* max 128 bytes */
        public int partySize;
        public int partyMax;
        public IntPtr matchSecret; /* max 128 bytes */
        public IntPtr joinSecret; /* max 128 bytes */
        public IntPtr spectateSecret; /* max 128 bytes */
        public bool instance;
    }

    [Serializable]
    public struct JoinRequest
    {
        public string userId;
        public string username;
        public string discriminator;
        public string avatar;
    }

    public enum Reply
    {
        No = 0,
        Yes = 1,
        Ignore = 2
    }

    [DllImport("discord-rpc", EntryPoint = "Discord_Initialize", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Initialize(string applicationId, ref EventHandlers handlers, bool autoRegister, string optionalSteamId);

    [DllImport("discord-rpc", EntryPoint = "Discord_Shutdown", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Shutdown();

    [DllImport("discord-rpc", EntryPoint = "Discord_RunCallbacks", CallingConvention = CallingConvention.Cdecl)]
    public static extern void RunCallbacks();

    [DllImport("discord-rpc", EntryPoint = "Discord_UpdatePresence", CallingConvention = CallingConvention.Cdecl)]
    private static extern void UpdatePresenceNative(ref RichPresenceStruct presence);

    [DllImport("discord-rpc", EntryPoint = "Discord_ClearPresence", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ClearPresence();

    [DllImport("discord-rpc", EntryPoint = "Discord_Respond", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Respond(string userId, Reply reply);

    public static void UpdatePresence(RichPresence presence)
    {
        var presencestruct = presence.GetStruct();
        UpdatePresenceNative(ref presencestruct);
        presence.FreeMem();
    }

    public class RichPresence
    {
        private RichPresenceStruct _presence;
        private readonly List<IntPtr> _buffers = new List<IntPtr>(10);

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

        /// <summary>
        /// Get the <see cref="RichPresenceStruct"/> reprensentation of this instance
        /// </summary>
        /// <returns><see cref="RichPresenceStruct"/> reprensentation of this instance</returns>
        internal RichPresenceStruct GetStruct()
        {
            if (_buffers.Count > 0)
            {
                FreeMem();
            }

            _presence.state = StrToPtr(state, 128);
            _presence.details = StrToPtr(details, 128);
            _presence.startTimestamp = startTimestamp;
            _presence.endTimestamp = endTimestamp;
            _presence.largeImageKey = StrToPtr(largeImageKey, 32);
            _presence.largeImageText = StrToPtr(largeImageText, 128);
            _presence.smallImageKey = StrToPtr(smallImageKey, 32);
            _presence.smallImageText = StrToPtr(smallImageText, 128);
            _presence.partyId = StrToPtr(partyId, 128);
            _presence.partySize = partySize;
            _presence.partyMax = partyMax;
            _presence.matchSecret = StrToPtr(matchSecret, 128);
            _presence.joinSecret = StrToPtr(joinSecret, 128);
            _presence.spectateSecret = StrToPtr(spectateSecret, 128);
            _presence.instance = instance;

            return _presence;
        }

        /// <summary>
        /// Returns a pointer to a representation of the given string with a size of maxbytes
        /// </summary>
        /// <param name="input">String to convert</param>
        /// <param name="maxbytes">Max number of bytes to use</param>
        /// <returns>Pointer to the UTF-8 representation of <see cref="input"/></returns>
        private IntPtr StrToPtr(string input, int maxbytes)
        {
            if (string.IsNullOrEmpty(input)) return IntPtr.Zero;
            var convstr = StrClampBytes(input, maxbytes);
            var convbytecnt = Encoding.UTF8.GetByteCount(convstr);
            var buffer = Marshal.AllocHGlobal(convbytecnt);
            _buffers.Add(buffer);
            Marshal.Copy(Encoding.UTF8.GetBytes(convstr), 0, buffer, convbytecnt);
            return buffer;
        }

        /// <summary>
        /// Convert string to UTF-8 and add null termination
        /// </summary>
        /// <param name="toconv">string to convert</param>
        /// <returns>UTF-8 representation of <see cref="toconv"/> with added null termination</returns>
        private static string StrToUtf8NullTerm(string toconv)
        {
            var str = toconv.Trim();
            var bytes = Encoding.Default.GetBytes(str);
            if (bytes.Length > 0 && bytes[bytes.Length - 1] != 0)
            {
                str += "\0\0";
            }
            return Encoding.UTF8.GetString(Encoding.UTF8.GetBytes(str));
        }

        /// <summary>
        /// Clamp the string to the given byte length preserving null termination
        /// </summary>
        /// <param name="toclamp">string to clamp</param>
        /// <param name="maxbytes">max bytes the resulting string should have (including null termination)</param>
        /// <returns>null terminated string with a byte length less or equal to <see cref="maxbytes"/></returns>
        private static string StrClampBytes(string toclamp, int maxbytes)
        {
            var str = StrToUtf8NullTerm(toclamp);
            var strbytes = Encoding.UTF8.GetBytes(str);

            if (strbytes.Length <= maxbytes)
            {
                return str;
            }

            var newstrbytes = new byte[] { };
            Array.Copy(strbytes, 0, newstrbytes, 0, maxbytes - 1);
            newstrbytes[newstrbytes.Length - 1] = 0;
            newstrbytes[newstrbytes.Length - 2] = 0;

            return Encoding.UTF8.GetString(newstrbytes);
        }

        /// <summary>
        /// Free the allocated memory for conversion to <see cref="RichPresenceStruct"/>
        /// </summary>
        internal void FreeMem()
        {
            for (var i = _buffers.Count - 1; i >= 0; i--)
            {
                Marshal.FreeHGlobal(_buffers[i]);
                _buffers.RemoveAt(i);
            }
        }
    }
}