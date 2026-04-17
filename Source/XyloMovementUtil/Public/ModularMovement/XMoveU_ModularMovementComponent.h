// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"
#include "MovementSyncedObject/XMoveU_MovementSyncedObjectInterface.h"
#include "XMoveU_ModularMovementComponent.generated.h"


class UXMoveU_SlideMoveMode;
class UXMoveU_LayeredMovementMode;
struct FXMoveU_RegisteredLayeredMovementMode;
class UXMoveU_JumpProfile;
class UXMoveU_MovementMode;
struct FXMoveU_RegisteredMovementMode;

/**
 * Information about the ground under the character. It only gets updated as needed.
 */
USTRUCT(BlueprintType)
struct XYLOMOVEMENTUTIL_API FXMoveU_CharacterGroundInfo
{
	GENERATED_BODY()

	FXMoveU_CharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

/**
 * Adds modular system for CustomMovementModes, LayeredMoves, and JumpProfiles
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYLOMOVEMENTUTIL_API UXMoveU_ModularMovementComponent : public UXMoveU_PredictionMovementComponent
{
	GENERATED_BODY()
	friend UXMoveU_SlideMoveMode;

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
	
	virtual void EvaluatePostLandedTransitions(const FHitResult& Hit);

public:
	virtual bool CanJumpWhileCrouched() const { return bCanJumpWhileCrouched; }
	
protected:
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite)
	bool bCanJumpWhileCrouched;
	
	// ~ImprovedInterface
/*====================================================================================================================*/

/*====================================================================================================================*/
	// GroundInfo

public:
	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	const FXMoveU_CharacterGroundInfo& GetGroundInfo();
protected:
	/** Cached ground info for the character.  Do not access this directly!
	 * It's only updated when accessed via GetGroundInfo(). */
	FXMoveU_CharacterGroundInfo CachedGroundInfo;

	float GroundTraceDistance = 100000.0f;

	// ~GroundInfo
/*====================================================================================================================*/
	
/*====================================================================================================================*/
	// JumpProfiles

public:
	UXMoveU_JumpProfile* GetJumpProfile() const;
	virtual void SetJumpProfileByClass(TSubclassOf<UXMoveU_JumpProfile> JumpProfileClass);
	virtual void SetJumpProfileFromPreset(UXMoveU_JumpProfile* JumpProfilePreset);
	virtual void ClearJumpProfile();
	
protected:
	virtual void OnJumpProfileSet(UXMoveU_JumpProfile* OldJumpProfile);
	
private:
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
	// CustomMovementModes

public:
	UFUNCTION(Category="Pawn|Components|CharacterMovement|CustomMovementModes", BlueprintCallable)
	UXMoveU_MovementMode* GetCurrentCustomMovementMode() const;

	UFUNCTION(Category="Pawn|Components|CharacterMovement|CustomMovementModes", BlueprintCallable)
	UXMoveU_MovementMode* GetCustomMovementMode(EMovementMode InMovementMode, uint8 InCustomMode) const;
	
	UFUNCTION(Category="Pawn|Components|CharacterMovement|CustomMovementModes", BlueprintCallable)
	UXMoveU_MovementMode* GetCustomMovementModeByTag(const FGameplayTag& MovementModeTag) const;

	UFUNCTION(Category="Pawn|Components|CharacterMovement|CustomMovementModes", BlueprintCallable)
	void SetMovementModeByTag(const FGameplayTag& MovementModeTag);

	UFUNCTION(Category="Pawn|Components|CharacterMovement|CustomMovementModes", BlueprintCallable)
	FGameplayTag GetCustomMovementModeTag(uint8 InCustomMode) const;

	UFUNCTION(Category="Pawn|Components|CharacterMovement|CustomMovementModes", BlueprintCallable)
	virtual bool IsInCustomMovementMode(const FGameplayTag& MovementModeTag) const;
	
protected:
	virtual void RegisterMovementModes();

	virtual void CheckMovementModesTransition(float DeltaSeconds);
	virtual bool CheckMovementModesPostLandedTransitions(const FHitResult& Hit);
	virtual void UpdateMovementModes(float DeltaSeconds);
	
private:
	UPROPERTY(Category="Character Movement: Custom Movement", EditDefaultsOnly)
	TArray<FXMoveU_RegisteredMovementMode> CustomMovementModes;
	
	// ~CustomMovementModes
/*====================================================================================================================*/

/*====================================================================================================================*/
	// LayeredMoves

public:
	UFUNCTION(Category="Pawn|Components|CharacterMovement|LayeredMovementModes", BlueprintCallable)
	virtual void RequestLayeredMovementMode(const FGameplayTag& LayeredMovementModeTag, bool bWantsToEnterMode);

	UFUNCTION(Category="Pawn|Components|CharacterMovement|LayeredMovementModes", BlueprintCallable)
	virtual bool IsInLayeredMovementMode(const FGameplayTag& LayeredMovementModeTag) const;
	
public:
	virtual void ReplicateLayeredMovementModeStatesToSimProxies(uint32 OldStates);
	
protected:
	virtual void RegisterLayeredMovementModes();

	virtual void CheckLayeredMovementModesTransition(float DeltaSeconds);
	virtual void TryLeaveLayeredMovementModes(float DeltaSeconds);
	virtual void UpdateLayeredMovementModes(float DeltaSeconds);

	virtual void ApplyLayeredMovementModesSpeedModifier(float& OutMaxSpeed) const;
	
public:
	UFUNCTION(Category="Pawn|Components|CharacterMovement|LayeredMovementModes", BlueprintCallable)
	virtual UXMoveU_LayeredMovementMode* GetLayeredMovementMode(const FGameplayTag& LayeredMovementModeTag) const;

	UFUNCTION(Category="Pawn|Components|CharacterMovement|LayeredMovementModes", BlueprintCallable)
	const TArray<FXMoveU_RegisteredLayeredMovementMode>& GetLayeredMovementModes() const { return LayeredMovementModes; }
	
private:
	UPROPERTY(Category="Character Movement: Custom Movement", EditDefaultsOnly)
	TArray<FXMoveU_RegisteredLayeredMovementMode> LayeredMovementModes;

	// ~LayeredMoves
/*====================================================================================================================*/
	
};
