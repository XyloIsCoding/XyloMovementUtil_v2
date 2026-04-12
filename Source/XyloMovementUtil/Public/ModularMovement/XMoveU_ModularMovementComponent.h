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
	 * UXMoveU_PredictionMovementComponent Interface
	 */

protected:
	virtual void GetPredictionManagers(TArray<UXMoveU_PredictionManager*>& OutPredictionManagers) const override;

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
	TArray<TWeakObjectPtr<UObject>> MovementSyncedObjects;

	/** As of now we must keep a strong reference to all prediction managers, otherwise we are going to get crashes due
	 * to serialization. Furthermore, rollbacks would miss since the prediction proxy would not be there to roll back
	 * the associated values. */
	UPROPERTY()
	TArray<TObjectPtr<UXMoveU_PredictionManager>> MovementSyncedPredictionManagers; // @FutureMeProblem: find a way to let this be a weak pointer.
	
	// ~MovementSyncedObjects
/*====================================================================================================================*/
	
	
};
