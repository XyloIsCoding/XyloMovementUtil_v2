// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"
#include "MovementSyncedObject/XMoveU_MovementSyncedObjectInterface.h"
#include "XMoveU_ModularMovementComponent.generated.h"


/**
 * Adds modular system for CustomMovementModes, LayeredMoves, and JumpProfiles
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYLOMOVEMENTUTIL_API UXMoveU_ModularMovementComponent : public UXMoveU_PredictionMovementComponent
{
	GENERATED_BODY()

public:
	UXMoveU_ModularMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UCharacterMovementComponent Interface
	 */
	
public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXMoveU_ModularMovementComponent
	 */

/*====================================================================================================================*/
	// MovementSyncedObjects

public:
	UFUNCTION(BlueprintCallable, Category="Pawn|Components|CharacterMovement|SyncedObjects")
	virtual void RegisterMovementSyncedObject(UObject* InObject, UXMoveU_PredictionManager* InPredictionManager);

	virtual void TickSyncedObjectsBeforeMovement(float DeltaSeconds);
	virtual void TickSyncedObjectsAfterMovement(float DeltaSeconds);
	
protected:
	UPROPERTY()
	TArray<TScriptInterface<IXMoveU_MovementSyncedObjectInterface>> MovementSyncedObjects;
	
	// ~MovementSyncedObjects
/*====================================================================================================================*/
	
	
};
