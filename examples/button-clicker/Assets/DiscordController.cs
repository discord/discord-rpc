﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DiscordController : MonoBehaviour {
    public DiscordRpc.RichPresence presence;
    public string applicationId;
    public string optionalSteamId;
    public int callbackCalls;
    public int clickCounter;
    public UnityEngine.Events.UnityEvent onConnect;
    public UnityEngine.Events.UnityEvent onDisconnect;

    DiscordRpc.EventHandlers handlers;

    public void OnClick()
    {
        Debug.Log("Discord: on click!");
        clickCounter++;

        presence.details = string.Format("Button clicked {0} times", clickCounter);

        DiscordRpc.UpdatePresence(ref presence);
    }

    public void ReadyCallback()
    {
        ++callbackCalls;
        Debug.Log("Discord: ready");
        onConnect.Invoke();
    }

    public void DisconnectedCallback(int errorCode, string message)
    {
        ++callbackCalls;
        Debug.Log(string.Format("Discord: disconnect {0}: {1}", errorCode, message));
        onDisconnect.Invoke();
    }

    public void ErrorCallback(int errorCode, string message)
    {
        ++callbackCalls;
        Debug.Log(string.Format("Discord: error {0}: {1}", errorCode, message));
    }

    public void JoinCallback(string secret)
    {
        ++callbackCalls;
        Debug.Log(string.Format("Discord: join ({0})", secret));
    }

    public void SpectateCallback(string secret)
    {
        ++callbackCalls;
        Debug.Log(string.Format("Discord: spectate ({0})", secret));
    }

    void Start () {
    }
	
	void Update () {
        DiscordRpc.RunCallbacks();
    }
    
    void OnEnable()
    {
        Debug.Log("Discord: init");
        callbackCalls = 0;

        handlers = new DiscordRpc.EventHandlers();
        handlers.readyCallback = ReadyCallback;
        handlers.disconnectedCallback += DisconnectedCallback;
        handlers.errorCallback += ErrorCallback;
        handlers.joinCallback += JoinCallback;
        handlers.spectateCallback += SpectateCallback;
        DiscordRpc.Initialize(applicationId, ref handlers, true, optionalSteamId);
    }

    void OnDisable()
    {
        Debug.Log("Discord: shutdown");
        DiscordRpc.Shutdown();
    }

    void OnDestroy()
    {
        
    }
}
