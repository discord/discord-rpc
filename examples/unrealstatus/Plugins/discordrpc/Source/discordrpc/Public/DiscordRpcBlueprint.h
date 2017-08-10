

#pragma once

#include "Engine.h"
#include "CoreMinimal.h"
#include "DiscordRpcBlueprint.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(Discord, Log, All);

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Discord RPC"), Category = "Discord")
class DISCORDRPC_API UDiscordRpc : public UObject
{
    GENERATED_BODY()

public:
	
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Discord initialize connection", Keywords = "Discord rpc"), Category = "Discord")
    void Initialize(const FString& applicationId, bool autoRegister);
	
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Discord shut down connection", Keywords = "Discord rpc"), Category = "Discord")
    void Shutdown();

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Discord check for callbacks", Keywords = "Discord rpc"), Category = "Discord")
    void RunCallbacks();

    UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Discord connected", Keywords = "Discord rpc"), Category = "Discord")
    bool IsConnected;
};
