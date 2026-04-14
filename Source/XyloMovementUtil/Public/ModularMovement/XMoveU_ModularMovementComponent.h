// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"
#include "MovementSyncedObject/XMoveU_MovementSyncedObjectInterface.h"
#include "XMoveU_ModularMovementComponent.generated.h"


struct FXMoveU_RegisteredLayeredMove;
class UXMoveU_JumpProfile;
class UXMoveU_MovementMode;
struct FXMoveU_RegisteredMovementMode;

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
	virtual void BeginPlay() override;

public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

public:
	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;
	
/*====================================================================================================================*/
	// CustomMovementModesIntegration
	
public:
	virtual bool IsFlying() const override;
	virtual bool IsFalling() const override;
	virtual bool IsSwimming() const override;
	virtual bool IsMovingOnGround() const override;

public:
	virtual float GetMaxSpeed() const override;
	virtual float GetMinAnalogSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

public:
	virtual FString GetMovementName() const override;
	
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	
protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

public:
	virtual bool CanCrouchInCurrentState() const override;
	
	// ~CustomMovementModesIntegration
/*====================================================================================================================*/

/*====================================================================================================================*/
	// HooksForImprovedInterface
	
public:
	virtual bool CanAttemptJump() const override;
	virtual bool DoJump(bool bReplayingMoves, float DeltaTime) override;
	
protected:
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;
	virtual void SetPostLandedPhysics(const FHitResult& Hit) override;

	// ~HooksForImprovedInterface
/*====================================================================================================================*/
	
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

protected:
	virtual void UpdateJumpBeforeMovement(float DeltaSeconds);
	virtual void UpdateCrouchBeforeMovement(float DeltaSeconds);
	virtual void UpdateCrouchAfterMovement(float DeltaSeconds);

/*====================================================================================================================*/
	// ImprovedInterface

public:
	virtual float GetMaxSpeedWaking() const;
	virtual float GetMaxSpeedFalling() const;
	virtual float GetMaxSpeedSwimming() const;
	virtual float GetMaxSpeedFlying() const;

public:
	/** Override this function to intercept and stop the jump pipeline (for example for a mantle mechanic).
	 * @remarks: Only called if using SyncedJumpInputCheck (default behaviour). */
	virtual bool TryJumpOverride();

	/** If true, JumpCurrentCount is increased before trying to perform the first jump. Useful to prevent jumping while
	 * falling already. */
	virtual bool ShouldSkipFirstJump();
	
	virtual bool CanJumpInCurrentState() const;
	
	virtual  void OnJumped() {}
	
protected:
	virtual bool ApplyJumpImpulse(bool bReplayingMoves, float DeltaTime);

	virtual bool JumpInitialImpulse(bool bReplayingMoves, float DeltaTime);
	
	virtual bool JumpSustainImpulse(bool bReplayingMoves, float DeltaTime);

	virtual bool EvaluatePostJumpedTransitions();
	
protected:
	virtual void OnLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations) {}

	virtual void PostLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations) {}
	
	virtual bool EvaluatePostLandedTransitions(const FHitResult& Hit);

public:
	virtual bool CanJumpWhileCrouched() const { return bCanJumpWhileCrouched; }
	
protected:
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite)
	bool bCanJumpWhileCrouched;
	
	// ~ImprovedInterface
/*====================================================================================================================*/

/*====================================================================================================================*/
	// JumpProfiles

public:
	virtual void SetJumpProfileByClass(TSubclassOf<UXMoveU_JumpProfile> JumpProfileClass);
	virtual void SetJumpProfileFromPreset(UXMoveU_JumpProfile* JumpProfilePreset);
	virtual void ClearJumpProfile();
	
protected:
	virtual void OnJumpProfileSet(UXMoveU_JumpProfile* OldJumpProfile);
	
	UPROPERTY(Category="Character Movement: Jumping / Falling", EditDefaultsOnly, Instanced)
	TObjectPtr<UXMoveU_JumpProfile> JumpProfile;
	
	// ~JumpProfiles
/*====================================================================================================================*/
	
/*====================================================================================================================*/
	// MovementSyncedObjects

public:
	UFUNCTION(Category="Pawn|Components|CharacterMovement|SyncedObjects", BlueprintCallable)
	virtual void RegisterMovementSyncedObject(UObject* InObject, UXMoveU_PredictionManager* InPredictionManager);

	virtual void TickSyncedObjectsBeforeMovement(float DeltaSeconds);
	
	virtual void TickSyncedObjectsAfterMovement(float DeltaSeconds);
	
protected:
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> MovementSyncedObjects;

	/** As of now we must keep a strong reference to all prediction managers, otherwise we are going to get crashes due
	 * to serialization if they get garbage collected. Furthermore, rollbacks would miss since the prediction proxy
	 * would not be there to roll back the associated values. */
	UPROPERTY()
	TArray<TObjectPtr<UXMoveU_PredictionManager>> MovementSyncedPredictionManagers; // @XMoveU - @FutureMeProblem: find a way to let this be a weak pointer.
	
	// ~MovementSyncedObjects
/*====================================================================================================================*/

/*====================================================================================================================*/
	// MovementModes

public:
	UFUNCTION(Category="Pawn|Components|CharacterMovement|MovementModes", BlueprintCallable)
	UXMoveU_MovementMode* GetCurrentCustomMovementMode() const;

	UFUNCTION(Category="Pawn|Components|CharacterMovement|MovementModes", BlueprintCallable)
	UXMoveU_MovementMode* GetCustomMovementMode(EMovementMode InMovementMode, uint8 InCustomMode) const;
	
	UFUNCTION(Category="Pawn|Components|CharacterMovement|MovementModes", BlueprintCallable)
	UXMoveU_MovementMode* GetCustomMovementModeByTag(const FGameplayTag& MovementModeTag) const;

	UFUNCTION(Category="Pawn|Components|CharacterMovement|MovementModes", BlueprintCallable)
	void SetMovementModeByTag(const FGameplayTag& MovementModeTag);

	UFUNCTION(Category="Pawn|Components|CharacterMovement|MovementModes", BlueprintCallable)
	FGameplayTag GetCustomMovementModeTag(uint8 InCustomMode) const;
	
protected:
	virtual void RegisterMovementModes();
	
protected:
	UPROPERTY(Category="Character Movement: Custom Movement", EditDefaultsOnly)
	TArray<FXMoveU_RegisteredMovementMode> CustomMovementModes;
	
	// ~MovementModes
/*====================================================================================================================*/

/*====================================================================================================================*/
	// LayeredMoves

protected:
	virtual void RegisterLayeredMoves();
	
protected:
	UPROPERTY(Category="Character Movement: Custom Movement", EditDefaultsOnly)
	TArray<FXMoveU_RegisteredLayeredMove> LayeredMoves;

	// ~LayeredMoves
/*====================================================================================================================*/
	
};
