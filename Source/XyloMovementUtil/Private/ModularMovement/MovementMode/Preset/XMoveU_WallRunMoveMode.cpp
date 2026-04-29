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
	MovementModeType = EXMoveU_MovementModeType::Custom;
}

bool UXMoveU_WallRunMoveMode::ShouldEnterMode()
{
	if (!CanWallRunInCurrentState())
	{
		return false;
	}

	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();

	// Do not start wall run if we are below the minimum allowed value
	float VelocityZ = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(MoveComp->Velocity) : MoveComp->Velocity.Z;
	if (VelocityZ < WallRunMinEnterVerticalSpeed)
	{
		return false;
	}
	
	FindWall(CurrentWall, (MoveComp->GetCurrentAcceleration().GetSafeNormal2D() + MoveComp->Velocity.GetSafeNormal2D() * 0.8), MaxWallDistance * 0.6);
	DrawDebugDirectionalArrow(GetWorld(), CurrentWall.WallHit.ImpactPoint, CurrentWall.WallHit.ImpactPoint + CurrentWall.WallHit.Normal * 10.f, 3.f, FColor::Green, false, 2.f, 0, 2.f);

	FHitResult WallHit;
	if (!FindWallAtHandsHeight(WallHit))
	{
		return false;
	}
	
	return CurrentWall.WallHit.IsValidBlockingHit();
}

void UXMoveU_WallRunMoveMode::OnEnteredMovementMode(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	float MinZVelocity = IsClimbing() ? ClimbMinVelocityZ : 0.f;
	if (MoveComp->HasCustomGravity())
	{
		MoveComp->SetGravitySpaceZ(MoveComp->Velocity, FMath::Max(MinZVelocity, MoveComp->GetGravitySpaceZ(MoveComp->Velocity)));
	}
	else
	{
		MoveComp->Velocity.Z = FMath::Max(MinZVelocity, MoveComp->Velocity.Z);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Entering Wallrun"))
}

void UXMoveU_WallRunMoveMode::OnLeftMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	WallRunReentryLockTimeRemaining = WallRunReentryTime;
	
	UE_LOG(LogTemp, Warning, TEXT("Leaving Wallrun"))
}

void UXMoveU_WallRunMoveMode::UpdateMode(float DeltaTime)
{
	// Decrease Slide cooldown timer
	if (WallRunReentryLockTimeRemaining > 0.f)
	{
		WallRunReentryLockTimeRemaining -= DeltaTime;
	}
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
		FindWall(CurrentWall, -CurrentWall.WallHit.Normal, MaxWallDistance);
		DrawDebugDirectionalArrow(GetWorld(), CurrentWall.WallHit.ImpactPoint, CurrentWall.WallHit.ImpactPoint + CurrentWall.WallHit.Normal * 10.f, 3.f, FColor::Orange, false, 0.1f, 0, 2.f);

		
		if (!CurrentWall.WallHit.IsValidBlockingHit())
		{
			UE_LOG(LogTemp, Warning, TEXT("Leaving Wallrun cause invalid wall (begin tick)"))
			OnWallEnded(remainingTime, Iterations);
			return;
		}

		// If acceleration points away from the wall, quit wall run
		if (!MoveComp->GetCurrentAcceleration().IsNearlyZero() && (CurrentWall.WallHit.Normal | MoveComp->GetCurrentAcceleration().GetSafeNormal2D()) > WallRunLeaveAngleCosine)
		{
			UE_LOG(LogTemp, Warning, TEXT("Leaving Wallrun cause acceleration pointing away"))
			MoveComp->SetMovementMode(MOVE_Falling);
			MoveComp->StartNewPhysics(remainingTime, Iterations);
			return;
		}
		
		// Ensure velocity is aligned with wall.
		MaintainWallPlaneVelocity();
		const FVector OldVelocity = MoveComp->Velocity;
		FVector WallProjectedAcceleration = FVector::VectorPlaneProject(MoveComp->Acceleration, CurrentWall.WallHit.Normal);
		if (FMath::Abs(MoveComp->Acceleration.GetSafeNormal() | CurrentWall.WallHit.Normal) > WallAccelerationDeadZoneAngleCosine)
		{
			// Delete wall projected acceleration if we are in dead-zone
			WallProjectedAcceleration = FVector::ZeroVector;
		}

		float WallDirectionAlpha = FMath::Clamp(-CurrentWall.WallHit.Normal | MoveComp->UpdatedComponent->GetForwardVector(), 0.f, 1.f);

		DrawDebugDirectionalArrow(GetWorld(), MoveComp->GetActorLocation(), MoveComp->GetActorLocation() + MoveComp->Acceleration.GetSafeNormal() * 50.f, 2.f, FColor::Yellow, false, 0.1f, 0, 1.f);

		
		// Apply acceleration
		if( !MoveComp->HasAnimRootMotion() && !MoveComp->CurrentRootMotion.HasOverrideVelocity() )
		{
			TGuardValue<FVector> RestoreAcceleration(MoveComp->Acceleration, WallProjectedAcceleration);
			if (MoveComp->HasCustomGravity())
			{
				MoveComp->Velocity = MoveComp->ProjectToGravityFloor(MoveComp->Velocity);
				const FVector GravityRelativeOffset = OldVelocity - MoveComp->Velocity;
				MoveComp->CalcVelocity(timeTick, MoveComp->GetBrakingFriction(), false, MoveComp->GetMaxBrakingDeceleration());
				MoveComp->Velocity += GravityRelativeOffset;
			}
			else
			{
				MoveComp->Velocity.Z = 0.f;
				MoveComp->CalcVelocity(timeTick, MoveComp->GetBrakingFriction(), false, MoveComp->GetMaxBrakingDeceleration());
				MoveComp->Velocity.Z = OldVelocity.Z;
			}
			
			devCode(ensureMsgf(!MoveComp->Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *MoveComp->Velocity.ToString()));
		}
		
		
		// Check if we want to climb
		float WallUpAcceleration = MoveComp->GetCurrentAcceleration() | -CurrentWall.WallHit.Normal;
		const bool bIsActivelyClimbing = IsClimbing() && WallUpAcceleration > 0.f;
		if (bIsActivelyClimbing)
		{
			FHitResult WallHit;
			FindWallAtHandsHeight(WallHit);
			if (WallHit.IsValidBlockingHit())
			{
				// Compute climb velocity
				const float VerticalVelocity = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(MoveComp->Velocity) : MoveComp->Velocity.Z;
				if (VerticalVelocity < WallClimbVerticalVelocity)
				{
					const float AddVelocity = FMath::Clamp(WallUpAcceleration / MoveComp->GetMaxAcceleration(), 0.f, 1.f) * WallClimbVerticalVelocity;
					MoveComp->SetGravitySpaceZ(MoveComp->Velocity, FMath::Min(VerticalVelocity + AddVelocity, WallClimbVerticalVelocity));
				}
			}
		}

		
		// Compute current gravity
		float GravityScale = 1.f;
		const float VerticalVelocity = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(MoveComp->Velocity) : MoveComp->Velocity.Z;
		if (VerticalVelocity > 0.f)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Wall Run Ascending %f"), WallDirectionAlpha)
			GravityScale = FMath::Lerp(WallRunMaxAscendingGravityScale, WallRunMinAscendingGravityScale, WallDirectionAlpha);
		}
		else
		{
			if (!bIsActivelyClimbing)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Wall Run Descending"))
				GravityScale = WallRunDescendingGravityScale;
			}
		}
		const FVector Gravity = -MoveComp->GetGravityDirection() * MoveComp->GetGravityZ() * GravityScale;
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

		// Calculate velocity to wall (not adding it directly to cmc velocity, since it is not real physical state but
		// just a pulling force we add in the movement Delta. Also adding it to velocity makes it so when jumping
		// away from the wall we get pulled back.)
		const FVector ToWallVelocity = -CurrentWall.WallHit.Normal * WallAttractionForce;

		// Compute move parameters
		const FVector MoveVelocity = MoveComp->Velocity + ToWallVelocity;
		FVector Delta = timeTick * MoveVelocity;

		FHitResult PredictedWallHit;
		float GravityUpDelta = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(Delta) : Delta.Z;
		FindWallAtHandsHeight(PredictedWallHit, GravityUpDelta * (-MoveComp->GetGravityDirection()));
		if (!PredictedWallHit.IsValidBlockingHit())
		{
			// Cannot move upward if no wall.
			MoveComp->SetGravitySpaceZ(Delta, FMath::Min(GravityUpDelta, 0.f));
		}
		
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

			MoveComp->HandleImpact(Hit, timeTick, Delta);
			MoveComp->SlideAlongSurface(Delta, (1.f - Hit.Time), CurrentWall.WallHit.Normal, Hit, true);
			
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
		FindWall(CurrentWall, -CurrentWall.WallHit.Normal, MaxWallDistance);
		if (!CurrentWall.WallHit.IsValidBlockingHit())
		{
			UE_LOG(LogTemp, Warning, TEXT("Leaving Wallrun cause invalid wall (end tick)"))
			OnWallEnded(remainingTime, Iterations);
			return;
		}
		
		
		// Detach from wall if Z velocity is lower than the detach threshold
		float VelocityZ = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(MoveComp->Velocity) : MoveComp->Velocity.Z;
		if (VelocityZ < WallRunVerticalSpeedDetachThreshold)
		{
			MoveComp->SetMovementMode(MOVE_Falling);
			MoveComp->StartNewPhysics(remainingTime, Iterations);
			return;
		}

		// TODO: check if we need to land

		// Allow overlap events and such to change physics state and velocity
		if (IsInMode())
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

	if (IsInMode())
	{
		MaintainWallPlaneVelocity();
	}
}

float UXMoveU_WallRunMoveMode::GetModeMaxSpeed() const
{
	return IsClimbing() ? WallClimbLateralVelocity : MaxWallRunningSpeed;
}

bool UXMoveU_WallRunMoveMode::CanWallRunInCurrentState() const
{
	if (WallRunReentryLockTimeRemaining > 0.f)
	{
		return false;
	}
	
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return !IsInMode() && MoveComp->IsFalling() && !MoveComp->IsCrouching(); 
}

bool UXMoveU_WallRunMoveMode::FindWall(FXMoveU_WallData& OutWallData, const FVector& Direction, float Distance)
{
	OutWallData.WallHit.Reset();
	
	if (Direction.IsNearlyZero())
	{
		return false;
	}
	const FVector NormalizedDirection = Direction.GetSafeNormal2D();
	
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();

	const UCapsuleComponent* CapsuleComp = MoveComp->CharacterOwner->GetCapsuleComponent();
	const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	
	const FVector TraceStart = MoveComp->GetActorLocation() + ((-CapsuleHalfHeight + WallRunFeetHeight) * CapsuleComp->GetUpVector());
	const FVector TraceEnd = TraceStart + NormalizedDirection * (MoveComp->GetScaledCapsuleRadius() + Distance);

	const ECollisionChannel CollisionChannel = (MoveComp->UpdatedComponent ? MoveComp->UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(XMoveU_CharacterMovementComponent_GetWallInfo), false, MoveComp->CharacterOwner);
	FCollisionResponseParams ResponseParam;
	MoveComp->InitCollisionParams(QueryParams, ResponseParam);

	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(5.f);

	int32 NumberOfTraces = 7;
	float TraceSeparationDistance = 15.f;

	int32 SuccessfulHits = 0;
	FVector AverageWallPosition = FVector::ZeroVector;
	FVector AverageWallNormal = FVector::ZeroVector;
	const float TraceRange = (NumberOfTraces - 1) / 2.f;
	const FVector RightVector = FVector::CrossProduct(MoveComp->GetGravityDirection(), MoveComp->ProjectToGravityFloor(NormalizedDirection));
	for (int32 i = 0; i < NumberOfTraces; ++i)
	{
		const FVector TraceOffset = RightVector * (TraceSeparationDistance * (i - TraceRange));
		
		FHitResult Hit;
		// GetWorld()->SweepSingleByChannel(Hit, TraceStart + TraceOffset, TraceEnd + TraceOffset, FQuat::Identity, CollisionChannel, CollisionShape, QueryParams, ResponseParam);
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart + TraceOffset, TraceEnd + TraceOffset, CollisionChannel, QueryParams, ResponseParam);
		
		DrawDebugDirectionalArrow(GetWorld(), Hit.TraceStart, Hit.bBlockingHit ? Hit.ImpactPoint : Hit.TraceEnd, 1.f, Hit.bBlockingHit ? FColor::Cyan : FColor::Red, false, 0.1f, 0, 0.5f);
		
		if (Hit.IsValidBlockingHit())
		{
			SuccessfulHits += 1;
			AverageWallPosition += Hit.ImpactPoint;
			AverageWallNormal += Hit.ImpactNormal;
		}
	}

	if (SuccessfulHits > 0)
	{
		AverageWallPosition /= SuccessfulHits;
		AverageWallNormal = (AverageWallNormal / SuccessfulHits).GetSafeNormal();

		DrawDebugDirectionalArrow(GetWorld(), AverageWallPosition, AverageWallPosition + AverageWallNormal * 30.f, 1.f, FColor::Magenta, false, 0.1f, 0, 0.5f);

		const FVector ToWallAverage = -AverageWallNormal * Distance;
		GetWorld()->SweepSingleByChannel(OutWallData.WallHit, TraceStart, TraceStart + ToWallAverage, FQuat::Identity, CollisionChannel, CollisionShape, QueryParams, ResponseParam);

		OutWallData.LastWallNormal = AverageWallNormal;
		OutWallData.WallHit.Normal = AverageWallNormal;
		OutWallData.WallHit.ImpactNormal = AverageWallNormal;
		return OutWallData.WallHit.bBlockingHit;
	}

	return false;
}

bool UXMoveU_WallRunMoveMode::IsClimbing() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	const float WallDirectionAlpha = -CurrentWall.WallHit.Normal | MoveComp->UpdatedComponent->GetForwardVector();
	return  WallDirectionAlpha > MinClimbWallAngleCosine;
}

float UXMoveU_WallRunMoveMode::GetHandsHeight() const
{
	return IsClimbing() ? ClimbHandsHeight : WallRunHandsHeight;
}

bool UXMoveU_WallRunMoveMode::FindWallAtHandsHeight(FHitResult& OutWallHit, const FVector& PositionOffset)
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	
	const UCapsuleComponent* CapsuleComp = MoveComp->CharacterOwner->GetCapsuleComponent();
	
	const FVector TraceStart = MoveComp->GetActorLocation() + (GetHandsHeight() * CapsuleComp->GetUpVector()) + PositionOffset;
	const FVector TraceEnd = TraceStart + (-CurrentWall.WallHit.Normal) * (MoveComp->GetScaledCapsuleRadius() + MaxWallDistance);

	const ECollisionChannel CollisionChannel = (MoveComp->UpdatedComponent ? MoveComp->UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(XMoveU_CharacterMovementComponent_GetWallInfo), false, MoveComp->CharacterOwner);
	FCollisionResponseParams ResponseParam;
	MoveComp->InitCollisionParams(QueryParams, ResponseParam);

	return GetWorld()->LineTraceSingleByChannel(OutWallHit, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);
}

void UXMoveU_WallRunMoveMode::MaintainWallPlaneVelocity()
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	MoveComp->Velocity = FVector::VectorPlaneProject(MoveComp->Velocity, CurrentWall.WallHit.Normal);
}

void UXMoveU_WallRunMoveMode::OnWallEnded(float remainingTime, int32 Iterations)
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	
	MoveComp->DoJump(Character->bClientUpdating, remainingTime);
	MoveComp->SetMovementMode(MOVE_Falling);
	MoveComp->StartNewPhysics(remainingTime, Iterations);
}
