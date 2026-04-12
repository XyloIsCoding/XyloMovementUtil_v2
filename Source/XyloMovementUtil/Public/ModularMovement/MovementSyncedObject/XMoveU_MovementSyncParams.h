// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "XMoveU_MovementSyncParams.generated.h"

class UXMoveU_ModularMovementComponent;

/**
 * 
 */
USTRUCT(BlueprintType)
struct XYLOMOVEMENTUTIL_API FXMoveU_MovementSyncParams
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UXMoveU_ModularMovementComponent> MovementComponent;
};
