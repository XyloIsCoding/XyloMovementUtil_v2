// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/MovementMode/Preset/XMoveU_WallRunMoveMode.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

// Defines for build configs
#if DO_CHECK && !UE_BUILD_SHIPPING // Disable even if checks in shipping are enabled.
	#define devCode( Code )		checkCode( Code )
#else
	#define devCode(...)
#endif


UXMoveU_WallRunMoveMode::UXMoveU_WallRunMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MovementModeType = EXMoveU_MovementModeType::Ground;
}

bool UXMoveU_WallRunMoveMode::ShouldEnterMode()
{
	if (!CanWallRunInCurrentState())
	{
		return false;
	}

	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();

	// Do not start wall run if we are already below detach threshold
	float VelocityZ = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(MoveComp->Velocity) : MoveComp->Velocity.Z;
	if (VelocityZ < WallRunVerticalSpeedDetachThreshold)
	{
		return false;
	}
	
	FindWall(CurrentWall, (MoveComp->GetCurrentAcceleration().GetSafeNormal2D() + MoveComp->Velocity.GetSafeNormal2D() * 0.8).GetSafeNormal2D(), MaxWallDistance * 0.6);
	DrawDebugDirectionalArrow(GetWorld(), CurrentWall.ImpactPoint, CurrentWall.ImpactPoint + CurrentWall.Normal * 10.f, 3.f, FColor::Green, false, 2.f, 0, 2.f);
	
	return CurrentWall.IsValidBlockingHit();
}

void UXMoveU_WallRunMoveMode::OnEnteredMovementMode(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
}

void UXMoveU_WallRunMoveMode::OnLeftMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
}

void UXMoveU_WallRunMoveMode::UpdateMode(float DeltaTime)
{
}

void UXMoveU_WallRunMoveMode::PhysUpdate(float DeltaTime, int32 Iterations)
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	
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
	
	if (MoveComp->IsCrouching())
	{
		MoveComp->SetMovementMode(MOVE_Falling);
		MoveComp->StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	devCode(ensureMsgf(!MoveComp->Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *MoveComp->Velocity.ToString()));
	
	MoveComp->bJustTeleported = false;
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

		
		MoveComp->RestorePreAdditiveRootMotionVelocity();

		// Find wall
		FindWall(CurrentWall, CurrentWall.ImpactPoint - CurrentWall.TraceStart, MaxWallDistance);
		DrawDebugDirectionalArrow(GetWorld(), CurrentWall.ImpactPoint, CurrentWall.ImpactPoint + CurrentWall.Normal * 10.f, 3.f, FColor::Red, false, 0.1f, 0, 2.f);

		
		if (!CurrentWall.IsValidBlockingHit())
		{
			MoveComp->SetMovementMode(MOVE_Falling);
			MoveComp->StartNewPhysics(remainingTime, Iterations);
			return;
		}

		// If acceleration points away from the wall, quit wall run
		if (!MoveComp->GetCurrentAcceleration().IsNearlyZero() && (CurrentWall.Normal | MoveComp->GetCurrentAcceleration().GetSafeNormal2D()) > WallRunLeaveAngleCosine)
		{
			MoveComp->SetMovementMode(MOVE_Falling);
			MoveComp->StartNewPhysics(remainingTime, Iterations);
			return;
		}
		
		// Ensure velocity is aligned with wall.
		MaintainWallPlaneVelocity();
		const FVector OldVelocity = MoveComp->Velocity;
		const FVector OldAcceleration = MoveComp->Acceleration;
		MoveComp->Acceleration = FVector::VectorPlaneProject(MoveComp->Acceleration, CurrentWall.Normal); 

		// Apply acceleration
		if( !MoveComp->HasAnimRootMotion() && !MoveComp->CurrentRootMotion.HasOverrideVelocity() )
		{
			MoveComp->CalcVelocity(timeTick, MoveComp->GetBrakingFriction(), false, MoveComp->GetMaxBrakingDeceleration()); 
			devCode(ensureMsgf(!MoveComp->Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *MoveComp->Velocity.ToString()));
		}

		// Restore acceleration
		MoveComp->Acceleration = OldAcceleration;
		
		// Compute current gravity
		const FVector Gravity = -MoveComp->GetGravityDirection() * MoveComp->GetGravityZ() * WallRunGravityScale;
		float GravityTime = timeTick;

		// Apply gravity
		MoveComp->Velocity = MoveComp->NewFallVelocity(MoveComp->Velocity, Gravity, GravityTime);
		
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
		FHitResult Hit;

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveComp->SafeMoveUpdatedComponent(Delta, MoveComp->UpdatedComponent->GetComponentQuat(), true, Hit);
			FVector WallAttractionDelta = -CurrentWall.Normal * WallAttractionForce * timeTick;
			MoveComp->SafeMoveUpdatedComponent(WallAttractionDelta, MoveComp->UpdatedComponent->GetComponentQuat(), true, Hit);
			
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

		// check if just entered water
		if ( MoveComp->IsSwimming() )
		{
			MoveComp->StartSwimming(OldLocation, MoveComp->Velocity, timeTick, remainingTime, Iterations);
			return;
		}

		
		// See if we need to start falling.
		FindWall(CurrentWall, CurrentWall.ImpactPoint - CurrentWall.TraceStart, MaxWallDistance);
		if (!CurrentWall.IsValidBlockingHit())
		{
			MoveComp->SetMovementMode(MOVE_Falling);
			MoveComp->StartNewPhysics(remainingTime, Iterations);
			return;
		}
		
		
		// Detach from wall if Z velocity is too low
		float VelocityZ = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(MoveComp->Velocity) : MoveComp->Velocity.Z;
		if (VelocityZ < WallRunVerticalSpeedDetachThreshold)
		{
			MoveComp->SetMovementMode(MOVE_Falling);
			MoveComp->StartNewPhysics(remainingTime, Iterations);
			return;
		}

		// TODO: check if we need to land

		// Allow overlap events and such to change physics state and velocity
		if (MoveComp->IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if( !MoveComp->bJustTeleported && !MoveComp->HasAnimRootMotion() && !MoveComp->CurrentRootMotion.HasOverrideVelocity() && timeTick >= MoveComp->MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				MoveComp->Velocity = (MoveComp->UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainWallPlaneVelocity(); 
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
		MaintainWallPlaneVelocity();
	}
}

bool UXMoveU_WallRunMoveMode::CanWallRunInCurrentState() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return !IsInMode() && MoveComp->IsFalling() && !MoveComp->IsCrouching(); 
}

bool UXMoveU_WallRunMoveMode::FindWall(FHitResult& OutWallHit, const FVector& Direction, float Distance)
{
	if (Direction.IsNearlyZero())
	{
		OutWallHit.Reset();
		return false;
	}
	
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();

	const UCapsuleComponent* CapsuleComp = MoveComp->CharacterOwner->GetCapsuleComponent();
	
	const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	const ECollisionChannel CollisionChannel = (MoveComp->UpdatedComponent ? MoveComp->UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
	const FVector TraceStart = MoveComp->GetActorLocation();
	const FVector TraceEnd = TraceStart + Direction * (MoveComp->GetScaledCapsuleRadius() + Distance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(XMoveU_CharacterMovementComponent_GetGroundInfo), false, MoveComp->CharacterOwner);
	FCollisionResponseParams ResponseParam;
	MoveComp->InitCollisionParams(QueryParams, ResponseParam);

	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(0.3f);

	// TODO: use multiple line traces and average them out to avoid getting stuck on corners. If this is a convex corner and the average normal is not close enough to either sides, abort.
	return GetWorld()->SweepSingleByChannel(OutWallHit, TraceStart, TraceEnd, FQuat::Identity, CollisionChannel, CollisionShape, QueryParams, ResponseParam);
}

void UXMoveU_WallRunMoveMode::MaintainWallPlaneVelocity()
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	MoveComp->Velocity = FVector::VectorPlaneProject(MoveComp->Velocity, CurrentWall.Normal);
}
