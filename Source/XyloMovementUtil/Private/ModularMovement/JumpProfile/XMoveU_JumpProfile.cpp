// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "ModularMovement/JumpProfile/XMoveU_JumpProfile.h"

#include "GameFramework/Character.h"
#include "ModularMovement/XMoveU_ModularCharacter.h"
#include "ModularMovement/XMoveU_ModularMovementComponent.h"

UXMoveU_JumpProfile::UXMoveU_JumpProfile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UXMoveU_ModularMovementComponent* UXMoveU_JumpProfile::GetOwningMovementComponent() const
{
	return Cast<UXMoveU_ModularMovementComponent>(GetOuter());
}

AXMoveU_ModularCharacter* UXMoveU_JumpProfile::GetOwningCharacter() const
{
	UXMoveU_ModularMovementComponent* MovementComponent = GetOwningMovementComponent();
	return MovementComponent ? Cast<AXMoveU_ModularCharacter>(MovementComponent->GetCharacterOwner()) : nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXMoveU_JumpProfile
 */

void UXMoveU_JumpProfile::ApplyJumpProfile(UXMoveU_ModularMovementComponent* MovementComponent)
{
	ACharacter* CharacterOwner = MovementComponent ? MovementComponent->GetCharacterOwner() : nullptr;
	if (!CharacterOwner)
	{
		return;
	}

	CharacterOwner->JumpMaxCount = JumpMaxCount;
	CharacterOwner->JumpMaxHoldTime = JumpMaxHoldTime;
}

void UXMoveU_JumpProfile::RemoveJumpProfile(UXMoveU_ModularMovementComponent* MovementComponent)
{
	ACharacter* CharacterOwner = MovementComponent ? MovementComponent->GetCharacterOwner() : nullptr;
	if (!CharacterOwner)
	{
		return;
	}
	
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	CharacterOwner->JumpMaxCount = DefaultCharacter->JumpMaxCount;
	CharacterOwner->JumpMaxHoldTime = DefaultCharacter->JumpMaxHoldTime;
}
