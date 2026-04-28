// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/XMoveU_ModularMovementComponent.h"

#include "XyloMovementUtil.h"
#include "AI/Navigation/NavigationDataInterface.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PhysicsVolume.h"
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

DECLARE_CYCLE_STAT(TEXT("Char PhysWalking"), STAT_CharPhysWalking, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysFalling"), STAT_CharPhysFalling, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char PhysNavWalking"), STAT_CharPhysNavWalking, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char NavProjectPoint"), STAT_CharNavProjectPoint, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char NavProjectLocation"), STAT_CharNavProjectLocation, STATGROUP_Character);
DECLARE_CYCLE_STAT(TEXT("Char ProcessLanded"), STAT_CharProcessLanded, STATGROUP_Character);

// Defines for build configs
#if DO_CHECK && !UE_BUILD_SHIPPING // Disable even if checks in shipping are enabled.
#define devCode( Code )		checkCode( Code )
#else
#define devCode(...)
#endif

// Caches CVars from base class and provides getters
// @XMoveU - @AfterUpdatingEngine: check that CVars did not change.
namespace CharacterMovementCVars
{
	static IConsoleVariable* CVar_bLedgeMovementApplyDirectMove = IConsoleManager::Get().FindConsoleVariable(TEXT("p.LedgeMovement.ApplyDirectMove"));
	static IConsoleVariable* CVar_UseTargetVelocityOnImpact = IConsoleManager::Get().FindConsoleVariable(TEXT("p.UseTargetVelocityOnImpact"));
	static IConsoleVariable* CVar_ForceJumpPeakSubstep = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ForceJumpPeakSubstep"));
	
	bool Get_bLedgeMovementApplyDirectMove()
	{
		return CharacterMovementCVars::CVar_bLedgeMovementApplyDirectMove->GetBool();
	}

	int32 Get_UseTargetVelocityOnImpact()
	{
		return CharacterMovementCVars::CVar_UseTargetVelocityOnImpact->GetInt();
	}

	int32 Get_ForceJumpPeakSubstep()
	{
		return CharacterMovementCVars::CVar_ForceJumpPeakSubstep->GetInt();
	}
}

// @XMoveU - @AfterUpdatingEngine: check that this did not change
namespace CharacterMovementConstants
{
	// MAGIC NUMBERS
	const float XMoveU_VERTICAL_SLOPE_NORMAL_Z = 0.001f; // Slope is vertical if Abs(Normal.Z) <= this threshold. Accounts for precision problems that sometimes angle normals slightly off horizontal for vertical surface.
}


UXMoveU_ModularMovementComponent::UXMoveU_ModularMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanJumpWhileCrouched = false;
	JumpHorizontalVelocity = 300.f;

	MaxCoyoteTimeDuration = 0.3f;
	CoyoteTimeFullDurationVelocity = 800.f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UCharacterMovementComponent Interface
 */

void UXMoveU_ModularMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	AutoRegisterAttachedSyncedObjects();
	RegisterMovementModes();
	RegisterLayeredMovementModes();
	OnJumpProfileSet(nullptr);
}

void UXMoveU_ModularMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	UpdateCoyoteTimeBeforeMovement(DeltaSeconds);
	UpdateJumpBeforeMovement(DeltaSeconds);
	UpdateCrouchBeforeMovement(DeltaSeconds);
	CheckLayeredMovementModesTransition(DeltaSeconds);
	UpdateLayeredMovementModes(DeltaSeconds);
	
	UpdateMovementModes(DeltaSeconds);
	CheckMovementModesTransition(DeltaSeconds);

	TickSyncedObjectsBeforeMovement(DeltaSeconds);
}

void UXMoveU_ModularMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	UpdateCrouchAfterMovement(DeltaSeconds);
	TryLeaveLayeredMovementModes(DeltaSeconds);
	
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
	float OutMaxSpeed;
	
	// @XMoveU - @CopiedFromSuper: but setting OutMaxSpeed instead of returning
	switch(MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		OutMaxSpeed = GetMaxSpeedWaking(); break;
	case MOVE_Falling:
		OutMaxSpeed = GetMaxSpeedFalling(); break;
	case MOVE_Swimming:
		OutMaxSpeed = GetMaxSpeedSwimming(); break;
	case MOVE_Flying:
		OutMaxSpeed = GetMaxSpeedFlying(); break;
	case MOVE_Custom:
		{
			// @XMoveU - @Change
			UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
			OutMaxSpeed = CurrentMovementMode ? CurrentMovementMode->GetModeMaxSpeed() : 0.f;
			break;
			// ~@XMoveU - @Change
		}
	case MOVE_None:
	default:
		OutMaxSpeed = 0.f; break;
	}
	// ~@XMoveU - @CopiedFromSuper

	ApplyLayeredMovementModesSpeedModifier(OutMaxSpeed);
	return OutMaxSpeed;
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
	float OutMaxBrakingDeceleration;
	
	// @XMoveU - @CopiedFromSuper: but setting OutMaxBrakingDeceleration instead of returning
	switch(MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		OutMaxBrakingDeceleration = GetMaxBrakingDecelerationWaking(); break;
	case MOVE_Falling:
		OutMaxBrakingDeceleration = GetMaxBrakingDecelerationFalling(); break;
	case MOVE_Swimming:
		OutMaxBrakingDeceleration = GetMaxBrakingDecelerationSwimming(); break;
	case MOVE_Flying:
		OutMaxBrakingDeceleration = GetMaxBrakingDecelerationFlying(); break;
	case MOVE_Custom:
		{
			// @XMoveU - @Change
			UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
			OutMaxBrakingDeceleration = CurrentMovementMode ? CurrentMovementMode->GetModeMaxBrakingDeceleration() : 0.f;
			break;
			// ~@XMoveU - @Change
		}
	case MOVE_None:
	default:
		OutMaxBrakingDeceleration = 0.f; break;
	}
	// ~@XMoveU - @CopiedFromSuper

	ApplyLayeredMovementModesBrakingDecelerationModifier(OutMaxBrakingDeceleration);
	return OutMaxBrakingDeceleration;
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


	// Start coyote time
	if (MovementMode == MOVE_Falling && PreviousMovementMode != MOVE_Falling)
	{
		StartCoyoteTime();
	}
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

#pragma region DefaultMoveModes
void UXMoveU_ModularMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	// @XMoveU - @CopiedFromSuper
	SCOPE_CYCLE_COUNTER(STAT_CharPhysWalking);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
	
	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	const EMovementMode StartingMovementMode = MovementMode;
	const uint8 StartingCustomMovementMode = CustomMovementMode;

	// Perform the move
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->GetController() || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

#if UE_WITH_REMOTE_OBJECT_HANDLE
		//Scale down impact force if CharacterMoveComponent is taking multiple substeps.
		const float LastFrameDt = GetWorld()->GetDeltaSeconds();
		PhysicsForceSubsteppingFactor = timeTick / LastFrameDt;
#endif
		
		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration = FVector::VectorPlaneProject(Acceleration, -GetGravityDirection());

		// Apply acceleration
		const bool bSkipForLedgeMove = bTriedLedgeMove && CharacterMovementCVars::Get_bLedgeMovementApplyDirectMove();
		if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bSkipForLedgeMove )
		{
			CalcVelocity(timeTick, GetBrakingFriction(), false, GetMaxBrakingDeceleration()); // @XMoveU - @Change: using GetBrakingFriction
			devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
		}
		
		ApplyRootMotionToVelocity(timeTick);
		devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
		{
			// Root motion could have taken us out of our current mode
			// No movement has taken place this movement tick so we pass on full time/past iteration count
			StartNewPhysics(remainingTime+timeTick, Iterations-1);
			return;
		}

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
			else if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
			{
				// pawn ended up in a different mode, probably due to the step-up-and-over flow
				// let's refund the estimated unused time (if any) and keep moving in the new mode
				const float DesiredDist = Delta.Size();
				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const float ActualDist = ProjectToGravityFloor(UpdatedComponent->GetComponentLocation() - OldLocation).Size();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, OldFloor);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta/timeTick;
				remainingTime += timeTick;
				Iterations--;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBaseFromFloor(CurrentFloor);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + MAX_FLOOR_DIST * -GetGravityDirection();
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}


		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}	
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
	// ~@XMoveU - @CopiedFromSuper
}

void UXMoveU_ModularMovementComponent::PhysNavWalking(float deltaTime, int32 Iterations)
{
	// @XMoveU - @CopiedFromSuper
	SCOPE_CYCLE_COUNTER(STAT_CharPhysNavWalking);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if ((!CharacterOwner || !CharacterOwner->GetController()) && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	const EMovementMode StartingMovementMode = MovementMode;
	const uint8 StartingCustomMovementMode = CustomMovementMode;

	RestorePreAdditiveRootMotionVelocity();

	// Ensure velocity is horizontal.
	MaintainHorizontalGroundVelocity();
	devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysNavWalking: Velocity contains NaN before CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	//bound acceleration
	Acceleration = ProjectToGravityFloor(Acceleration);
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(deltaTime, GetBrakingFriction(), false, GetMaxBrakingDeceleration()); // @XMoveU - @Change: using GetBrakingFriction
		devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysNavWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
	}

	ApplyRootMotionToVelocity(deltaTime);

	if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
	{
		// Root motion could have taken us out of our current mode
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	Iterations++;

	const FVector DesiredMove = ProjectToGravityFloor(Velocity);

	const FVector OldLocation = GetActorFeetLocation();
	const FVector DeltaMove = DesiredMove * deltaTime;
	const bool bDeltaMoveNearlyZero = DeltaMove.IsNearlyZero();

	FVector AdjustedDest = OldLocation + DeltaMove;
	FNavLocation DestNavLocation;

	bool bSameNavLocation = false;
	if (CachedNavLocation.NodeRef != INVALID_NAVNODEREF)
	{
		if (bProjectNavMeshWalking)
		{
			const float DistSq2D = ProjectToGravityFloor(OldLocation - CachedNavLocation.Location).SizeSquared();
			const float DistZ = FMath::Abs(GetGravitySpaceZ(OldLocation - CachedNavLocation.Location));

			const float TotalCapsuleHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;
			const float ProjectionScale = (GetGravitySpaceZ(OldLocation) > GetGravitySpaceZ(CachedNavLocation.Location)) ? NavMeshProjectionHeightScaleUp : NavMeshProjectionHeightScaleDown;
			const float DistZThr = TotalCapsuleHeight * FMath::Max(0.f, ProjectionScale);

			bSameNavLocation = (DistSq2D <= UE_KINDA_SMALL_NUMBER) && (DistZ < DistZThr);
		}
		else
		{
			bSameNavLocation = CachedNavLocation.Location.Equals(OldLocation);
		}

		if (bDeltaMoveNearlyZero && bSameNavLocation)
		{
			if (const INavigationDataInterface* NavData = GetNavData())
			{
				if (!NavData->IsNodeRefValid(CachedNavLocation.NodeRef))
				{
					CachedNavLocation.NodeRef = INVALID_NAVNODEREF;
					bSameNavLocation = false;
				}
			}
		}
	}

	if (bDeltaMoveNearlyZero && bSameNavLocation)
	{
		DestNavLocation = CachedNavLocation;
		UE_LOG(LogNavMeshMovement, VeryVerbose, TEXT("%s using cached navmesh location! (bProjectNavMeshWalking = %d)"), *GetNameSafe(CharacterOwner), bProjectNavMeshWalking);
	}
	else
	{
		SCOPE_CYCLE_COUNTER(STAT_CharNavProjectPoint);

		// Start the trace from the Z location of the last valid trace.
		// Otherwise if we are projecting our location to the underlying geometry and it's far above or below the navmesh,
		// we'll follow that geometry's plane out of range of valid navigation.
		if (bSameNavLocation && bProjectNavMeshWalking)
		{
			SetGravitySpaceZ(AdjustedDest, GetGravitySpaceZ(CachedNavLocation.Location));
		}

		bool bFoundPointOnNavMesh = false;
		if (bSlideAlongNavMeshEdge)
		{
			if (const INavigationDataInterface* NavDataInterface = GetNavData())
			{
				const IPathFollowingAgentInterface* PathFollowingAgent = GetPathFollowingAgent();
				const bool bIsOnNavLink = PathFollowingAgent && PathFollowingAgent->IsFollowingNavLink();

				if (!bIsOnNavLink)
				{
					FNavLocation StartingNavFloorLocation;
					bool bHasValidCachedNavLocation = NavDataInterface->IsNodeRefValid(CachedNavLocation.NodeRef);

					// If we don't have a valid CachedNavLocation lets try finding the NavFloor where we're currently at and use that
					if (!bHasValidCachedNavLocation)
					{
						bHasValidCachedNavLocation = FindNavFloor(OldLocation, OUT StartingNavFloorLocation);
					}
					else
					{
						StartingNavFloorLocation = CachedNavLocation;
					}

					if (bHasValidCachedNavLocation)
					{
						bFoundPointOnNavMesh = NavDataInterface->FindMoveAlongSurface(StartingNavFloorLocation, AdjustedDest, OUT DestNavLocation);

						if (bFoundPointOnNavMesh)
						{
							AdjustedDest = ProjectToGravityFloor(DestNavLocation.Location) + GetGravitySpaceComponentZ(AdjustedDest);
						}
					}
				}
				else
				{
					bFoundPointOnNavMesh = FindNavFloor(AdjustedDest, DestNavLocation);
				}
			}
		}
		else
		{
			bFoundPointOnNavMesh = FindNavFloor(AdjustedDest, DestNavLocation);
		}
		
		if (!bFoundPointOnNavMesh)
		{
			SetMovementMode(MOVE_Walking);
			return;
		}

		CachedNavLocation = DestNavLocation;
	}

	if (DestNavLocation.NodeRef != INVALID_NAVNODEREF)
	{
		FVector NewLocation = ProjectToGravityFloor(AdjustedDest) + GetGravitySpaceComponentZ(DestNavLocation.Location);
		if (bProjectNavMeshWalking)
		{
			SCOPE_CYCLE_COUNTER(STAT_CharNavProjectLocation);
			const float TotalCapsuleHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;
			const float UpOffset = TotalCapsuleHeight * FMath::Max(0.f, NavMeshProjectionHeightScaleUp);
			const float DownOffset = TotalCapsuleHeight * FMath::Max(0.f, NavMeshProjectionHeightScaleDown);
			NewLocation = ProjectLocationFromNavMesh(deltaTime, OldLocation, NewLocation, UpOffset, DownOffset);
		}

		FVector AdjustedDelta = NewLocation - OldLocation;

		if (!AdjustedDelta.IsNearlyZero())
		{
			FHitResult HitResult;
			SafeMoveUpdatedComponent(AdjustedDelta, UpdatedComponent->GetComponentQuat(), bSweepWhileNavWalking, HitResult);
		}

		// Update velocity to reflect actual move
		if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasVelocity())
		{
			Velocity = (GetActorFeetLocation() - OldLocation) / deltaTime;
			MaintainHorizontalGroundVelocity();
		}

		bJustTeleported = false;
	}
	else
	{
		StartFalling(Iterations, deltaTime, deltaTime, DeltaMove, OldLocation);
	}
	// ~@XMoveU - @CopiedFromSuper
}

void UXMoveU_ModularMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	// @XMoveU - @CopiedFromSuper
	SCOPE_CYCLE_COUNTER(STAT_CharPhysFalling);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const FVector FallAcceleration = ProjectToGravityFloor(GetFallingLateralAcceleration(deltaTime));
	const bool bHasLimitedAirControl = ShouldLimitAirControl(deltaTime, FallAcceleration);

	float remainingTime = deltaTime;
	while( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) )
	{
		Iterations++;
		float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;
		
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
		bJustTeleported = false;

		const FVector OldVelocityWithRootMotion = Velocity;

		RestorePreAdditiveRootMotionVelocity();

		const FVector OldVelocity = Velocity;

		// Apply input
		const float MaxDecel = GetMaxBrakingDeceleration();
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			// Compute Velocity
			{
				// Acceleration = FallAcceleration for CalcVelocity(), but we restore it after using it.
				TGuardValue<FVector> RestoreAcceleration(Acceleration, FallAcceleration);
				if (HasCustomGravity())
				{
					Velocity = ProjectToGravityFloor(Velocity);
					const FVector GravityRelativeOffset = OldVelocity - Velocity;
					CalcVelocity(timeTick, GetBrakingFriction(), false, MaxDecel); // @XMoveU - @Change: using GetBrakingFriction
					Velocity += GravityRelativeOffset;
				}
				else
				{
					Velocity.Z = 0.f;
					CalcVelocity(timeTick, GetBrakingFriction(), false, MaxDecel); // @XMoveU - @Change: using GetBrakingFriction
					Velocity.Z = OldVelocity.Z;
				}
			}
		}

		// Compute current gravity
		const FVector Gravity = -GetGravityDirection() * GetGravityZ();
		float GravityTime = timeTick;

		// If jump is providing force, gravity may be affected.
		bool bEndingJumpForce = false;
		if (CharacterOwner->JumpForceTimeRemaining > 0.0f)
		{
			// Consume some of the force time. Only the remaining time (if any) is affected by gravity when bApplyGravityWhileJumping=false.
			const float JumpForceTime = FMath::Min(CharacterOwner->JumpForceTimeRemaining, timeTick);
			GravityTime = bApplyGravityWhileJumping ? timeTick : FMath::Max(0.0f, timeTick - JumpForceTime);
			
			// Update Character state
			CharacterOwner->JumpForceTimeRemaining -= JumpForceTime;
			if (CharacterOwner->JumpForceTimeRemaining <= 0.0f)
			{
				CharacterOwner->ResetJumpState();
				bEndingJumpForce = true;
			}
		}

		// Apply gravity
		Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);

		//UE_LOG(LogCharacterMovement, Log, TEXT("dt=(%.6f) OldLocation=(%s) OldVelocity=(%s) OldVelocityWithRootMotion=(%s) NewVelocity=(%s)"), timeTick, *(UpdatedComponent->GetComponentLocation()).ToString(), *OldVelocity.ToString(), *OldVelocityWithRootMotion.ToString(), *Velocity.ToString());
		ApplyRootMotionToVelocity(timeTick);
		DecayFormerBaseVelocity(timeTick);

		// See if we need to sub-step to exactly reach the apex. This is important for avoiding "cutting off the top" of the trajectory as framerate varies.
		const FVector::FReal GravityRelativeOldVelocityWithRootMotionZ = GetGravitySpaceZ(OldVelocityWithRootMotion);
		if (CharacterMovementCVars::Get_ForceJumpPeakSubstep() && GravityRelativeOldVelocityWithRootMotionZ > 0.f && GetGravitySpaceZ(Velocity) <= 0.f && NumJumpApexAttempts < MaxJumpApexAttemptsPerSimulation)
		{
			const FVector DerivedAccel = (Velocity - OldVelocityWithRootMotion) / timeTick;
			const FVector::FReal GravityRelativeDerivedAccelZ = GetGravitySpaceZ(DerivedAccel);
			if (!FMath::IsNearlyZero(GravityRelativeDerivedAccelZ))
			{
				const float TimeToApex = -GravityRelativeOldVelocityWithRootMotionZ / GravityRelativeDerivedAccelZ;
				
				// The time-to-apex calculation should be precise, and we want to avoid adding a substep when we are basically already at the apex from the previous iteration's work.
				const float ApexTimeMinimum = 0.0001f;
				if (TimeToApex >= ApexTimeMinimum && TimeToApex < timeTick)
				{
					const FVector ApexVelocity = OldVelocityWithRootMotion + (DerivedAccel * TimeToApex);
					if (HasCustomGravity())
					{
						Velocity = ProjectToGravityFloor(ApexVelocity); // Should be nearly zero anyway, but this makes apex notifications consistent.
					}
					else
					{
						Velocity = ApexVelocity;
						Velocity.Z = 0.f; // Should be nearly zero anyway, but this makes apex notifications consistent.
					}
					
					// We only want to move the amount of time it takes to reach the apex, and refund the unused time for next iteration.
					const float TimeToRefund = (timeTick - TimeToApex);

					remainingTime += TimeToRefund;
					timeTick = TimeToApex;
					Iterations--;
					NumJumpApexAttempts++;

					// Refund time to any active Root Motion Sources as well
					for (TSharedPtr<FRootMotionSource> RootMotionSource : CurrentRootMotion.RootMotionSources)
					{
						const float RewoundRMSTime = FMath::Max(0.0f, RootMotionSource->GetTime() - TimeToRefund);
						RootMotionSource->SetTime(RewoundRMSTime);
					}
				}
			}
		}

		if (bNotifyApex && (GetGravitySpaceZ(Velocity) < 0.f))
		{
			// Just passed jump apex since now going down
			bNotifyApex = false;
			NotifyJumpApex();
		}

		// Compute change in position (using midpoint integration method).
		FVector Adjusted = 0.5f * (OldVelocityWithRootMotion + Velocity) * timeTick;
		
		// Special handling if ending the jump force where we didn't apply gravity during the jump.
		if (bEndingJumpForce && !bApplyGravityWhileJumping)
		{
			// We had a portion of the time at constant speed then a portion with acceleration due to gravity.
			// Account for that here with a more correct change in position.
			const float NonGravityTime = FMath::Max(0.f, timeTick - GravityTime);
			Adjusted = (OldVelocityWithRootMotion * NonGravityTime) + (0.5f*(OldVelocityWithRootMotion + Velocity) * GravityTime);
		}

		// Move
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent( Adjusted, PawnRotation, true, Hit);
		
		if (!HasValidData())
		{
			return;
		}
		
		float LastMoveTimeSlice = timeTick;
		float subTimeTickRemaining = timeTick * (1.f - Hit.Time);
		
		if ( IsSwimming() ) //just entered water
		{
			remainingTime += subTimeTickRemaining;
			StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
			return;
		}
		else if ( Hit.bBlockingHit )
		{
			if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
			{
				remainingTime += subTimeTickRemaining;
				ProcessLanded(Hit, remainingTime, Iterations);
				return;
			}
			else
			{
				// Compute impact deflection based on final velocity, not integration step.
				// This allows us to compute a new velocity from the deflected vector, and ensures the full gravity effect is included in the slide result.
				Adjusted = Velocity * timeTick;

				// See if we can convert a normally invalid landing spot (based on the hit result) to a usable one.
				if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit))
				{
					const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
					FFindFloorResult FloorResult;
					FindFloor(PawnLocation, FloorResult, false);

					// Note that we only care about capsule sweep floor results, since the line trace may detect a lower walkable surface that our falling capsule wouldn't actually reach yet.
					if (!FloorResult.bLineTrace && FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
					{
						remainingTime += subTimeTickRemaining;
						ProcessLanded(FloorResult.HitResult, remainingTime, Iterations);
						return;
					}
				}

				HandleImpact(Hit, LastMoveTimeSlice, Adjusted);
				
				// If we've changed physics mode, abort.
				if (!HasValidData() || !IsFalling())
				{
					return;
				}

				// Limit air control based on what we hit.
				// We moved to the impact point using air control, but may want to deflect from there based on a limited air control acceleration.
				FVector VelocityNoAirControl = OldVelocity;
				FVector AirControlAccel = Acceleration;
				if (bHasLimitedAirControl)
				{
					// Compute VelocityNoAirControl
					{
						// Find velocity *without* acceleration.
						TGuardValue<FVector> RestoreAcceleration(Acceleration, FVector::ZeroVector);
						TGuardValue<FVector> RestoreVelocity(Velocity, OldVelocity);
						if (HasCustomGravity())
						{
							Velocity = ProjectToGravityFloor(Velocity);
							const FVector GravityRelativeOffset = OldVelocity - Velocity;
							CalcVelocity(timeTick, GetBrakingFriction(), false, MaxDecel); // @XMoveU - @Change: using GetBrakingFriction
							VelocityNoAirControl = Velocity + GravityRelativeOffset;
						}
						else
						{
							Velocity.Z = 0.f;
							CalcVelocity(timeTick, GetBrakingFriction(), false, MaxDecel); // @XMoveU - @Change: using GetBrakingFriction
							VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, OldVelocity.Z);
						}
						
						VelocityNoAirControl = NewFallVelocity(VelocityNoAirControl, Gravity, GravityTime);
					}

					const bool bCheckLandingSpot = false; // we already checked above.
					AirControlAccel = (Velocity - VelocityNoAirControl) / timeTick;
					const FVector AirControlDeltaV = LimitAirControl(LastMoveTimeSlice, AirControlAccel, Hit, bCheckLandingSpot) * LastMoveTimeSlice;
					Adjusted = (VelocityNoAirControl + AirControlDeltaV) * LastMoveTimeSlice;
				}

				const FVector OldHitNormal = Hit.Normal;
				const FVector OldHitImpactNormal = Hit.ImpactNormal;				
				FVector Delta = ComputeSlideVector(Adjusted, 1.f - Hit.Time, OldHitNormal, Hit);

				// Compute velocity after deflection (only gravity component for RootMotion)
				const UPrimitiveComponent* HitComponent = Hit.GetComponent();
				if (CharacterMovementCVars::Get_UseTargetVelocityOnImpact() && !Velocity.IsNearlyZero() && MovementBaseUtility::IsSimulatedBase(HitComponent))
				{
					const FVector ContactVelocity = MovementBaseUtility::GetMovementBaseVelocity(HitComponent, NAME_None) + MovementBaseUtility::GetMovementBaseTangentialVelocity(HitComponent, NAME_None, Hit.ImpactPoint);
					const FVector NewVelocity = Velocity - Hit.ImpactNormal * FVector::DotProduct(Velocity - ContactVelocity, Hit.ImpactNormal);
					Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? ProjectToGravityFloor(Velocity) + GetGravitySpaceComponentZ(NewVelocity) : NewVelocity;
				}
				else if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && !bJustTeleported)
				{
					const FVector NewVelocity = (Delta / subTimeTickRemaining);
					Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? ProjectToGravityFloor(Velocity) + GetGravitySpaceComponentZ(NewVelocity) : NewVelocity;
				}

				if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && (Delta | Adjusted) > 0.f)
				{
					// Move in deflected direction.
					SafeMoveUpdatedComponent( Delta, PawnRotation, true, Hit);
					
					if (Hit.bBlockingHit)
					{
						// hit second wall
						LastMoveTimeSlice = subTimeTickRemaining;
						subTimeTickRemaining = subTimeTickRemaining * (1.f - Hit.Time);

						if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
						{
							remainingTime += subTimeTickRemaining;
							ProcessLanded(Hit, remainingTime, Iterations);
							return;
						}

						HandleImpact(Hit, LastMoveTimeSlice, Delta);

						// If we've changed physics mode, abort.
						if (!HasValidData() || !IsFalling())
						{
							return;
						}

						// Act as if there was no air control on the last move when computing new deflection.
						if (bHasLimitedAirControl && GetGravitySpaceZ(Hit.Normal) > CharacterMovementConstants::XMoveU_VERTICAL_SLOPE_NORMAL_Z)
						{
							const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
							Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit);
						}

						FVector PreTwoWallDelta = Delta;
						TwoWallAdjust(Delta, Hit, OldHitNormal);

						// Limit air control, but allow a slide along the second wall.
						if (bHasLimitedAirControl)
						{
							const bool bCheckLandingSpot = false; // we already checked above.
							const FVector AirControlDeltaV = LimitAirControl(subTimeTickRemaining, AirControlAccel, Hit, bCheckLandingSpot) * subTimeTickRemaining;

							// Only allow if not back in to first wall
							if (FVector::DotProduct(AirControlDeltaV, OldHitNormal) > 0.f)
							{
								Delta += (AirControlDeltaV * subTimeTickRemaining);
							}
						}

						// Compute velocity after deflection (only gravity component for RootMotion)
						if (subTimeTickRemaining > UE_KINDA_SMALL_NUMBER && !bJustTeleported)
						{
							const FVector NewVelocity = (Delta / subTimeTickRemaining);
							Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? ProjectToGravityFloor(Velocity) + GetGravitySpaceComponentZ(NewVelocity) : NewVelocity;
						}

						// bDitch=true means that pawn is straddling two slopes, neither of which it can stand on
						bool bDitch = ( (GetGravitySpaceZ(OldHitImpactNormal) > 0.f) && (GetGravitySpaceZ(Hit.ImpactNormal) > 0.f) && (FMath::Abs(GetGravitySpaceZ(Delta)) <= UE_KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f) );
						SafeMoveUpdatedComponent( Delta, PawnRotation, true, Hit);
						if ( Hit.Time == 0.f )
						{
							// if we are stuck then try to side step
							FVector SideDelta = ProjectToGravityFloor(OldHitNormal + Hit.ImpactNormal).GetSafeNormal();
							if ( SideDelta.IsNearlyZero() )
							{
								if (HasCustomGravity())
								{
									const FVector GravityRelativeHitNormal = RotateWorldToGravity(OldHitNormal);
									SideDelta = RotateGravityToWorld(FVector(GravityRelativeHitNormal.Y, -GravityRelativeHitNormal.X, 0.f)).GetSafeNormal();
								}
								else
								{
									SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0).GetSafeNormal();	
								}
							}
							SafeMoveUpdatedComponent( SideDelta, PawnRotation, true, Hit);
						}
							
						if ( bDitch || IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit) || Hit.Time == 0.f  )
						{
							remainingTime = 0.f;
							ProcessLanded(Hit, remainingTime, Iterations);
							return;
						}
						else if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && GetGravitySpaceZ(OldHitImpactNormal) >= GetWalkableFloorZ())
						{
							// We might be in a virtual 'ditch' within our perch radius. This is rare.
							const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
							const float ZMovedDist = FMath::Abs(GetGravitySpaceZ(PawnLocation - OldLocation));
							const float MovedDist2D = ProjectToGravityFloor(PawnLocation - OldLocation).Size();
							if (ZMovedDist <= 0.2f * timeTick && MovedDist2D <= 4.f * timeTick)
							{
								FVector GravityRelativeVelocity = RotateWorldToGravity(Velocity);
								GravityRelativeVelocity.X += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
								GravityRelativeVelocity.Y += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
								GravityRelativeVelocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
								Velocity = RotateGravityToWorld(GravityRelativeVelocity);
								Delta = Velocity * timeTick;
								SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
							}
						}
					}
				}
			}
		}

		const FVector GravityProjectedVelocity = ProjectToGravityFloor(Velocity);
		if (GravityProjectedVelocity.SizeSquared() <= UE_KINDA_SMALL_NUMBER * 10.f)
		{
			Velocity = GetGravitySpaceComponentZ(Velocity);
		}
	}
	// ~@XMoveU - @CopiedFromSuper
}
#pragma endregion

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

void UXMoveU_ModularMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
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

	// From LayeredCustomMovementModes
	for (const FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			UXMoveU_PredictionManager* PredictionManager = RegisteredLayeredMove.Mode->GetPredictionManager();
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

float UXMoveU_ModularMovementComponent::GetMaxBrakingDecelerationWaking() const
{
	return BrakingDecelerationWalking;
}

float UXMoveU_ModularMovementComponent::GetMaxBrakingDecelerationFalling() const
{
	return BrakingDecelerationFalling;
}

float UXMoveU_ModularMovementComponent::GetMaxBrakingDecelerationSwimming() const
{
	return BrakingDecelerationSwimming;
}

float UXMoveU_ModularMovementComponent::GetMaxBrakingDecelerationFlying() const
{
	return BrakingDecelerationFlying;
}

float UXMoveU_ModularMovementComponent::GetBrakingFriction() const
{
	// Same concept as GetMaxBrakingDeceleration
	
	float OutMaxBrakingFriction;
	
	switch(MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		OutMaxBrakingFriction = GetGroundFriction(); break;
	case MOVE_Falling:
		OutMaxBrakingFriction = GetFallingLateralFriction(); break;
	case MOVE_Swimming:
		OutMaxBrakingFriction = GetPhysicsVolume()->FluidFriction; break;
	case MOVE_Flying:
		OutMaxBrakingFriction = GetPhysicsVolume()->FluidFriction; break;
	case MOVE_Custom:
		{
			UXMoveU_MovementMode* CurrentMovementMode = GetCurrentCustomMovementMode();
			OutMaxBrakingFriction = CurrentMovementMode ? CurrentMovementMode->GetModeBrakingFriction() : 0.f;
			break;
		}
	case MOVE_None:
	default:
		OutMaxBrakingFriction = 0.f; break;
	}
	
	ApplyLayeredMovementModesBrakingFrictionModifier(OutMaxBrakingFriction);
	return OutMaxBrakingFriction;
}

float UXMoveU_ModularMovementComponent::GetGroundFriction() const
{
	return GroundFriction;
}

float UXMoveU_ModularMovementComponent::GetFallingLateralFriction() const
{
	return FallingLateralFriction;
}

bool UXMoveU_ModularMovementComponent::IsWalkingStrict() const
{
	if (!UpdatedComponent) return false;
	return MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking;
}

bool UXMoveU_ModularMovementComponent::TryJumpOverride(float DeltaTime)
{
	if (TryReplaceJumpWithLayeredMovementModes(DeltaTime))
	{
		return true;
	}
	return false;
}

bool UXMoveU_ModularMovementComponent::ShouldSkipFirstJump()
{
	// Default cmc only checks for IsFalling, but we do not want to skip the first jump if we are in coyote time.
	return IsFalling() && !IsInCoyoteTime();
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
	UXMoveU_MovementMode* CurrentMoveMode = GetCurrentCustomMovementMode();
	UXMoveU_JumpProfile* CustomModeJumpProfile = CurrentMoveMode ? CurrentMoveMode->GetJumpProfileOverride() : nullptr;
	
	// We are using JumpKeyHoldTime instead of "JumpCurrentCountPreJump == 0" so it gets called every time jump input
	// is pressed. It is up to JumpProfile to filter further.
	if (CharacterOwner->JumpKeyHoldTime == 0.f)
	{
		// First time jump is called.
		
		if (CustomModeJumpProfile && CustomModeJumpProfile->OverrideInitialImpulse())
		{
			return CustomModeJumpProfile->JumpInitialImpulse(bReplayingMoves, DeltaTime);
		}
		
		if (JumpProfile && JumpProfile->OverrideInitialImpulse())
		{
			return JumpProfile->JumpInitialImpulse(bReplayingMoves, DeltaTime);
		}
		return JumpInitialImpulse(bReplayingMoves, DeltaTime);
	}
	else
	{
		// Successive iterations
		
		if (CustomModeJumpProfile && CustomModeJumpProfile->OverrideSustainImpulse())
		{
			return CustomModeJumpProfile->JumpSustainImpulse(bReplayingMoves, DeltaTime);
		}
	
		if (JumpProfile && JumpProfile->OverrideSustainImpulse())
		{
			return JumpProfile->JumpSustainImpulse(bReplayingMoves, DeltaTime);
		}
		return JumpSustainImpulse(bReplayingMoves, DeltaTime);	
	}
}

bool UXMoveU_ModularMovementComponent::JumpInitialImpulse(bool bReplayingMoves, float DeltaTime)
{
	// @XMoveU - @SameAsSuper::DoJump: removed check for JumpCurrentCountPreJump == 0, cause that would
	// remove the ability to double jump if bDontFallBelowJumpZVelocityDuringJump is false
	const FVector::FReal NewVerticalVelocity = FMath::Max<FVector::FReal>(HasCustomGravity() ? GetGravitySpaceZ(Velocity) : Velocity.Z, JumpZVelocity);
	UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(this, GetCurrentAcceleration() / GetMaxAcceleration(), NewVerticalVelocity, JumpHorizontalVelocity, true);
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

void UXMoveU_ModularMovementComponent::OnLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations)
{
	OnLandedDelegate.Broadcast(Hit);
}

void UXMoveU_ModularMovementComponent::PostLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations)
{
	OnPostLandedDelegate.Broadcast(Hit);
}

void UXMoveU_ModularMovementComponent::EvaluatePostLandedTransitions(const FHitResult& Hit)
{
	CheckMovementModesPostLandedTransitions(Hit);
	if (!IsFalling())
	{
		// If we transitioned out of falling, there is no need to go on with other checks.
		return;
	}
	
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
}

// ~ImprovedInterface
/*====================================================================================================================*/

/*====================================================================================================================*/
// Helpers

float UXMoveU_ModularMovementComponent::GetScaledCapsuleRadius() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UXMoveU_ModularMovementComponent::GetScaledCapsuleHalfHeight() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

FVector UXMoveU_ModularMovementComponent::GetControllerForwardVector() const
{
	return CharacterOwner->GetControlRotation().Vector();
}

FVector UXMoveU_ModularMovementComponent::GetControllerDownVector() const
{
	return CharacterOwner->GetControlRotation().RotateVector(FVector::DownVector);
}

FVector UXMoveU_ModularMovementComponent::GetControllerRightVector() const
{
	return FVector::CrossProduct(GetControllerDownVector(), GetControllerForwardVector());
}

// ~Helpers
/*====================================================================================================================*/

/*====================================================================================================================*/
// GroundInfo

const FXMoveU_CharacterGroundInfo& UXMoveU_ModularMovementComponent::GetGroundInfo()
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(XMoveU_CharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;
	return CachedGroundInfo;
}

// ~GroundInfo
/*====================================================================================================================*/

/*====================================================================================================================*/
// CoyoteTime

bool UXMoveU_ModularMovementComponent::IsInCoyoteTime() const
{
	return GetCoyoteTimeDuration() > 0.f;
}

void UXMoveU_ModularMovementComponent::SetCoyoteTimeDuration(float NewCoyoteTimeDuration)
{
	CoyoteTimeDurationLeft = FMath::Clamp(NewCoyoteTimeDuration, 0.f, MaxCoyoteTimeDuration);
}

void UXMoveU_ModularMovementComponent::SetMaxCoyoteTimeDuration(float NewMaxCoyoteTimeDuration)
{
	MaxCoyoteTimeDuration = FMath::Max(0.f, NewMaxCoyoteTimeDuration);
}

void UXMoveU_ModularMovementComponent::SetCoyoteTimeFullDurationVelocity(float NewCoyoteTimeVelocityScale)
{
	CoyoteTimeFullDurationVelocity = FMath::Max(0.f, NewCoyoteTimeVelocityScale);
}

void UXMoveU_ModularMovementComponent::StartCoyoteTime()
{
	float HorizontalVelocity = (HasCustomGravity() ? RotateWorldToGravity(Velocity) : Velocity).Size2D();
	SetCoyoteTimeDuration(GetMaxCoyoteTimeDuration() * HorizontalVelocity / GetCoyoteTimeFullDurationVelocity());
}

void UXMoveU_ModularMovementComponent::UpdateCoyoteTimeBeforeMovement(float DeltaSeconds)
{
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		SetCoyoteTimeDuration(GetCoyoteTimeDuration() - DeltaSeconds);
	}
}

// ~CoyoteTime
/*====================================================================================================================*/

/*====================================================================================================================*/
// JumpProfiles

UXMoveU_JumpProfile* UXMoveU_ModularMovementComponent::GetJumpProfile() const
{
	return IsValid(JumpProfile) ? JumpProfile : nullptr;
}

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
		OldJumpProfile->RemoveJumpProfile();
	}
	if (IsValid(JumpProfile))
	{
		JumpProfile->ApplyJumpProfile();
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

void UXMoveU_ModularMovementComponent::AutoRegisterAttachedSyncedObjects()
{
	// Register character if it implements the interface.
	if (IXMoveU_MovementSyncedObjectInterface* SyncedChar = Cast<IXMoveU_MovementSyncedObjectInterface>(GetCharacterOwner()))
	{
		RegisterMovementSyncedObject(GetCharacterOwner(), SyncedChar->GetPredictionManager());
	}

	// Register all character's component implementing the interface.
	const TSet<UActorComponent*>& CharComponents = GetCharacterOwner()->GetComponents();
	for (UActorComponent* Component : CharComponents)
	{
		if (IXMoveU_MovementSyncedObjectInterface* SyncedComponent = Cast<IXMoveU_MovementSyncedObjectInterface>(Component))
		{
			RegisterMovementSyncedObject(Component, SyncedComponent->GetPredictionManager());
		}
	}
}

// ~MovementSyncedObjects
/*====================================================================================================================*/

/*====================================================================================================================*/
// CustomMovementModes

UXMoveU_MovementMode* UXMoveU_ModularMovementComponent::GetCurrentCustomMovementMode() const
{
	return GetCustomMovementMode(MovementMode, CustomMovementMode);
}

UXMoveU_MovementMode* UXMoveU_ModularMovementComponent::GetCustomMovementMode(EMovementMode InMovementMode, uint8 InCustomMode) const
{
	if (InMovementMode != MOVE_Custom)
	{
		// Not really necessary cause InCustomMode would be 0
		return nullptr;
	}

	if (CustomMovementModes.IsValidIndex(InCustomMode - 1))
	{
		UXMoveU_MovementMode* MoveMode = CustomMovementModes[InCustomMode - 1].Mode;
		return IsValid(MoveMode) ? MoveMode : nullptr;
	}

	UE_LOG(LogXyloMovementUtil, Error, TEXT("UXMoveU_ModularMovementComponent::GetCustomMovementMode >> No movement mode registered with id [%i]"), InCustomMode)
	return nullptr;
}

UXMoveU_MovementMode* UXMoveU_ModularMovementComponent::GetCustomMovementModeByTag(FGameplayTag MovementModeTag) const
{
	for (const FXMoveU_RegisteredMovementMode& CustomMode : CustomMovementModes)
	{
		if (MovementModeTag.MatchesTagExact(CustomMode.Tag))
		{
			return IsValid(CustomMode.Mode) ? CustomMode.Mode : nullptr;
		}
	}

	UE_LOG(LogXyloMovementUtil, Error, TEXT("UXMoveU_ModularMovementComponent::GetCustomMovementModeByTag >> No movement mode registered with tag [%s]"), *MovementModeTag.ToString())
	return nullptr;
}

void UXMoveU_ModularMovementComponent::SetMovementModeByTag(FGameplayTag MovementModeTag)
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

bool UXMoveU_ModularMovementComponent::IsInCustomMovementMode(FGameplayTag MovementModeTag) const
{
	if (MovementMode != MOVE_Custom)
	{
		return false;
	}

	if (!CustomMovementModes.IsValidIndex(CustomMovementMode - 1))
	{
		return false;
	}
	
	FGameplayTag CustomModeTag = CustomMovementModes[CustomMovementMode - 1].Tag;
	return CustomModeTag.MatchesTagExact(MovementModeTag);
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
		else
		{
			UE_LOG(LogXyloMovementUtil, Error, TEXT("UXMoveU_ModularMovementComponent::RegisterMovementModes >> No MovementMode Object associated with tag [%s]"), *RegisteredMoveMode.Tag.ToString())
		}
	}
}
	
void UXMoveU_ModularMovementComponent::CheckMovementModesTransition(float DeltaSeconds)
{
	// Proxies get replicated layered modes state.
	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
		
	for (FXMoveU_RegisteredMovementMode& RegisteredMoveMode : CustomMovementModes)
	{
		if (IsValid(RegisteredMoveMode.Mode))
		{
			// Enter mode.
			if (RegisteredMoveMode.Mode->ShouldEnterMode())
			{
				SetMovementModeByTag(RegisteredMoveMode.Tag);
			}
		}
	}
}

void UXMoveU_ModularMovementComponent::CheckMovementModesPostLandedTransitions(const FHitResult& Hit)
{
	for (FXMoveU_RegisteredMovementMode& RegisteredMoveMode : CustomMovementModes)
	{
		if (IsValid(RegisteredMoveMode.Mode))
		{
			// Enter mode.
			if (RegisteredMoveMode.Mode->ShouldEnterModePostLanded(Hit))
			{
				SetMovementModeByTag(RegisteredMoveMode.Tag);
			}
		}
	}
}

void UXMoveU_ModularMovementComponent::UpdateMovementModes(float DeltaSeconds)
{
	for (FXMoveU_RegisteredMovementMode& RegisteredMoveMode : CustomMovementModes)
	{
		if (IsValid(RegisteredMoveMode.Mode))
		{
			RegisteredMoveMode.Mode->UpdateMode(DeltaSeconds);
		}
	}
}

// ~CustomMovementModes
/*====================================================================================================================*/
	
/*====================================================================================================================*/
// LayeredMoves

void UXMoveU_ModularMovementComponent::RequestLayeredMovementMode(FGameplayTag LayeredMovementModeTag, bool bWantsToEnterMode)
{
	for (FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (LayeredMovementModeTag.MatchesTagExact(RegisteredLayeredMove.Tag))
		{
			if (IsValid(RegisteredLayeredMove.Mode))
			{
				// If we want to request the mode, then check if we are allowed to. Instead, if we want to stop the
				// request, just go ahead without checks.
				if (!bWantsToEnterMode || RegisteredLayeredMove.Mode->CanRequestMode())
				{
					RegisteredLayeredMove.Mode->RequestMode(bWantsToEnterMode);
				}
			}
			return;
		}
	}

	UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_ModularMovementComponent::RequestLayeredMovementMode >> No layered movement mode registered with tag [%s]"), *LayeredMovementModeTag.ToString())
}

bool UXMoveU_ModularMovementComponent::IsInLayeredMovementMode(FGameplayTag LayeredMovementModeTag) const
{
	for (const FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (LayeredMovementModeTag.MatchesTagExact(RegisteredLayeredMove.Tag))
		{
			return IsValid(RegisteredLayeredMove.Mode) && RegisteredLayeredMove.Mode->IsInMode();
		}
	}

	UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_ModularMovementComponent::RequestLayeredMovementMode >> No layered movement mode registered with tag [%s]"), *LayeredMovementModeTag.ToString())
	return false;
}

uint32 UXMoveU_ModularMovementComponent::GetLayeredMovementModesRequests() const
{
	uint32 Output = 0;
	for (auto It = LayeredMovementModes.CreateConstIterator(); It; ++It)
	{
		const FXMoveU_RegisteredLayeredMovementMode& LayeredMoveMode = *It;
		if (IsValid(LayeredMoveMode.Mode) && LayeredMoveMode.Mode->WantsToBeInMode())
		{
			uint32 ModeIndex = It.GetIndex();
			Output |= (1 << ModeIndex);
		}
	}
	return Output;
}

void UXMoveU_ModularMovementComponent::SetLayeredMovementModesRequests(uint32 CompressedRequests)
{
	for (auto It = LayeredMovementModes.CreateConstIterator(); It; ++It)
	{
		const FXMoveU_RegisteredLayeredMovementMode& LayeredMoveMode = *It;
		if (IsValid(LayeredMoveMode.Mode))
		{
			uint32 ModeIndex = It.GetIndex();
			bool bWantsToBeInMode = ((CompressedRequests & (1 << ModeIndex)) != 0);
			LayeredMoveMode.Mode->RequestMode(bWantsToBeInMode);
		}
	}
}

void UXMoveU_ModularMovementComponent::ReplicateLayeredMovementModeStatesToSimProxies(uint32 OldStates)
{
	for (auto It = LayeredMovementModes.CreateIterator(); It; ++It)
	{
		FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove = *It;
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			uint32 ModeIndex = It.GetIndex();
			bool bWasInMode = ((OldStates & (1 << ModeIndex)) != 0);
			bool bIsInMode = RegisteredLayeredMove.Mode->IsInMode();
			if (bWasInMode != bIsInMode)
			{
				RegisteredLayeredMove.Mode->ReplicateStateToSimProxies();
			}
		}
	}
}

void UXMoveU_ModularMovementComponent::RegisterLayeredMovementModes()
{
	for (auto It = LayeredMovementModes.CreateIterator(); It; ++It)
	{
		FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove = *It;
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			RegisteredLayeredMove.Mode->OnRegistered(It.GetIndex());
			
			UXMoveU_PredictionManager* PredictionManager = RegisteredLayeredMove.Mode->GetPredictionManager();
			if (IsValid(PredictionManager))
			{
				PredictionManager->OnRegistered(this);
			}
		}
		else
		{
			UE_LOG(LogXyloMovementUtil, Error, TEXT("UXMoveU_ModularMovementComponent::RegisterLayeredMovementModes >> No LayeredMovementMode Object associated with tag [%s]"), *RegisteredLayeredMove.Tag.ToString())
		}
	}
}

void UXMoveU_ModularMovementComponent::CheckLayeredMovementModesTransition(float DeltaSeconds)
{
	// Proxies get replicated layered modes state.
	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
		
	for (FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			// Leave mode.
			if (RegisteredLayeredMove.Mode->ShouldLeaveMode(DeltaSeconds))
			{
				RegisteredLayeredMove.Mode->LeaveMode(false);
			}

			// Enter mode.
			if (RegisteredLayeredMove.Mode->ShouldEnterMode(DeltaSeconds))
			{
				RegisteredLayeredMove.Mode->EnterMode(false);
			}

			// Reset input request if necessary
			if (RegisteredLayeredMove.Mode->ShouldCancelRequestAfterTransitionCheck())
			{
				RegisteredLayeredMove.Mode->RequestMode(false);
			}
		}
	}
}

void UXMoveU_ModularMovementComponent::TryLeaveLayeredMovementModes(float DeltaSeconds)
{
	// Proxies get replicated layered modes state.
	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
	
	for (FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			// Leave mode if no longer able to be in this mode.
			if (RegisteredLayeredMove.Mode->ShouldForceLeaveMode(DeltaSeconds))
			{
				RegisteredLayeredMove.Mode->LeaveMode(false);
			}
		}
	}
}

bool UXMoveU_ModularMovementComponent::TryReplaceJumpWithLayeredMovementModes(float DeltaSeconds)
{
	// Proxies get replicated layered modes state.
	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return false;
	}
	
	for (FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			// Enter mode instead of jumping.
			if (RegisteredLayeredMove.Mode->ShouldReplaceJump(DeltaSeconds))
			{
				RegisteredLayeredMove.Mode->EnterMode(false);
				return true;
			}
		}
	}

	return false;
}

void UXMoveU_ModularMovementComponent::UpdateLayeredMovementModes(float DeltaSeconds)
{
	for (const FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode))
		{
			RegisteredLayeredMove.Mode->UpdateMode(DeltaSeconds);
		}
	}
}

void UXMoveU_ModularMovementComponent::ApplyLayeredMovementModesSpeedModifier(float& OutMaxSpeed) const
{
	for (const FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode) && RegisteredLayeredMove.Mode->IsInMode())
		{
			RegisteredLayeredMove.Mode->ModifyMaxSpeed(OutMaxSpeed);
		}
	}
}

void UXMoveU_ModularMovementComponent::ApplyLayeredMovementModesBrakingFrictionModifier(float& OutBrakingFriction) const
{
	for (const FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode) && RegisteredLayeredMove.Mode->IsInMode())
		{
			RegisteredLayeredMove.Mode->ModifyBrakingFriction(OutBrakingFriction);
		}
	}
}

void UXMoveU_ModularMovementComponent::ApplyLayeredMovementModesBrakingDecelerationModifier(float& OutBrakingDeceleration) const
{
	for (const FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (IsValid(RegisteredLayeredMove.Mode) && RegisteredLayeredMove.Mode->IsInMode())
		{
			RegisteredLayeredMove.Mode->ModifyBrakingDeceleration(OutBrakingDeceleration);
		}
	}
}

UXMoveU_LayeredMovementMode* UXMoveU_ModularMovementComponent::GetLayeredMovementMode(FGameplayTag LayeredMovementModeTag) const
{
	for (const FXMoveU_RegisteredLayeredMovementMode& RegisteredLayeredMove : LayeredMovementModes)
	{
		if (LayeredMovementModeTag.MatchesTagExact(RegisteredLayeredMove.Tag))
		{
			return IsValid(RegisteredLayeredMove.Mode) ? RegisteredLayeredMove.Mode : nullptr;
		}
	}

	UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_ModularMovementComponent::GetLayeredMovementMode >> No layered movement mode registered with tag [%s]"), *LayeredMovementModeTag.ToString())
	return nullptr;
}

// ~LayeredMoves
/*====================================================================================================================*/
