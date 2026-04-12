// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "XMoveU_MovementSyncParams.h"
#include "UObject/Interface.h"
#include "XMoveU_MovementSyncedObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UXMoveU_MovementSyncedObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOMOVEMENTUTIL_API IXMoveU_MovementSyncedObjectInterface
{
	GENERATED_BODY()

public:
	virtual void OnRegistered(UXMoveU_ModularMovementComponent* OwningMovementComponent) {}
	virtual void TickBeforeMovement(const FXMoveU_MovementSyncParams& Params, float DeltaSeconds) {}
	virtual void TickAfterMovement(const FXMoveU_MovementSyncParams& Params, float DeltaSeconds) {}
};
