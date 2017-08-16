

#pragma once

#include "Engine.h"
#include "CoreMinimal.h"
#include "DiscordRpcBlueprint.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(Discord, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDiscordConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDiscordDisconnected, int, errorCode, const FString&, errorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDiscordErrored, int, errorCode, const FString&, errorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordJoin, const FString&, joinSecret);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordSpectate, const FString&, spectateSecret);

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Discord RPC"), Category = "Discord")
class DISCORDRPC_API UDiscordRpc : public UObject
{
    GENERATED_BODY()

public:
	
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize connection", Keywords = "Discord rpc"), Category = "Discord")
    void Initialize(const FString& applicationId, bool autoRegister);
	
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Shut down connection", Keywords = "Discord rpc"), Category = "Discord")
    void Shutdown();

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Check for callbacks", Keywords = "Discord rpc"), Category = "Discord")
    void RunCallbacks();

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Is Discord connected", Keywords = "Discord rpc"), Category = "Discord")
    bool IsConnected;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On connection", Keywords = "Discord rpc"), Category = "Discord")
    FDiscordConnected OnConnected;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On disconnection", Keywords = "Discord rpc"), Category = "Discord")
    FDiscordDisconnected OnDisconnected;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On error message", Keywords = "Discord rpc"), Category = "Discord")
    FDiscordErrored OnErrored;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "When Discord user presses join", Keywords = "Discord rpc"), Category = "Discord")
    FDiscordJoin OnJoin;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "When Discord user presses spectate", Keywords = "Discord rpc"), Category = "Discord")
    FDiscordSpectate OnSpectate;

};
