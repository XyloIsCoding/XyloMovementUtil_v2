// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_ModularMovementComponent.h"

#include "XyloMovementUtil.h"
#include "ModularMovement/MovementSyncedObject/XMoveU_MovementSyncedObjectInterface.h"


UXMoveU_ModularMovementComponent::UXMoveU_ModularMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UCharacterMovementComponent Interface
 */

void UXMoveU_ModularMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	TickSyncedObjectsBeforeMovement(DeltaSeconds);
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UXMoveU_ModularMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	TickSyncedObjectsAfterMovement(DeltaSeconds);
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXMoveU_ModularMovementComponent
 */

/*====================================================================================================================*/
// MovementSyncedObjects

void UXMoveU_ModularMovementComponent::RegisterMovementSyncedObject(UObject* InObject, UXMoveU_PredictionManager* InPredictionManager)
{
	if (!IsValid(InObject))
	{
		UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_ModularMovementComponent::RegisterMovementSyncedObject >> Object is not valid"))
		return;
	}
	
	IXMoveU_MovementSyncedObjectInterface* MoveSyncedObject = Cast<IXMoveU_MovementSyncedObjectInterface>(InObject);
	if (!MoveSyncedObject)
	{
		UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_ModularMovementComponent::RegisterMovementSyncedObject >> Cannot register objects not implementing IXMoveU_MovementSyncedObjectInterface"))
		return;
	}

	MovementSyncedObjects.Add(InObject);
	MoveSyncedObject->OnRegistered(this);

	// Just check for nullptr since prediction managers are optional. The register function already checks for IsValid.
	if (InPredictionManager)
	{
		RegisterPredictionManager(InPredictionManager);
	}
}

void UXMoveU_ModularMovementComponent::TickSyncedObjectsBeforeMovement(float DeltaSeconds)
{
	FXMoveU_MovementSyncParams Params;
	Params.MovementComponent = this;
	
	for (TScriptInterface<IXMoveU_MovementSyncedObjectInterface> MoveSyncObject : MovementSyncedObjects)
	{
		if (IsValid(MoveSyncObject.GetObject()))
		{
			MoveSyncObject->TickBeforeMovement(Params, DeltaSeconds);
		}
	}
}

void UXMoveU_ModularMovementComponent::TickSyncedObjectsAfterMovement(float DeltaSeconds)
{
	FXMoveU_MovementSyncParams Params;
	Params.MovementComponent = this;
	
	for (TScriptInterface<IXMoveU_MovementSyncedObjectInterface> MoveSyncObject : MovementSyncedObjects)
	{
		if (IsValid(MoveSyncObject.GetObject()))
		{
			MoveSyncObject->TickAfterMovement(Params, DeltaSeconds);
		}
	}
}

// ~MovementSyncedObjects
/*====================================================================================================================*/


