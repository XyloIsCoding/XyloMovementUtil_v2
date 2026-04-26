// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/MovementMode/Preset/XMoveU_WallRunMoveMode.h"

#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_WallRunMoveMode::UXMoveU_WallRunMoveMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UXMoveU_WallRunMoveMode::ShouldEnterMode()
{
	return Super::ShouldEnterMode();
}

void UXMoveU_WallRunMoveMode::OnEnteredMovementMode(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnEnteredMovementMode(PreviousMovementMode, PreviousCustomMode);
}

void UXMoveU_WallRunMoveMode::OnLeftMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	Super::OnLeftMovementMode(NewMovementMode, NewCustomMode);
}

void UXMoveU_WallRunMoveMode::UpdateMode(float DeltaTime)
{
	Super::UpdateMode(DeltaTime);
}

void UXMoveU_WallRunMoveMode::PhysUpdate(float DeltaTime, int32 Iterations)
{
	Super::PhysUpdate(DeltaTime, Iterations);
}

bool UXMoveU_WallRunMoveMode::CanWallRunInCurrentState() const
{
	UXMoveU_ModularMovementComponent* MoveComp = GetOwningMoveComp();
	return !IsInMode() && MoveComp->IsFalling(); 
}
