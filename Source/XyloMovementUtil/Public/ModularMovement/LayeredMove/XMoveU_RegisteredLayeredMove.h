// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "XMoveU_RegisteredLayeredMove.generated.h"

class UXMoveU_LayeredMove;

/**
 * 
 */
USTRUCT()
struct XYLOMOVEMENTUTIL_API FXMoveU_RegisteredLayeredMove
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Instanced)
	TObjectPtr<UXMoveU_LayeredMove> Move;
};
