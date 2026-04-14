// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "XMoveU_RegisteredLayeredMovementMode.generated.h"

class UXMoveU_LayeredMovementMode;

/**
 * 
 */
USTRUCT()
struct XYLOMOVEMENTUTIL_API FXMoveU_RegisteredLayeredMovementMode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Instanced)
	TObjectPtr<UXMoveU_LayeredMovementMode> Mode;
};
