// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/MovementMode/Preset/XMoveU_SlideMoveMode.h"

#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

// Defines for build configs
#if DO_CHECK && !UE_BUILD_SHIPPING // Disable even if checks in shipping are enabled.
	#define devCode( Code )		checkCode( Code )
#else
	#define devCode(...)
#endif


UXMoveU_SlideMoveMode::UXMoveU_SlideMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MovementModeType = EXMoveU_MovementModeType::Ground;

	MaxSlidingSpeed = 0.f;
	MaxBrakingDecelerationSliding = 500.f;
	MinAnalogSlidingSpeed = 0.f;
	
	SlideEnterSpeed = 450.f;
	SlideExitSpeed = 100.f;
	InitialVelocityBoost = 400.f;
	InitialVelocityBoostFalling = 400.f;
	SlideGravityScale = 0.2f;
	SlidingFriction = 0.f;

	SlideCooldown = 2.f;
}

void UXMoveU_SlideMoveMode::OnRegistered()
{
	Super::OnRegistered();
	TimeSinceLastSlide = SlideCooldown;
}

bool UXMoveU_SlideMoveMode::ShouldEnterMode() const
{
	// Start sliding if crouching and enough speed.
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	if (MoveComp->IsCrouching() && MoveComp->IsMovingOnGround() && !IsInMode())
	{
		if (MoveComp->Velocity.SizeSquared() > FMath::Square(SlideEnterSpeed))
		{
			return true;
		}
	}
	return false;
}

bool UXMoveU_SlideMoveMode::ShouldEnterModePostLanded(const FHitResult& Hit)
{
	// Enter sliding if landing while crouched with enough speed.
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	if (MoveComp->IsFalling())
	{
		if (MoveComp->IsCrouching())
		{
			if (FVector::VectorPlaneProject(MoveComp->Velocity, Hit.ImpactNormal).SizeSquared() > FMath::Square(SlideEnterSpeed))
			{
				return true;
			}
		}
	}
	return false;
}

void UXMoveU_SlideMoveMode::OnEnteredMovementMode(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnEnteredMovementMode(PreviousMovementMode, PreviousCustomMode);
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	
	MoveComp->bCrouchMaintainsBaseLocation = true;

	// TODO @Davide: find out where does Walking mode do this before its Phys function. cause here might not be the best place
	MoveComp->FindFloor(MoveComp->UpdatedComponent->GetComponentLocation(), MoveComp->CurrentFloor, false, nullptr);
	
	// Only boost if slide cooldown has passed
	if (TimeSinceLastSlide >= SlideCooldown)
	{
		// Delete Z velocity, and project to floor
		float OldVerticalVelocity = MoveComp->Velocity.Z;
		MoveComp->MaintainHorizontalGroundVelocity();
		MaintainFloorPlaneGroundVelocity();
		
		// Add velocity boost to projected velocity
		MoveComp->Velocity += MoveComp->Velocity.GetSafeNormal() * (PreviousMovementMode != MOVE_Falling ? InitialVelocityBoost : InitialVelocityBoostFalling);

		// Add Z velocity back
		MoveComp->Velocity.Z += OldVerticalVelocity;
	}
	MaintainFloorPlaneGroundVelocity();

	TimeSinceLastSlide = 0.f;
}

void UXMoveU_SlideMoveMode::OnLeftMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	Super::OnLeftMovementMode(NewMovementMode, NewCustomMode);
}

void UXMoveU_SlideMoveMode::UpdateMode(float DeltaTime)
{
	// Increase Slide cooldown timer
	if (TimeSinceLastSlide < SlideCooldown)
	{
		TimeSinceLastSlide += DeltaTime;
	}
}

void UXMoveU_SlideMoveMode::PhysUpdate(float DeltaTime, int32 Iterations)
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();

	// @XMoveU - @SameAsSuper: basically this function is a copy of the default walking movement mode
	
	if (DeltaTime < MoveComp->MIN_TICK_TIME)
	{
		return;
	}

	if (!Character || (!Character->Controller && !MoveComp->bRunPhysicsWithNoController && !MoveComp->HasAnimRootMotion() && !MoveComp->CurrentRootMotion.HasOverrideVelocity() && (Character->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		MoveComp->Acceleration = FVector::ZeroVector;
		MoveComp->Velocity = FVector::ZeroVector;
		return;
	}

	if (!MoveComp->UpdatedComponent->IsQueryCollisionEnabled())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
		return;
	}

	// @XMoveU - @Change
	if (!MoveComp->IsCrouching())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
		return;
	}
	// ~@XMoveU - @Change

	devCode(ensureMsgf(!MoveComp->Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *MoveComp->Velocity.ToString()));
	
	MoveComp->bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = DeltaTime;

	const EMovementMode StartingMovementMode = MoveComp->MovementMode;
	const uint8 StartingCustomMovementMode = MoveComp->CustomMovementMode;
	
	// Perform the move
	while ( (remainingTime >= MoveComp->MIN_TICK_TIME) && (Iterations < MoveComp->MaxSimulationIterations) && Character && (Character->Controller || MoveComp->bRunPhysicsWithNoController || MoveComp->HasAnimRootMotion() || MoveComp->CurrentRootMotion.HasOverrideVelocity() || (Character->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		MoveComp->bJustTeleported = false;
		const float timeTick = MoveComp->GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent * const OldBase = MoveComp->GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = MoveComp->UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = MoveComp->CurrentFloor;

		MoveComp->RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainFloorPlaneGroundVelocity(); // @XMoveU - @Change
		const FVector OldVelocity = MoveComp->Velocity;
		MoveComp->Acceleration = FVector::VectorPlaneProject(MoveComp->Acceleration, -MoveComp->GetGravityDirection());

		// @XMoveU - @Change
		// Gravity in slope direction
		const FVector SlopeDownVector = FVector::VectorPlaneProject(MoveComp->GetGravityDirection(), MoveComp->CurrentFloor.HitResult.ImpactNormal);
		MoveComp->Velocity += SlopeDownVector * -MoveComp->GetGravityZ() * SlideGravityScale * DeltaTime;
		// ~@XMoveU - @Change
		
		// Apply acceleration
		if( !MoveComp->HasAnimRootMotion() && !MoveComp->CurrentRootMotion.HasOverrideVelocity() )
		{
			MoveComp->CalcVelocity(timeTick, SlidingFriction, false, MoveComp->GetMaxBrakingDeceleration()); // @XMoveU - @Change: using SlidingFriction
			devCode(ensureMsgf(!MoveComp->Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *MoveComp->Velocity.ToString()));
		}
		
		MoveComp->ApplyRootMotionToVelocity(timeTick);
		devCode(ensureMsgf(!MoveComp->Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *MoveComp->Velocity.ToString()));

		if (MoveComp->MovementMode != StartingMovementMode || MoveComp->CustomMovementMode != StartingCustomMovementMode)
		{
			// Root motion could have taken us out of our current mode
			// No movement has taken place this movement tick so we pass on full time/past iteration count
			MoveComp->StartNewPhysics(remainingTime+timeTick, Iterations-1);
			return;
		}

		// Compute move parameters
		const FVector MoveVelocity = MoveComp->Velocity;
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
			MoveComp->MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);
			
			if (MoveComp->IsSwimming()) //just entered water
			{
				MoveComp->StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
			else if (MoveComp->MovementMode != StartingMovementMode || MoveComp->CustomMovementMode != StartingCustomMovementMode)
			{
				// pawn ended up in a different mode, probably due to the step-up-and-over flow
				// let's refund the estimated unused time (if any) and keep moving in the new mode
				const float DesiredDist = Delta.Size();
				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (MoveComp->UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				MoveComp->StartNewPhysics(remainingTime,Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			MoveComp->CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			MoveComp->FindFloor(MoveComp->UpdatedComponent->GetComponentLocation(), MoveComp->CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		const bool bCheckLedges = !MoveComp->CanWalkOffLedges();
		if ( bCheckLedges && !MoveComp->CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector GravDir = MoveComp->GetGravityDirection();
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : MoveComp->GetLedgeMove(OldLocation, Delta, OldFloor);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				MoveComp->RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				MoveComp->Velocity = NewDelta/timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && MoveComp->CheckFall(OldFloor, MoveComp->CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				MoveComp->RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (MoveComp->CurrentFloor.IsWalkableFloor())
			{
				if (MoveComp->ShouldCatchAir(OldFloor, MoveComp->CurrentFloor))
				{
					MoveComp->HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (MoveComp->IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						MoveComp->StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				MoveComp->AdjustFloorHeight();
				MoveComp->SetBase(MoveComp->CurrentFloor.HitResult.Component.Get(), MoveComp->CurrentFloor.HitResult.BoneName);
			}
			else if (MoveComp->CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(MoveComp->CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + MoveComp->RotateGravityToWorld(FVector(0.f, 0.f, MoveComp->MAX_FLOOR_DIST));
				const FVector RequestedAdjustment = MoveComp->GetPenetrationAdjustment(Hit);
				MoveComp->ResolvePenetration(RequestedAdjustment, Hit, MoveComp->UpdatedComponent->GetComponentQuat());
				MoveComp->bForceNextFloorCheck = true;
			}

			// check if just entered water
			if ( MoveComp->IsSwimming() )
			{
				MoveComp->StartSwimming(OldLocation, MoveComp->Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!MoveComp->CurrentFloor.IsWalkableFloor() && !MoveComp->CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = MoveComp->bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && MoveComp->CheckFall(OldFloor, MoveComp->CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}

		// @XMoveU - @Change
		if (MoveComp->Velocity.SizeSquared() < FMath::Square(SlideExitSpeed))
		{
			MoveComp->SetMovementMode(MOVE_Walking);
			return;
		}
		// ~@XMoveU - @Change

		// Allow overlap events and such to change physics state and velocity
		if (MoveComp->IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if( !MoveComp->bJustTeleported && !MoveComp->HasAnimRootMotion() && !MoveComp->CurrentRootMotion.HasOverrideVelocity() && timeTick >= MoveComp->MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				MoveComp->Velocity = (MoveComp->UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainFloorPlaneGroundVelocity(); // @XMoveU - @Change
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (MoveComp->UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}	
	}

	if (MoveComp->IsMovingOnGround())
	{
		MaintainFloorPlaneGroundVelocity(); // @XMoveU - @Change
	}
}

void UXMoveU_SlideMoveMode::MaintainFloorPlaneGroundVelocity()
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	MoveComp->Velocity = FVector::VectorPlaneProject(MoveComp->Velocity, MoveComp->CurrentFloor.HitResult.ImpactNormal);
}
