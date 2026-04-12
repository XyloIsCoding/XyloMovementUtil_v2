// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "XMoveU_RegisteredMovementMode.generated.h"

class UXMoveU_PredictionManager;
class UXMoveU_MovementMode;

/**
 * 
 */
USTRUCT()
struct XYLOMOVEMENTUTIL_API FXMoveU_RegisteredMovementMode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Instanced)
	TObjectPtr<UXMoveU_MovementMode> Mode;

	UPROPERTY(EditDefaultsOnly, Instanced)
	TObjectPtr<UXMoveU_PredictionManager> PredictionManager;
};
