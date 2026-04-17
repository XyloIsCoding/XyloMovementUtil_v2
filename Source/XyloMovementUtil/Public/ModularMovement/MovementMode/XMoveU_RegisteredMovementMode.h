// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "XMoveU_RegisteredMovementMode.generated.h"

class UXMoveU_MovementMode;

/**
 * 
 */
USTRUCT(BlueprintType)
struct XYLOMOVEMENTUTIL_API FXMoveU_RegisteredMovementMode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<UXMoveU_MovementMode> Mode;
};
