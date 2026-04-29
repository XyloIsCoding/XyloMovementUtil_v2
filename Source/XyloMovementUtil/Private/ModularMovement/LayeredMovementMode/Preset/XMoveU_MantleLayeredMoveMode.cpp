// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Preset/XMoveU_MantleLayeredMoveMode.h"

#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

#if ROOT_MOTION_DEBUG
static IConsoleVariable* Get_CVarDebugRootMotionSourcesLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.RootMotion.DebugSourceLifeTime"));
#endif


FXMoveU_RootMotionSource_Mantle::FXMoveU_RootMotionSource_Mantle()
	: bRestrictSpeedToExpected(false)
{
}

void FXMoveU_RootMotionSource_Mantle::CalculateMantleTrajectory(const FVector& Start, const FVector& Mid, const FVector& End)
{
	MantlePathCurve.Reset();
	// Time 0.0: Start
	MantlePathCurve.AddPoint(0.0f, Start);
	// Time 0.5: Mid (The "Buffer" point)
	MantlePathCurve.AddPoint(0.7f, Mid);
	// Time 1.0: End
	MantlePathCurve.AddPoint(1.0f, End);

	// This turns the 3 points into a smooth cubic spline automatically
	MantlePathCurve.AutoSetTangents(0.0f, true);
}

FRootMotionSource* FXMoveU_RootMotionSource_Mantle::Clone() const
{
	FXMoveU_RootMotionSource_Mantle* CopyPtr = new FXMoveU_RootMotionSource_Mantle(*this);
	return CopyPtr;
}

bool FXMoveU_RootMotionSource_Mantle::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}

	// We can cast safely here since in FRootMotionSource::Matches() we ensured ScriptStruct equality
	const FXMoveU_RootMotionSource_Mantle* OtherCast = static_cast<const FXMoveU_RootMotionSource_Mantle*>(Other);

	return bRestrictSpeedToExpected == OtherCast->bRestrictSpeedToExpected &&
		MantlePathCurve == OtherCast->MantlePathCurve;
}

bool FXMoveU_RootMotionSource_Mantle::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	// Check that it matches
	if (!FRootMotionSource::MatchesAndHasSameState(Other))
	{
		return false;
	}

	return true; // MoveToForce has no unique state
}

bool FXMoveU_RootMotionSource_Mantle::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup)
{
	if (!FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup))
	{
		return false;
	}

	return true; // MoveToForce has no unique state other than Time which is handled by FRootMotionSource
}

void FXMoveU_RootMotionSource_Mantle::SetTime(float NewTime)
{
	FRootMotionSource::SetTime(NewTime);

	// TODO-RootMotionSource: Check if reached destination?
}

void FXMoveU_RootMotionSource_Mantle::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();

	if (Duration > UE_SMALL_NUMBER && MovementTickTime > UE_SMALL_NUMBER)
	{
		const float MoveFraction = (GetTime() + SimulationTime) / Duration;

		FVector CurrentTargetLocation = MantlePathCurve.Eval(MoveFraction);

		const FVector CurrentLocation = Character.GetActorLocation();

		FVector Force = (CurrentTargetLocation - CurrentLocation) / MovementTickTime;

		if (bRestrictSpeedToExpected && !Force.IsNearlyZero(UE_KINDA_SMALL_NUMBER))
		{
			// Calculate expected current location (if we didn't have collision and moved exactly where our velocity should have taken us)
			const float PreviousMoveFraction = GetTime() / Duration;
			FVector CurrentExpectedLocation = MantlePathCurve.Eval(PreviousMoveFraction);

			// Restrict speed to the expected speed, allowing some small amount of error
			const FVector ExpectedForce = (CurrentTargetLocation - CurrentExpectedLocation) / MovementTickTime;
			const float ExpectedSpeed = ExpectedForce.Size();
			const float CurrentSpeedSqr = Force.SizeSquared();

			const float ErrorAllowance = 0.5f; // in cm/s
			if (CurrentSpeedSqr > FMath::Square(ExpectedSpeed + ErrorAllowance))
			{
				Force.Normalize();
				Force *= ExpectedSpeed;
			}
		}

		// Debug
#if ROOT_MOTION_DEBUG
		if (RootMotionSourceDebug::CVarDebugRootMotionSources.GetValueOnGameThread() != 0)
		{
			const FVector LocDiff = MoveComponent.UpdatedComponent->GetComponentLocation() - CurrentLocation;
			const float DebugLifetime = Get_CVarDebugRootMotionSourcesLifetime->AsVariableFloat()->GetValueOnGameThread();

			// Current
			DrawDebugCapsule(Character.GetWorld(), MoveComponent.UpdatedComponent->GetComponentLocation(), Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Red, true, DebugLifetime);

			// Current Target
			DrawDebugCapsule(Character.GetWorld(), CurrentTargetLocation + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Green, true, DebugLifetime);

			// Target
			DrawDebugCapsule(Character.GetWorld(), MantlePathCurve.Eval(1.f) + LocDiff, Character.GetSimpleCollisionHalfHeight(), Character.GetSimpleCollisionRadius(), FQuat::Identity, FColor::Blue, true, DebugLifetime);

			// Force
			DrawDebugLine(Character.GetWorld(), CurrentLocation, CurrentLocation+Force, FColor::Blue, true, DebugLifetime);
		}
#endif

		FTransform NewTransform(Force);
		RootMotionParams.Set(NewTransform);
	}
	else
	{
		checkf(Duration > UE_SMALL_NUMBER, TEXT("FXMoveU_RootMotionSource_Mantle prepared with invalid duration."));
	}

	SetTime(GetTime() + SimulationTime);
}

bool FXMoveU_RootMotionSource_Mantle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << MantlePathCurve;
	Ar << bRestrictSpeedToExpected;

	bOutSuccess = true;
	return true;
}

UScriptStruct* FXMoveU_RootMotionSource_Mantle::GetScriptStruct() const
{
	return FXMoveU_RootMotionSource_Mantle::StaticStruct();
}

FString FXMoveU_RootMotionSource_Mantle::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FXMoveU_RootMotionSource_Mantle %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FXMoveU_RootMotionSource_Mantle::AddReferencedObjects(class FReferenceCollector& Collector)
{
	FRootMotionSource::AddReferencedObjects(Collector);
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*====================================================================================================================*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





UXMoveU_MantleLayeredMoveMode::UXMoveU_MantleLayeredMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxVerticalWallReach = 180.f;
	MaxHorizontalWallReach = 60.f;
	MinMantleAngleCosine = FMath::Cos(FMath::DegreesToRadians(60));
	MantleSpeed = 550.f;
	PostMantleHorizontalVelocity = 450.f;
	PostMantleVerticalVelocity = 300.f;
}

bool UXMoveU_MantleLayeredMoveMode::ShouldReplaceJump(float DeltaSeconds)
{
	if (IsInMode())
	{
		return false;
	}
	
	// UKismetSystemLibrary::PrintString(CharacterOwner, FString::Printf(TEXT("Mantle >> TRYING TO PERFORM MANTLE ----------------")));
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	FVector Direction = MoveComp->GetCurrentAcceleration().GetSafeNormal2D();
	
	FHitResult WallHit;
	if (!FindWall(Direction, WallHit))
	{
		return false;
	}

	FVector WallNormal2D = WallHit.Normal.GetSafeNormal2D();
	if (((Direction | -WallNormal2D) < MinMantleAngleCosine) || ((MoveComp->GetForwardVector().GetSafeNormal2D() | -WallNormal2D) < MinMantleAngleCosine))
	{
		return false;
	}

	FHitResult LedgeHit;
	if (!FindLedge(Direction, WallHit, LedgeHit))
	{
		return false;
	}

	if (!IsLedgeValid(WallHit, LedgeHit))
	{
		return false;
	}

	FVector OnLedge = LedgeHit.ImpactPoint + MoveComp->GetScaledCapsuleHalfHeight() * MoveComp->UpdatedComponent->GetUpVector();

	FVector LedgeToWallHorizontal = FVector::VectorPlaneProject(WallHit.ImpactPoint - LedgeHit.ImpactPoint, FVector::DownVector);
	FVector PreLedge = OnLedge + LedgeToWallHorizontal - (Direction * MoveComp->GetScaledCapsuleRadius());
	
	FVector FeetLocation = MoveComp->GetActorLocation() - MoveComp->GetScaledCapsuleHalfHeight() * MoveComp->UpdatedComponent->GetUpVector();
	FVector ToLedgeVector = LedgeHit.ImpactPoint - FeetLocation;
	float Height = ToLedgeVector.Z;
	float Distance = ToLedgeVector.Size2D();
	
	MantleRMS.Reset();
	MantleRMS = MakeShared<FXMoveU_RootMotionSource_Mantle>();
	MantleRMS->AccumulateMode = ERootMotionAccumulateMode::Override;
	MantleRMS->Duration = (Distance + Height) / MantleSpeed;
	MantleRMS->CalculateMantleTrajectory(MoveComp->GetActorLocation(), PreLedge, OnLedge);
	
	return true;
}

bool UXMoveU_MantleLayeredMoveMode::ShouldForceLeaveMode(float DeltaSeconds)
{
	if (!IsInMode())
	{
		return false;
	}
	
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	TSharedPtr<FRootMotionSource> MantleRootMotion = MoveComp->GetRootMotionSourceByID(MantleRMS_ID);
	return MantleRootMotion && MantleRootMotion->Status.HasFlag(ERootMotionSourceStatusFlags::Finished);
}

void UXMoveU_MantleLayeredMoveMode::OnEnteredMode()
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	MoveComp->SetMovementMode(MOVE_Flying);
	MantleRMS_ID = MoveComp->ApplyRootMotionSource(MantleRMS);
}

void UXMoveU_MantleLayeredMoveMode::OnLeftMode()
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	MoveComp->SetMovementMode(MOVE_Falling);
	
	MoveComp->Velocity = PostMantleHorizontalVelocity * MoveComp->GetCurrentAcceleration().GetSafeNormal2D() + PostMantleVerticalVelocity * MoveComp->GetGravityDirection();
}

bool UXMoveU_MantleLayeredMoveMode::FindWall(const FVector& Direction, FHitResult& OutWallHit) const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ComputeWallEdge), false, GetOwningCharacter());
	FCollisionResponseParams ResponseParam;
	MoveComp->InitCollisionParams(QueryParams, ResponseParam);
	const ECollisionChannel CollisionChannel = MoveComp->UpdatedComponent->GetCollisionObjectType();
	
	FCollisionShape WallTestCapsule;
	WallTestCapsule.SetCapsule(10.f, MaxVerticalWallReach / 2.f);
	
	FVector FeetLocation = MoveComp->GetActorLocation() - MoveComp->GetScaledCapsuleHalfHeight() * MoveComp->UpdatedComponent->GetUpVector() + 3.f * MoveComp->UpdatedComponent->GetUpVector();
	FVector SweepStartLocation = FeetLocation + WallTestCapsule.GetCapsuleHalfHeight() * MoveComp->UpdatedComponent->GetUpVector();
	FVector SweepEndLocation = SweepStartLocation + (MaxHorizontalWallReach + MoveComp->GetScaledCapsuleRadius()) * Direction;
	
	GetWorld()->SweepSingleByChannel(OutWallHit, SweepStartLocation, SweepEndLocation, FQuat::Identity, CollisionChannel, WallTestCapsule, QueryParams, ResponseParam);

	// DrawDebugCapsule(GetWorld(), SweepStartLocation, WallTestCapsule.GetCapsuleHalfHeight(), 10.f, FQuat::Identity, FColor::Green, false, 2.f, 0, 1.f);
	// DrawDebugCapsule(GetWorld(), SweepEndLocation, WallTestCapsule.GetCapsuleHalfHeight(), 10.f, FQuat::Identity, !OutWallHit.bBlockingHit ? FColor::Orange : FColor::Green, false, 2.f, 0, 1.f);
	// if (!OutWallHit.bBlockingHit) UKismetSystemLibrary::PrintString(CharacterOwner, FString::Printf(TEXT("Mantle >> Did not find wall")));
	return OutWallHit.bBlockingHit && !MoveComp->IsWalkable(OutWallHit);
}

bool UXMoveU_MantleLayeredMoveMode::FindLedge(const FVector& Direction, const FHitResult& WallHit, FHitResult& OutLedgeHit) const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ComputeWallEdge), false, GetOwningCharacter());
	FCollisionResponseParams ResponseParam;
	MoveComp->InitCollisionParams(QueryParams, ResponseParam);
	const ECollisionChannel CollisionChannel = MoveComp->UpdatedComponent->GetCollisionObjectType();
	
	//FVector WallUpVector = FVector::VectorPlaneProject(UpdatedComponent->GetUpVector(), WallHit.ImpactNormal).GetSafeNormal();

	FVector FeetLocation = MoveComp->GetActorLocation() - MoveComp->GetScaledCapsuleHalfHeight() * MoveComp->UpdatedComponent->GetUpVector() + 3.f * MoveComp->UpdatedComponent->GetUpVector();
	float FeetToWallHitHeight = WallHit.ImpactPoint.Z - FeetLocation.Z;
	float MaxVerticalReachFromWallHit = MaxVerticalWallReach - FeetToWallHitHeight;
	FVector MaxReachOnWall = WallHit.ImpactPoint + MaxVerticalReachFromWallHit * MoveComp->UpdatedComponent->GetUpVector();
	FVector LoweredWallImpactPoint = WallHit.ImpactPoint - 1.f * MoveComp->UpdatedComponent->GetUpVector();
	
	FVector LedgeForwardOffset = MoveComp->GetScaledCapsuleRadius() * Direction;
	GetWorld()->LineTraceSingleByChannel(OutLedgeHit, MaxReachOnWall + LedgeForwardOffset, LoweredWallImpactPoint + LedgeForwardOffset, CollisionChannel, QueryParams, ResponseParam);

	if (!MoveComp->IsWalkable(OutLedgeHit))
	{
		// DrawDebugDirectionalArrow(GetWorld(), MaxReachOnWall + LedgeForwardOffset, LoweredWallImpactPoint + LedgeForwardOffset, 10.f, FColor::Orange, false, 2.f, 0, 1.f);
		// UKismetSystemLibrary::PrintString(CharacterOwner, FString::Printf(TEXT("Mantle >> Cannot find ledge")));
		return false;
	}

	// DrawDebugDirectionalArrow(GetWorld(), MaxReachOnWall + LedgeForwardOffset, OutLedgeHit.ImpactPoint, 10.f, FColor::Green, false, 2.f, 0, 1.f);
	return true;
}

bool UXMoveU_MantleLayeredMoveMode::IsLedgeValid(const FHitResult& WallHit, const FHitResult& LedgeHit) const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ComputeWallEdge), false, GetOwningCharacter());
	FCollisionResponseParams ResponseParam;
	MoveComp->InitCollisionParams(QueryParams, ResponseParam);
	const ECollisionChannel CollisionChannel = MoveComp->UpdatedComponent->GetCollisionObjectType();

	FCollisionShape CapsuleShape = MoveComp->GetPawnCapsuleCollisionShape(SHRINK_None);
	
	// Check if top ledge is available
	FVector SmallUpVector = 60.f * MoveComp->UpdatedComponent->GetUpVector();
	FVector CapsuleAdjustment = CapsuleShape.GetCapsuleHalfHeight() * MoveComp->UpdatedComponent->GetUpVector();
	FVector CapsuleFinalPosition = LedgeHit.ImpactPoint + CapsuleAdjustment + SmallUpVector;
	bool bLedgeBlocked = GetWorld()->OverlapAnyTestByChannel(CapsuleFinalPosition, FQuat::Identity, CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

	// DrawDebugCapsule(GetWorld(), CapsuleFinalPosition, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), FQuat::Identity, bLedgeBlocked ? FColor::Orange : FColor::Blue, false, 2.f, 0, 1.f);
	if (bLedgeBlocked)
	{
		// UKismetSystemLibrary::PrintString(CharacterOwner, FString::Printf(TEXT("Mantle >> Ledge Is Blocked")));
		return false;
	}

	// Check if we can reach mantle apex
	FVector FeetLocation = MoveComp->GetActorLocation() - MoveComp->GetScaledCapsuleHalfHeight() * MoveComp->UpdatedComponent->GetUpVector();
	FVector ToLedgeVector = LedgeHit.ImpactPoint - FeetLocation;
	float Height = ToLedgeVector.Z;

	FVector CapsuleInitialPosition = FeetLocation + CapsuleAdjustment + SmallUpVector;
	FVector CapsuleMantleApexPosition = CapsuleInitialPosition + (Height * MoveComp->UpdatedComponent->GetUpVector());
	bool bCannotMoveUp = GetWorld()->SweepTestByChannel(CapsuleInitialPosition, CapsuleMantleApexPosition, FQuat::Identity, CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

	// DrawDebugCapsule(GetWorld(), CapsuleInitialPosition, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), FQuat::Identity, bCannotMoveUp ? FColor::Orange : FColor::Blue, false, 2.f, 0, 1.f);
	// DrawDebugCapsule(GetWorld(), CapsuleMantleApexPosition, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), FQuat::Identity, bCannotMoveUp ? FColor::Orange : FColor::Blue, false, 2.f, 0, 1.f);
	if (bCannotMoveUp)
	{
		// UKismetSystemLibrary::PrintString(CharacterOwner, FString::Printf(TEXT("Mantle >> Cannot reach apex")));
		return false;
	}

	// Check if we can reach mantle final position
	bool bCannotMoveForward = GetWorld()->SweepTestByChannel(CapsuleMantleApexPosition, CapsuleFinalPosition, FQuat::Identity, CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

	// DrawDebugCapsule(GetWorld(), CapsuleFinalPosition, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), FQuat::Identity, bCannotMoveForward ? FColor::Orange : FColor::Blue, false, 2.f, 0, 1.f);
	if (bCannotMoveForward)
	{
		// UKismetSystemLibrary::PrintString(CharacterOwner, FString::Printf(TEXT("Mantle >> Cannot reach final position")));
		return false;
	}
	
	return true;
}
