

#pragma once

#include "Engine.h"
#include "CoreMinimal.h"
#include "DiscordRpcBlueprint.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DiscordLogCategory, Log, All);

/**
 * 
 */
UCLASS()
class DISCORDRPC_API UDiscordRpcBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Discord Initialize connection", Keywords = "Discord rpc"), Category = "Discord")
    static void Initialize(const FString& applicationId, bool autoRegister);
	
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Discord shut down connection", Keywords = "Discord rpc"), Category = "Discord")
    static void Shutdown();

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Discord check for callbacks", Keywords = "Discord rpc"), Category = "Discord")
    static void RunCallbacks();
};
