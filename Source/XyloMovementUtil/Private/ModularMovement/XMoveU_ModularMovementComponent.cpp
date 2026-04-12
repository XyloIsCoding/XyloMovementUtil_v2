// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_ModularMovementComponent.h"

#include "XyloMovementUtil.h"
#include "GameFramework/Character.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"
#include "ModularMovement/MovementMode/XMoveU_MovementMode.h"
#include "ModularMovement/MovementMode/XMoveU_RegisteredMovementMode.h"
#include "ModularMovement/MovementSyncedObject/XMoveU_MovementSyncedObjectInterface.h"


UXMoveU_ModularMovementComponent::UXMoveU_ModularMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UCharacterMovementComponent Interface
 */

void UXMoveU_ModularMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	RegisterMovementModes();
}

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

bool UXMoveU_ModularMovementComponent::IsFlying() const
{
	if (!UpdatedComponent) return false;
	if (MovementMode == MOVE_Flying) return true;

	// @XMoveU - @Change
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->IsFlyingMode() : false;
	// ~@XMoveU - @Change
}

bool UXMoveU_ModularMovementComponent::IsFalling() const
{
	if (!UpdatedComponent) return false;
	if (MovementMode == MOVE_Falling) return true;

	// @XMoveU - @Change
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->IsFallingMode() : false;
	// ~@XMoveU - @Change
}

bool UXMoveU_ModularMovementComponent::IsSwimming() const
{
	if (!UpdatedComponent) return false;
	if (MovementMode == MOVE_Swimming) return true;

	// @XMoveU - @Change
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->IsSwimmingMode() : false;
	// ~@XMoveU - @Change
}

bool UXMoveU_ModularMovementComponent::IsMovingOnGround() const
{
	if (!UpdatedComponent) return false;
	if (MovementMode == MOVE_NavWalking || MovementMode == MOVE_Walking) return true;

	// @XMoveU - @Change
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->IsGroundMode() : false;
	// ~@XMoveU - @Change
}

float UXMoveU_ModularMovementComponent::GetMaxSpeed() const
{
	// @XMoveU - @CopiedFromSuper
	switch(MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return GetMaxSpeedWaking();
	case MOVE_Falling:
		return GetMaxSpeedFalling();
	case MOVE_Swimming:
		return GetMaxSpeedSwimming();
	case MOVE_Flying:
		return GetMaxSpeedFlying();
	case MOVE_Custom:
		{
			// @XMoveU - @Change
			UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
			return CurrentMovementMode ? CurrentMovementMode->GetModeMaxSpeed() : 0.f;
			// ~@XMoveU - @Change
		}
	case MOVE_None:
	default:
		return 0.f;
	}
	// ~@XMoveU - @CopiedFromSuper
}

float UXMoveU_ModularMovementComponent::GetMinAnalogSpeed() const
{
	// @XMoveU - @CopiedFromSuper
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
	case MOVE_Falling:
		return MinAnalogWalkSpeed;
	default:
		break; // @XMoveU - @Change
	}
	// ~@XMoveU - @CopiedFromSuper
	
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->GetModeMinAnalogSpeed() : 0.f;
}

float UXMoveU_ModularMovementComponent::GetMaxBrakingDeceleration() const
{
	// @XMoveU - @CopiedFromSuper
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return BrakingDecelerationWalking;
	case MOVE_Falling:
		return BrakingDecelerationFalling;
	case MOVE_Swimming:
		return BrakingDecelerationSwimming;
	case MOVE_Flying:
		return BrakingDecelerationFlying;
	case MOVE_Custom:
		{
			// @XMoveU - @Change
			UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
			return CurrentMovementMode ? CurrentMovementMode->GetModeMaxBrakingDeceleration() : 0.f;
			// ~@XMoveU - @Change
		}
	case MOVE_None:
	default:
		return 0.f;
	}
	// ~@XMoveU - @CopiedFromSuper
}

FString UXMoveU_ModularMovementComponent::GetMovementName() const
{
	// @XMoveU - @CopiedFromSuper
	if( CharacterOwner )
	{
		if ( CharacterOwner->GetRootComponent() && CharacterOwner->GetRootComponent()->IsSimulatingPhysics() )
		{
			return TEXT("Rigid Body");
		}
	}

	// Using character movement
	switch( MovementMode )
	{
	case MOVE_None:				return TEXT("NULL"); break;
	case MOVE_Walking:			return TEXT("Walking"); break;
	case MOVE_NavWalking:		return TEXT("NavWalking"); break;
	case MOVE_Falling:			return TEXT("Falling"); break;
	case MOVE_Swimming:			return TEXT("Swimming"); break;
	case MOVE_Flying:			return TEXT("Flying"); break;
		// @XMoveU - @Change
	case MOVE_Custom:			return GetCustomMovementModeTag(CustomMovementMode).ToString(); break;
		// ~@XMoveU - @Change
	default:					break;
	}
	return TEXT("Unknown");
	// ~@XMoveU - @CopiedFromSuper
}

void UXMoveU_ModularMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode())
	{
		CurrentMovementMode->PhysUpdate(deltaTime, Iterations);
	}
	
	Super::PhysCustom(deltaTime, Iterations);
}

void UXMoveU_ModularMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	// @XMoveU - @CopiedFromSuper
	if (!HasValidData())
	{
		return;
	}

	// Update collision settings if needed
	if (MovementMode == MOVE_NavWalking)
	{
		// Reset cached nav location used by NavWalking
		CachedNavLocation = FNavLocation();

		SetGroundMovementMode(MovementMode);
		// Walking uses only XY velocity
		Velocity = ProjectToGravityFloor(Velocity);
		SetNavWalkingPhysics(true);
	}
	else if (PreviousMovementMode == MOVE_NavWalking)
	{
		if (MovementMode == DefaultLandMovementMode || IsWalking())
		{
			const bool bSucceeded = TryToLeaveNavWalking();
			if (!bSucceeded)
			{
				return;
			}
		}
		else
		{
			SetNavWalkingPhysics(false);
		}
	}

	// React to changes in the movement mode.
	if (MovementMode == MOVE_Walking)
	{
		// Walking uses only XY velocity, and must be on a walkable floor, with a Base.
		Velocity = FVector::VectorPlaneProject(Velocity, -GetGravityDirection());
		bCrouchMaintainsBaseLocation = true;
		SetGroundMovementMode(MovementMode);

		// make sure we update our new floor/base on initial entry of the walking physics
		FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, false);
		AdjustFloorHeight();
		SetBaseFromFloor(CurrentFloor);
	}
	else
	{
		CurrentFloor.Clear();
		bCrouchMaintainsBaseLocation = false;

		if (MovementMode == MOVE_Falling)
		{
			if (bStayBasedInAir == false)
			{
				ApplyImpartedMovementBaseVelocity();
			}
			CharacterOwner->Falling();
		}

		if (!IsFalling() || bStayBasedInAir == false)
		{
			SetBase(NULL);
		}

		if (MovementMode == MOVE_None)
		{
			// Kill velocity and clear queued up events
			StopMovementKeepPathing();
			CharacterOwner->ResetJumpState();
			ClearAccumulatedForces();
		}
	}

	if (MovementMode == MOVE_Falling && PreviousMovementMode != MOVE_Falling)
	{
		IPathFollowingAgentInterface* PFAgent = GetPathFollowingAgent();
		if (PFAgent)
		{
			PFAgent->OnStartedFalling();
		}
	}

	// @XMoveU - @Change
	// Hooks for custom movement modes
	if (UXMoveU_MovementMode* PreviousMode = GetCustomMovementMode(PreviousMovementMode, PreviousCustomMode))
	{
		PreviousMode->OnLeftMovementMode(MovementMode, CustomMovementMode);
	}
	if (UXMoveU_MovementMode* NewMode = GetCustomMovementMode(MovementMode, CustomMovementMode))
	{
		NewMode->OnEnteredMovementMode(PreviousMovementMode, PreviousCustomMode);
	}
	// ~@XMoveU - @Change

	CharacterOwner->OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	ensureMsgf(GetGroundMovementMode() == MOVE_Walking || GetGroundMovementMode() == MOVE_NavWalking, TEXT("Invalid GroundMovementMode %d. MovementMode: %d, PreviousMovementMode: %d"), GetGroundMovementMode(), MovementMode.GetValue(), PreviousMovementMode);
	// ~@XMoveU - @CopiedFromSuper
}

void UXMoveU_ModularMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, remainingTime, Iterations);
}

void UXMoveU_ModularMovementComponent::SetPostLandedPhysics(const FHitResult& Hit)
{
	Super::SetPostLandedPhysics(Hit);
}

bool UXMoveU_ModularMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
	return Super::DoJump(bReplayingMoves, DeltaTime);
}


bool UXMoveU_ModularMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump();
}

bool UXMoveU_ModularMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState();
}

void UXMoveU_ModularMovementComponent::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);
}

void UXMoveU_ModularMovementComponent::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXMoveU_PredictionMovementComponent Interface
 */

void UXMoveU_ModularMovementComponent::GetPredictionManagers(TArray<UXMoveU_PredictionManager*>& OutPredictionManagers) const
{
	Super::GetPredictionManagers(OutPredictionManagers);

	// From MovementSyncedObjects
	OutPredictionManagers.Append(MovementSyncedPredictionManagers);

	// From CustomMovementModes
	for (const FXMoveU_RegisteredMovementMode& RegisteredMoveMode : CustomMovementModes)
	{
		OutPredictionManagers.Add(RegisteredMoveMode.PredictionManager);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXMoveU_ModularMovementComponent
 */

/*====================================================================================================================*/
// ModularizedExpansion

float UXMoveU_ModularMovementComponent::GetMaxSpeedWaking() const
{
	return IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed;
}

float UXMoveU_ModularMovementComponent::GetMaxSpeedFalling() const
{
	return MaxWalkSpeed;
}

float UXMoveU_ModularMovementComponent::GetMaxSpeedSwimming() const
{
	return MaxSwimSpeed;
}

float UXMoveU_ModularMovementComponent::GetMaxSpeedFlying() const
{
	return MaxFlySpeed;
}

void UXMoveU_ModularMovementComponent::UpdateJumpBeforeMovement(float DeltaSeconds)
{
}

void UXMoveU_ModularMovementComponent::UpdateCrouchBeforeMovement(float DeltaSeconds)
{
}

void UXMoveU_ModularMovementComponent::UpdateCrouchAfterMovement(float DeltaSeconds)
{
}

bool UXMoveU_ModularMovementComponent::TryJumpOverride()
{
	return false;
}

bool UXMoveU_ModularMovementComponent::ShouldSkipFirstJump()
{
	return IsFalling();
}

bool UXMoveU_ModularMovementComponent::CanJumpInCurrentState() const
{
	if (MovementMode != MOVE_Custom)
	{
		// @XMoveU - @CopiedFromSuper::CanAttemptJump
		return (!bWantsToCrouch || CanJumpWhileCrouched()) && (IsMovingOnGround() || IsFalling());
		// ~@XMoveU - @CopiedFromSuper
	}
	
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->CanJumpInCurrentMode() : false;
}

void UXMoveU_ModularMovementComponent::OnJumped()
{
}

bool UXMoveU_ModularMovementComponent::ApplyJumpImpulse(bool bReplayingMoves)
{
	// TODO: actually jump
	return true;
}

bool UXMoveU_ModularMovementComponent::EvaluatePostJumpedTransitions()
{
	SetMovementMode(MOVE_Falling);
	return true;
}

void UXMoveU_ModularMovementComponent::OnLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations)
{
}

void UXMoveU_ModularMovementComponent::PostLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations)
{
}

bool UXMoveU_ModularMovementComponent::EvaluatePostLandedTransitions(const FHitResult& Hit)
{
	// @XMoveU - @CopiedFromSuper::SetPostLandedPhysics: without applying impact forces 
	if (CanEverSwim() && IsInWater())
	{
		SetMovementMode(MOVE_Swimming);
	}
	else
	{
		if (DefaultLandMovementMode == MOVE_Walking ||
			DefaultLandMovementMode == MOVE_NavWalking ||
			DefaultLandMovementMode == MOVE_Falling)
		{
			SetMovementMode(GetGroundMovementMode());
		}
		else
		{
			SetDefaultMovementMode();
		}
	}
	// ~@XMoveU - @CopiedFromSuper
	
	return true;
}

// ~ModularizedExpansion
/*====================================================================================================================*/

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

	// Just check for nullptr since prediction managers are optional.
	if (InPredictionManager)
	{
		if (!IsValid(InPredictionManager))
		{
			UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_ModularMovementComponent::RegisterMovementSyncedObject >> PredictionManager is not valid"))
			return;
		}

		// Register Synced Prediction Manager
		MovementSyncedPredictionManagers.Add(InPredictionManager);
		InPredictionManager->OnRegistered(this);
	}
}

void UXMoveU_ModularMovementComponent::TickSyncedObjectsBeforeMovement(float DeltaSeconds)
{
	FXMoveU_MovementSyncParams Params;
	Params.MovementComponent = this;
	
	for (TWeakObjectPtr<UObject> MoveSyncObject : MovementSyncedObjects)
	{
		if (IXMoveU_MovementSyncedObjectInterface* MoveSyncedObjInterface = Cast<IXMoveU_MovementSyncedObjectInterface>(MoveSyncObject.Get()))
		{
			MoveSyncedObjInterface->TickBeforeMovement(Params, DeltaSeconds);
		}
	}
}

void UXMoveU_ModularMovementComponent::TickSyncedObjectsAfterMovement(float DeltaSeconds)
{
	FXMoveU_MovementSyncParams Params;
	Params.MovementComponent = this;
	
	for (TWeakObjectPtr<UObject> MoveSyncObject : MovementSyncedObjects)
	{
		if (IXMoveU_MovementSyncedObjectInterface* MoveSyncedObjInterface = Cast<IXMoveU_MovementSyncedObjectInterface>(MoveSyncObject.Get()))
		{
			MoveSyncedObjInterface->TickAfterMovement(Params, DeltaSeconds);
		}
	}
}

// ~MovementSyncedObjects
/*====================================================================================================================*/


/*====================================================================================================================*/
// MovementModes

UXMoveU_MovementMode* UXMoveU_ModularMovementComponent::GetCurrentCustomMovementMode() const
{
	return GetCustomMovementMode(MovementMode, CustomMovementMode);
}

UXMoveU_MovementMode* UXMoveU_ModularMovementComponent::GetCustomMovementMode(EMovementMode InMovementMode, uint8 InCustomMode) const
{
	if (InMovementMode != MOVE_Custom) return nullptr; // Not really necessary cause InCustomMode would be 0

	if (CustomMovementModes.IsValidIndex(InCustomMode - 1))
	{
		return CustomMovementModes[InCustomMode - 1].Mode;
	}

	UE_LOG(LogXyloMovementUtil, Error, TEXT("UXMoveU_ModularMovementComponent::GetCustomMovementMode >> No movement mode registered with id [%i]"), InCustomMode)
	return nullptr;
}

UXMoveU_MovementMode* UXMoveU_ModularMovementComponent::GetCustomMovementModeByTag(const FGameplayTag& MovementModeTag) const
{
	for (const FXMoveU_RegisteredMovementMode& CustomMode : CustomMovementModes)
	{
		if (MovementModeTag.MatchesTagExact(CustomMode.Tag))
		{
			return CustomMode.Mode;
		}
	}

	UE_LOG(LogXyloMovementUtil, Error, TEXT("UXMoveU_ModularMovementComponent::GetCustomMovementModeByTag >> No movement mode registered with tag [%s]"), *MovementModeTag.ToString())
	return nullptr;
}

void UXMoveU_ModularMovementComponent::SetMovementModeByTag(const FGameplayTag& MovementModeTag)
{
	for (int32 MoveIndex = 0; MoveIndex < CustomMovementModes.Num(); ++MoveIndex)
	{
		if (MovementModeTag.MatchesTagExact(CustomMovementModes[MoveIndex].Tag))
		{
			SetMovementMode(MOVE_Custom, MoveIndex + 1);
			return;
		}
	}

	UE_LOG(LogXyloMovementUtil, Error, TEXT("UXMoveU_ModularMovementComponent::SetMovementModeByTag >> No movement mode registered with tag [%s]"), *MovementModeTag.ToString())
}

FGameplayTag UXMoveU_ModularMovementComponent::GetCustomMovementModeTag(uint8 InCustomMode) const
{
	if (CustomMovementModes.IsValidIndex(InCustomMode - 1))
	{
		return CustomMovementModes[InCustomMode - 1].Tag;
	}

	UE_LOG(LogXyloMovementUtil, Error, TEXT("UXMoveU_ModularMovementComponent::GetCustomMovementModeTag >> No movement mode registered with id [%i]"), InCustomMode)
	return FGameplayTag();
}

void UXMoveU_ModularMovementComponent::RegisterMovementModes()
{
	for (FXMoveU_RegisteredMovementMode& RegisteredMoveMode : CustomMovementModes)
	{
		// TODO: check that both movement mode and prediction manager have this movement component as outer.
		if (IsValid(RegisteredMoveMode.Mode))
		{
			RegisteredMoveMode.Mode->OnRegistered();

			if (ensureMsgf(RegisteredMoveMode.Mode->GetOuter() == this, TEXT("CustomMoveMode DOES NOT HAVE RIGHT OUTER")))
			{
				UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_ModularMovementComponent::RegisterMovementModes >> CustomMoveMode has right outer"))
			}
		}

		if (IsValid(RegisteredMoveMode.PredictionManager))
		{
			RegisteredMoveMode.PredictionManager->OnRegistered(this);

			if (ensureMsgf(RegisteredMoveMode.PredictionManager->GetOuter() == this, TEXT("CustomModePredictionManager DOES NOT HAVE RIGHT OUTER")))
			{
				UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_ModularMovementComponent::RegisterMovementModes >> CustomModePredictionManager has right outer"))
			}
		}
	}
}
	
// ~MovementModes
/*====================================================================================================================*/
	

