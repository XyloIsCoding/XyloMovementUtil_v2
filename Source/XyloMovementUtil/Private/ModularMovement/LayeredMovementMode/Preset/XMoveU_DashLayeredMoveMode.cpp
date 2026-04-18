// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Preset/XMoveU_DashLayeredMoveMode.h"

#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_DashLayeredMoveMode::UXMoveU_DashLayeredMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCancelRequestAfterTransitionCheck = true;
	
	DashVerticalImpulseSpeed = 100.f;
	DashHorizontalImpulseSpeed = 1300.f;
	bUseDeadZoneAngle = true;
	DashAngleCosineDeadZone = 0.5f;
	DashDuration = 0.5f;
	DashMaxCharges = 2;
	DashRechargeTime = 4.f;
}

void UXMoveU_DashLayeredMoveMode::OnRegistered(uint32 InModeIndex)
{
	Super::OnRegistered(InModeIndex);
	DashCharge = DashMaxCharges;
}

bool UXMoveU_DashLayeredMoveMode::ShouldEnterMode(float DeltaSeconds) const
{
	if (FMath::Floor<int32>(DashCharge) < 1)
	{
		return false;
	}
	return !IsInMode() && WantsToBeInMode() && CanDashInCurrentState();
}

bool UXMoveU_DashLayeredMoveMode::ShouldForceLeaveMode(float DeltaSeconds) const
{
	return IsInMode() && (TimeSinceDash > DashDuration || !CanDashInCurrentState(true));
}

bool UXMoveU_DashLayeredMoveMode::CanDashInCurrentState(bool bIgnoreDeadZone) const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	if (!MoveComp || !MoveComp->UpdatedComponent || MoveComp->UpdatedComponent->IsSimulatingPhysics())
	{
		return false;
	}
	
	return MoveComp->IsFalling() || (MoveComp->IsMovingOnGround() && (bIgnoreDeadZone || !IsInputInDeadZone()));
}

bool UXMoveU_DashLayeredMoveMode::IsInputInDeadZone() const
{
	// If not using DeadZone angle we can never be in DeadZone.
	if (!bUseDeadZoneAngle)
	{
		return false;
	}
	
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	FVector InputDirection = MoveComp->GetCurrentAcceleration().GetSafeNormal();

	// No acceleration means forward. So we are in DeadZone for sure.
	if (InputDirection.IsNearlyZero())
	{
		return true;
	}

	// We are in DeadZone if the cosine between input and capsule direction is greater than threshold.
	return (InputDirection | MoveComp->UpdatedComponent->GetForwardVector()) > DashAngleCosineDeadZone;
}

void UXMoveU_DashLayeredMoveMode::OnEnteredMode()
{
	TimeSinceDash = 0.f;
	DashCharge -= 1.f;
	
	if (GetOwningCharacter()->GetLocalRole() != ROLE_SimulatedProxy)
	{
		UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
		FVector InputDirection = MoveComp->GetCurrentAcceleration().GetSafeNormal();
		if (InputDirection.IsNearlyZero())
		{
			InputDirection = MoveComp->GetForwardVector();
		}
		
		float VerticalVelocity = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(MoveComp->Velocity) : MoveComp->Velocity.Z;
		VerticalVelocity = FMath::Max(VerticalVelocity, 0.f);
		VerticalVelocity += DashVerticalImpulseSpeed;

		float HorizontalVelocity = DashHorizontalImpulseSpeed + (MoveComp->Velocity | InputDirection);
		
		MoveComp->Velocity = HorizontalVelocity * InputDirection + (VerticalVelocity * -MoveComp->GetGravityDirection());

		CachedDashDirection = InputDirection;
		
		if (DashVerticalImpulseSpeed > 0.f)
		{
			MoveComp->SetMovementMode(MOVE_Falling);
		}
	}
}

void UXMoveU_DashLayeredMoveMode::UpdateMode(float DeltaSeconds)
{
	if (TimeSinceDash < DashDuration)
	{
		TimeSinceDash += DeltaSeconds;
	}
	
	if (DashCharge < DashMaxCharges)
	{
		DashCharge += DeltaSeconds / DashRechargeTime;
		DashCharge = FMath::Min<float>(DashCharge, DashMaxCharges);
	}
}
