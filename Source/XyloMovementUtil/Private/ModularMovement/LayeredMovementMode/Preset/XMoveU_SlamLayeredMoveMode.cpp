// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/LayeredMovementMode/Preset/XMoveU_SlamLayeredMoveMode.h"

#include "ModularMovement/XMoveU_JumpStaticLibrary.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_SlamLayeredMoveMode::UXMoveU_SlamLayeredMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCancelRequestAfterTransitionCheck = true;
	
	SlamGravityMultiplier = 3.f;
	SlamHorizontalVelocity = 700.f;
	SlamMinHeight = 200.f;
}

void UXMoveU_SlamLayeredMoveMode::OnRegistered(uint32 InModeIndex)
{
	Super::OnRegistered(InModeIndex);

	AXMoveU_ModularCharacter* Character = GetOwningCharacter();
	Character->MoveBlockedByDelegate.AddUObject(this, &UXMoveU_SlamLayeredMoveMode::OnImpact);

	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	MoveComp->OnPostLandedDelegate.AddUObject(this, &UXMoveU_SlamLayeredMoveMode::OnImpact);
}

bool UXMoveU_SlamLayeredMoveMode::ShouldEnterMode(float DeltaSeconds)
{
	return !IsInMode() && WantsToBeInMode() && CanSlamInCurrentState();
}

bool UXMoveU_SlamLayeredMoveMode::ShouldForceLeaveMode(float DeltaSeconds)
{
	return IsInMode() && !CanSlamInCurrentState(true);
}

bool UXMoveU_SlamLayeredMoveMode::CanSlamInCurrentState(bool bIgnoreMinHeight) const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	if (!MoveComp || !MoveComp->UpdatedComponent || MoveComp->UpdatedComponent->IsSimulatingPhysics())
	{
		return false;
	}

	return MoveComp->IsFalling() && (bIgnoreMinHeight || MoveComp->GetGroundInfo().GroundDistance > SlamMinHeight);
}

void UXMoveU_SlamLayeredMoveMode::OnEnteredMode()
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	MoveComp->Velocity = FVector::ZeroVector;
}

void UXMoveU_SlamLayeredMoveMode::UpdateMode(float DeltaSeconds)
{
	if (IsInMode())
	{
		UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
		
		float VerticalVelocity = MoveComp->HasCustomGravity() ? MoveComp->GetGravitySpaceZ(MoveComp->Velocity) : MoveComp->Velocity.Z;
		VerticalVelocity += MoveComp->GetGravityZ() * SlamGravityMultiplier * DeltaSeconds;

		FVector Direction = MoveComp->GetForwardVector();
		MoveComp->Velocity = SlamHorizontalVelocity * Direction + (VerticalVelocity * -MoveComp->GetGravityDirection());
	}
}

void UXMoveU_SlamLayeredMoveMode::OnImpact(const FHitResult& Impact)
{
	if (IsInMode())
	{
		LeaveMode(false);
		OnLeftModeAfterImpact(Impact);
	}
}

void UXMoveU_SlamLayeredMoveMode::OnLeftModeAfterImpact(const FHitResult& Impact)
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	if (MoveComp->IsWalkingStrict())
	{
		FVector Direction = MoveComp->GetForwardVector();
		MoveComp->Velocity = MoveComp->GetMaxSpeed() * Direction;
	}
}
