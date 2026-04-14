// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_ModularMovementComponent.h"

#include "XyloMovementUtil.h"
#include "GameFramework/Character.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"
#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/JumpProfile/XMoveU_JumpProfile.h"
#include "ModularMovement/LayeredMovementMode/XMoveU_LayeredMovementMode.h"
#include "ModularMovement/LayeredMovementMode/XMoveU_RegisteredLayeredMovementMode.h"
#include "ModularMovement/MovementMode/XMoveU_MovementMode.h"
#include "ModularMovement/MovementMode/XMoveU_RegisteredMovementMode.h"
#include "ModularMovement/MovementSyncedObject/XMoveU_MovementSyncedObjectInterface.h"


DEFINE_LOG_CATEGORY_STATIC(LogNavMeshMovement, Log, All);

DECLARE_CYCLE_STAT(TEXT("Char ProcessLanded"), STAT_CharProcessLanded, STATGROUP_Character);


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
	OnJumpProfileSet(nullptr);
}

void UXMoveU_ModularMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	UpdateJumpBeforeMovement(DeltaSeconds);
	UpdateCrouchBeforeMovement(DeltaSeconds);

	TickSyncedObjectsBeforeMovement(DeltaSeconds);
}

void UXMoveU_ModularMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	UpdateCrouchAfterMovement(DeltaSeconds);

	TickSyncedObjectsAfterMovement(DeltaSeconds);
}

void UXMoveU_ModularMovementComponent::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);
}

void UXMoveU_ModularMovementComponent::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);
}

/*====================================================================================================================*/
// CustomMovementModesIntegration

bool UXMoveU_ModularMovementComponent::IsFlying() const
{
	// @XMoveU - @SameAsSuper
	if (!UpdatedComponent) return false;
	if (MovementMode == MOVE_Flying) return true;
	// ~@XMoveU - @SameAsSuper
	
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->IsFlyingMode() : false;
}

bool UXMoveU_ModularMovementComponent::IsFalling() const
{
	// @XMoveU - @SameAsSuper
	if (!UpdatedComponent) return false;
	if (MovementMode == MOVE_Falling) return true;
	// ~@XMoveU - @SameAsSuper
	
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->IsFallingMode() : false;
}

bool UXMoveU_ModularMovementComponent::IsSwimming() const
{
	// @XMoveU - @SameAsSuper
	if (!UpdatedComponent) return false;
	if (MovementMode == MOVE_Swimming) return true;
	// ~@XMoveU - @SameAsSuper
	
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->IsSwimmingMode() : false;
}

bool UXMoveU_ModularMovementComponent::IsMovingOnGround() const
{
	// @XMoveU - @SameAsSuper
	if (!UpdatedComponent) return false;
	if (MovementMode == MOVE_NavWalking || MovementMode == MOVE_Walking) return true;
	// ~@XMoveU - @SameAsSuper
	
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->IsGroundMode() : false;
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

bool UXMoveU_ModularMovementComponent::CanCrouchInCurrentState() const
{
	// @XMoveU - @SameAsSuper
	if (!CanEverCrouch()) return false;
	if (!UpdatedComponent || UpdatedComponent->IsSimulatingPhysics()) return false;

	if (MovementMode != MOVE_Custom)
	{
		return IsMovingOnGround() || IsFalling();
	}
	// ~@XMoveU - @SameAsSuper
	
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->CanCrouchInCurrentMode() : false;
}

// ~CustomMovementModesIntegration
/*====================================================================================================================*/

/*====================================================================================================================*/
// HooksForImprovedInterface

bool UXMoveU_ModularMovementComponent::CanAttemptJump() const
{
	// @XMoveU - @SameAsSuper
	if (!IsJumpAllowed()) return false;
	return CanJumpInCurrentState(); // ~@XMoveU - @Change: delegating mode dependent check to other function.
	// ~@XMoveU - @SameAsSuper
}

bool UXMoveU_ModularMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
	// @XMoveU - @CopiedFromSuper
	if ( CharacterOwner && CharacterOwner->CanJump() )
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || !FMath::IsNearlyEqual(FMath::Abs(GetGravitySpaceZ(PlaneConstraintNormal)), 1.f))
		{
			// @XMoveU - @Change: Delegated Jump logic to ApplyJumpImpulse, and allowing override of post jump movement mode
			if (ApplyJumpImpulse(bReplayingMoves, DeltaTime))
			{
				EvaluatePostJumpedTransitions();
				return true;
			}
			// ~@XMoveU - @Change
		}
	}
	
	return false;
	// ~@XMoveU - @CopiedFromSuper
}

void UXMoveU_ModularMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	// @XMoveU - @CopiedFromSuper
	SCOPE_CYCLE_COUNTER(STAT_CharProcessLanded);

	OnLanded(Hit, remainingTime, Iterations); // @XMoveU - @Change
	
	if( CharacterOwner && CharacterOwner->ShouldNotifyLanded(Hit) )
	{
		CharacterOwner->Landed(Hit);
	}
	if( IsFalling() )
	{
		if (GetGroundMovementMode() == MOVE_NavWalking)
		{
			// verify navmesh projection and current floor
			// otherwise movement will be stuck in infinite loop:
			// navwalking -> (no navmesh) -> falling -> (standing on something) -> navwalking -> ....

			const FVector TestLocation = GetActorFeetLocation();
			FNavLocation NavLocation;

			const bool bHasNavigationData = FindNavFloor(TestLocation, NavLocation);
			if (!bHasNavigationData || NavLocation.NodeRef == INVALID_NAVNODEREF)
			{
				SetGroundMovementMode(MOVE_Walking);
				UE_LOG(LogNavMeshMovement, Verbose, TEXT("ProcessLanded(): %s tried to go to NavWalking but couldn't find NavMesh! Using Walking instead."), *GetNameSafe(CharacterOwner));
			}
		}

		SetPostLandedPhysics(Hit);
	}

	PostLanded(Hit, remainingTime, Iterations); // @XMoveU - @Change
	
	IPathFollowingAgentInterface* PFAgent = GetPathFollowingAgent();
	if (PFAgent)
	{
		PFAgent->OnLanded();
	}

	StartNewPhysics(remainingTime, Iterations);
	// ~@XMoveU - @CopiedFromSuper
}

void UXMoveU_ModularMovementComponent::SetPostLandedPhysics(const FHitResult& Hit)
{
	// @XMoveU - @SameAsSuper
	if (!CharacterOwner)
	{
		return;
	}

	const FVector PreImpactAccel = Acceleration + (IsFalling() ? -GetGravityDirection() * GetGravityZ() : FVector::ZeroVector);
	const FVector PreImpactVelocity = Velocity;

	EvaluatePostLandedTransitions(Hit); // @XMoveU - @Change: moved transition logic to its own function

	if (IsMovingOnGround())
	{
		ApplyImpactPhysicsForces(Hit, PreImpactAccel, PreImpactVelocity);
	}
	// ~@XMoveU - @SameAsSuper
}

// ~HooksForImprovedInterface
/*====================================================================================================================*/

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
		if (IsValid(RegisteredMoveMode.Mode))
		{
			UXMoveU_PredictionManager* PredictionManager = RegisteredMoveMode.Mode->GetPredictionManager();
			if (IsValid(PredictionManager))
			{
				OutPredictionManagers.Add(PredictionManager);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXMoveU_ModularMovementComponent
 */

void UXMoveU_ModularMovementComponent::UpdateJumpBeforeMovement(float DeltaSeconds)
{
	// Do not use custom CheckJumpInput if default one is requested.
	AXMoveU_ModularCharacter* Char = Cast<AXMoveU_ModularCharacter>(CharacterOwner);
	if (Char->ShouldUseDefaultCheckJumpInput()) return;
	
	// Proxies do not check jump input in default cmc, neither should we.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		Char->CheckJumpInputSynced(DeltaSeconds);
	}
}

void UXMoveU_ModularMovementComponent::UpdateCrouchBeforeMovement(float DeltaSeconds)
{
	// @XMoveU - @CopiedFromSuper::UpdateCharacterStateBeforeMovement
	
	// Proxies get replicated crouch state.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Check for a change in crouch state. Players toggle crouch by changing bWantsToCrouch.
		const bool bIsCrouching = IsCrouching();
		if (bIsCrouching && (!bWantsToCrouch || !CanCrouchInCurrentState()))
		{
			UnCrouch(false);
		}
		else if (!bIsCrouching && bWantsToCrouch && CanCrouchInCurrentState())
		{
			Crouch(false);
		}
	}
	// ~@XMoveU - @CopiedFromSuper
}

void UXMoveU_ModularMovementComponent::UpdateCrouchAfterMovement(float DeltaSeconds)
{
	// @XMoveU - @CopiedFromSuper::UpdateCharacterStateAfterMovement
	
	// Proxies get replicated crouch state.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Uncrouch if no longer allowed to be crouched
		if (IsCrouching() && !CanCrouchInCurrentState())
		{
			UnCrouch(false);
		}
	}
	// ~@XMoveU - @CopiedFromSuper
}

/*====================================================================================================================*/
// ImprovedInterface

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
		// @XMoveU - @CopiedFromSuper::CanAttemptJump: but added CanJumpWhileCrouched
		return (!bWantsToCrouch || CanJumpWhileCrouched()) && (IsMovingOnGround() || IsFalling());
		// ~@XMoveU - @CopiedFromSuper
	}
	
	UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
	return CurrentMovementMode ? CurrentMovementMode->CanJumpInCurrentMode() : false;
}

bool UXMoveU_ModularMovementComponent::ApplyJumpImpulse(bool bReplayingMoves, float DeltaTime)
{
	// First time jump is called.
	// We are using JumpKeyHoldTime instead of "JumpCurrentCountPreJump == 0" so it gets called every time jump input
	// is pressed. It is up to JumpProfile to filter further.
	if (CharacterOwner->JumpKeyHoldTime == 0.f)
	{
		if (JumpProfile && JumpProfile->OverrideInitialImpulse())
		{
			return JumpProfile->JumpInitialImpulse(bReplayingMoves, DeltaTime);
		}
		return JumpInitialImpulse(bReplayingMoves, DeltaTime);
	}

	// Successive iterations
	if (JumpProfile && JumpProfile->OverrideSustainImpulse())
	{
		return JumpProfile->JumpSustainImpulse(bReplayingMoves, DeltaTime);
	}
	return JumpSustainImpulse(bReplayingMoves, DeltaTime);
}

bool UXMoveU_ModularMovementComponent::JumpInitialImpulse(bool bReplayingMoves, float DeltaTime)
{
	// @XMoveU - @SameAsSuper::DoJump: only handling the first jump case
	const bool bFirstJump = (CharacterOwner->JumpCurrentCountPreJump == 0);
	if (bFirstJump)
	{
		const FVector::FReal NewVerticalVelocity = FMath::Max<FVector::FReal>(HasCustomGravity() ? GetGravitySpaceZ(Velocity) : Velocity.Z, JumpZVelocity);
		UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(this, FVector::ZeroVector, NewVerticalVelocity, 0.f, true);
	}
	return true;
}

bool UXMoveU_ModularMovementComponent::JumpSustainImpulse(bool bReplayingMoves, float DeltaTime)
{
	// @XMoveU - @SameAsSuper::DoJump: only handling the sustain case
	if (bDontFallBelowJumpZVelocityDuringJump)
	{
		const FVector::FReal NewVerticalVelocity = FMath::Max<FVector::FReal>(HasCustomGravity() ? GetGravitySpaceZ(Velocity) : Velocity.Z, JumpZVelocity);
		UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(this, FVector::ZeroVector, NewVerticalVelocity, 0.f, true);
	}
	return true;
}

bool UXMoveU_ModularMovementComponent::EvaluatePostJumpedTransitions()
{
	SetMovementMode(MOVE_Falling);
	return true;
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
	// ~@XMoveU - @CopiedFromSuper::SetPostLandedPhysics
	
	return true;
}

// ~ImprovedInterface
/*====================================================================================================================*/

/*====================================================================================================================*/
// JumpProfiles

void UXMoveU_ModularMovementComponent::SetJumpProfileByClass(TSubclassOf<UXMoveU_JumpProfile> JumpProfileClass)
{
	if (!ensureMsgf(JumpProfileClass, TEXT("UXMoveU_ModularMovementComponent::SetJumpProfileByClass >> JumpProfileClass not valid")))
	{
		return;
	}

	UXMoveU_JumpProfile* OldJumpProfile = JumpProfile;
	JumpProfile = NewObject<UXMoveU_JumpProfile>(this, JumpProfileClass);
	OnJumpProfileSet(OldJumpProfile);
}

void UXMoveU_ModularMovementComponent::SetJumpProfileFromPreset(UXMoveU_JumpProfile* JumpProfilePreset)
{
	if (!ensureMsgf(IsValid(JumpProfilePreset), TEXT("UXMoveU_ModularMovementComponent::SetJumpProfileFromPreset >> JumpProfilePreset not valid")))
	{
		return;
	}

	UXMoveU_JumpProfile* OldJumpProfile = JumpProfile;
	JumpProfile = DuplicateObject(JumpProfilePreset, this);
	OnJumpProfileSet(OldJumpProfile);
}

void UXMoveU_ModularMovementComponent::ClearJumpProfile()
{
	UXMoveU_JumpProfile* OldJumpProfile = JumpProfile;
	JumpProfile = nullptr;
	OnJumpProfileSet(OldJumpProfile);
}

void UXMoveU_ModularMovementComponent::OnJumpProfileSet(UXMoveU_JumpProfile* OldJumpProfile)
{
	if (OldJumpProfile)
	{
		JumpProfile->RemoveJumpProfile(this);
	}
	if (IsValid(JumpProfile))
	{
		JumpProfile->ApplyJumpProfile(this);
	}
}
	
// ~JumpProfiles
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
		if (IsValid(RegisteredMoveMode.Mode))
		{
			RegisteredMoveMode.Mode->OnRegistered();
			
			UXMoveU_PredictionManager* PredictionManager = RegisteredMoveMode.Mode->GetPredictionManager();
			if (IsValid(PredictionManager))
			{
				PredictionManager->OnRegistered(this);
			}
		}
	}
}
	
// ~MovementModes
/*====================================================================================================================*/
	
/*====================================================================================================================*/
// LayeredMoves

void UXMoveU_ModularMovementComponent::ReplicateLayeredMovementModeStatesToSimProxies(uint32 OldStates)
{
	for (FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			RegisteredLayeredMove.Mode->ReplicateStateToSimProxies();
		}
	}
}

void UXMoveU_ModularMovementComponent::RegisterLayeredMovementModes()
{
	for (FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			RegisteredLayeredMove.Mode->OnRegistered(); // TODO: assign ModeIndex
			
			UXMoveU_PredictionManager* PredictionManager = RegisteredLayeredMove.Mode->GetPredictionManager();
			if (IsValid(PredictionManager))
			{
				PredictionManager->OnRegistered(this);
			}
		}
	}
}

// ~LayeredMoves
/*====================================================================================================================*/
