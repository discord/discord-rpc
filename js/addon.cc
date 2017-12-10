#include <functional>
#include <napi.h>
#include <discord-rpc.h>
#include <discord_register.h>

Napi::FunctionReference handleReady;
Napi::FunctionReference handleDisconnected;
Napi::FunctionReference handleErrored;
Napi::FunctionReference handleJoinGame;
Napi::FunctionReference handleSpectateGame;
Napi::FunctionReference handleJoinRequest;

void callReady() {
  handleReady.Value().Call({});
}

void callDisconnected(int errorCode, const char* message) {
  Napi::Env env = handleDisconnected.Env();
  handleDisconnected.Value().Call({
    Napi::Number::New(env, errorCode),
    Napi::String::New(env, message),
  });
}

void callErrored(int errorCode, const char* message) {
  Napi::Env env = handleDisconnected.Env();
  handleErrored.Value().Call({
    Napi::Number::New(env, errorCode),
    Napi::String::New(env, message),
  });
}

void callJoinGame(const char* joinSecret) {
  Napi::Env env = handleJoinGame.Env();
  handleJoinGame.Value().Call({ Napi::String::New(env, joinSecret) });
}

void callSpectateGame(const char* spectateSecret) {
  Napi::Env env = handleSpectateGame.Env();
  handleSpectateGame.Value().Call({ Napi::String::New(env, spectateSecret) });
}

void callJoinRequest(const DiscordJoinRequest* request) {
  Napi::Env env = handleJoinRequest.Env();
  Napi::Object obj = Napi::Object::New(env);

  obj.Set("userId", Napi::String::New(env, request->userId));
  obj.Set("username", Napi::String::New(env, request->username));
  obj.Set("avatar", Napi::String::New(env, request->avatar));

  handleJoinRequest.Value().Call({ obj });
}

Napi::FunctionReference makeFuncRef(Napi::Value info) {
  return Napi::Reference<Napi::Function>::New(info.As<Napi::Function>(), 1);
}

Napi::Value Initialize(const Napi::CallbackInfo& info) {
  const char* applicationId = info[0].As<Napi::String>().Utf8Value().c_str();
  Napi::Object hds = info[1].As<Napi::Object>();
  Napi::Boolean autoRegister = info[2].As<Napi::Boolean>();
  const char* optionalSteamId = info[3].As<Napi::String>().Utf8Value().c_str();

  DiscordEventHandlers handlers;

  if (hds.Has("ready")) {
    handlers.ready = callReady;
    handleReady = makeFuncRef(hds.Get("ready"));
  }
  if (hds.Has("disconnected")) {
    handlers.disconnected = callDisconnected;
    handleDisconnected = makeFuncRef(hds.Get("disconnected"));
  }
  if (hds.Has("errored")) {
    handlers.errored = callErrored;
    handleErrored = makeFuncRef(hds.Get("errored"));
  }
  if (hds.Has("joinGame")) {
    handlers.joinGame = callJoinGame;
    handleJoinGame = makeFuncRef(hds.Get("joinGame"));
  }
  if (hds.Has("spectateGame")) {
    handlers.spectateGame = callSpectateGame;
    handleSpectateGame = makeFuncRef(hds.Get("spectateGame"));
  }
  if (hds.Has("joinRequest")) {
    handlers.joinRequest = callJoinRequest;
    handleJoinRequest = makeFuncRef(hds.Get("joinRequest"));
  }

  Discord_Initialize(applicationId, &handlers, autoRegister, optionalSteamId);

  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value Shutdown(const Napi::CallbackInfo& info) {
  Discord_Shutdown();

  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value RunCallbacks(const Napi::CallbackInfo& info) {
  Discord_RunCallbacks();

  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value UpdatePresence(const Napi::CallbackInfo& info) {
  Napi::Object obj = info[0].As<Napi::Object>();

  DiscordRichPresence presence;

  presence.state = obj.Get("state").ToString().Utf8Value().c_str();
  presence.details = obj.Get("details").ToString().Utf8Value().c_str();
  presence.startTimestamp = obj.Get("startTimestamp").ToNumber();
  presence.endTimestamp = obj.Get("endTimestamp").ToNumber();
  presence.largeImageKey = obj.Get("largeImageKey").ToString().Utf8Value().c_str();
  presence.largeImageText = obj.Get("largeImageText").ToString().Utf8Value().c_str();
  presence.smallImageKey = obj.Get("smallImageKey").ToString().Utf8Value().c_str();
  presence.smallImageText = obj.Get("smallImageText").ToString().Utf8Value().c_str();
  presence.partyId = obj.Get("partyId").ToString().Utf8Value().c_str();
  presence.partySize = obj.Get("partySize").ToNumber();
  presence.partyMax = obj.Get("partyMax").ToNumber();
  presence.matchSecret = obj.Get("matchSecret").ToString().Utf8Value().c_str();
  presence.joinSecret = obj.Get("joinSecret").ToString().Utf8Value().c_str();
  presence.spectateSecret = obj.Get("spectateSecret").ToString().Utf8Value().c_str();
  presence.instance = obj.Get("instance").ToBoolean();

  Discord_UpdatePresence(&presence);

  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value Respond(const Napi::CallbackInfo& info) {
  Napi::String userId = info[0].As<Napi::String>();
  Napi::Number reply = info[1].As<Napi::Number>();
  
  Discord_Respond(userId.Utf8Value().c_str(), reply);

  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value Register(const Napi::CallbackInfo& info) {
  const char* applicationId = info[0].As<Napi::String>().Utf8Value().c_str();
  const char* command = info[1].As<Napi::String>().Utf8Value().c_str();
  
  Discord_Register(applicationId, command);

  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value RegisterSteamGame(const Napi::CallbackInfo& info) {
  const char* applicationId = info[0].As<Napi::String>().Utf8Value().c_str();
  const char* steamId = info[1].As<Napi::String>().Utf8Value().c_str();

  Discord_RegisterSteamGame(applicationId, steamId);

  return Napi::Boolean::New(info.Env(), true);
}


Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  exports.Set("DISCORD_REPLY_YES", Napi::Number::New(env, DISCORD_REPLY_YES));
  exports.Set("DISCORD_REPLY_NO", Napi::Number::New(env, DISCORD_REPLY_NO));
  exports.Set("DISCORD_REPLY_IGNORE", Napi::Number::New(env, DISCORD_REPLY_IGNORE));

  exports.Set("initialize", Napi::Function::New(env, Initialize));
  exports.Set("shutdown", Napi::Function::New(env, Shutdown));
  exports.Set("runCallbacks", Napi::Function::New(env, RunCallbacks));
  exports.Set("updatePresence", Napi::Function::New(env, UpdatePresence));
  exports.Set("respond", Napi::Function::New(env, Respond));

  exports.Set("register", Napi::Function::New(env, Register));
  exports.Set("registerSteamGame", Napi::Function::New(env, RegisterSteamGame));

  return exports;
}

NODE_API_MODULE(addon, InitAll)
