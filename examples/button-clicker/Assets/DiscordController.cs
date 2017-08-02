using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DiscordController : MonoBehaviour {
    public DiscordRpc.EventHandlers handlers;
    public DiscordRpc.RichPresence presence;
    public string applicationId;
    public int callbackCalls;
    public int clickCounter;

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
    }

    public void DisconnectedCallback(int errorCode, string message)
    {
        ++callbackCalls;
        Debug.Log(string.Format("Discord: disconnect {0}: {1}", errorCode, message));
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
        handlers = new DiscordRpc.EventHandlers();
        handlers.readyCallback = ReadyCallback;
        handlers.disconnectedCallback = DisconnectedCallback;
        handlers.errorCallback = ErrorCallback;
        handlers.joinCallback = JoinCallback;
        handlers.spectateCallback = SpectateCallback;
    }
	
	void Update () {

    }

    void OnEnable()
    {
        var p = System.Diagnostics.Process.GetCurrentProcess();
        Debug.Log(string.Format("Discord: init {0}", p.Id));
        callbackCalls = 0;
        DiscordRpc.Initialize(applicationId, ref handlers, true);
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
