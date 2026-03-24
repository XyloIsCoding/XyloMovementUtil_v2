// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularMovement/XMoveU_ModularCharacter.h"

#include "ModularMovement/XMoveU_ModularMovementComponent.h"


AXMoveU_ModularCharacter::AXMoveU_ModularCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UXMoveU_ModularMovementComponent>(ACharacter::CharacterMovementComponentName))
{
}
