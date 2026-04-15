// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Preset/XMoveU_DashLayeredMoveMode.h"

#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_DashLayeredMoveMode::UXMoveU_DashLayeredMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCancelRequestAfterTransitionCheck = true;
	
	DashVerticalImpulseSpeed = 300.f;
	DashHorizontalImpulseSpeed = 600.f;
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

	const bool bOutOfDeadZone = bIgnoreDeadZone || (MoveComp->GetCurrentAcceleration().GetSafeNormal() | MoveComp->UpdatedComponent->GetForwardVector()) < DashAngleCosineDeadZone;
	return MoveComp->IsFalling() || (MoveComp->IsMovingOnGround() && !MoveComp->IsCrouching() && bOutOfDeadZone);
}

void UXMoveU_DashLayeredMoveMode::OnEnteredMode()
{
	TimeSinceDash = 0.f;
	DashCharge -= 1.f;
	
	if (GetOwningCharacter()->GetLocalRole() != ROLE_SimulatedProxy)
	{
		UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
		UXMoveU_JumpStaticLibrary::LimitMinVerticalVelocity(MoveComp, 0.f);
		UXMoveU_JumpStaticLibrary::ApplyJumpImpulse(MoveComp, MoveComp->GetCurrentAcceleration().GetSafeNormal(), DashVerticalImpulseSpeed, DashHorizontalImpulseSpeed, false);
		MoveComp->SetMovementMode(MOVE_Falling);
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
