// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralizedPrediction/XMoveU_PredictionCharacter.h"

#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"


AXMoveU_PredictionCharacter::AXMoveU_PredictionCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UXMoveU_PredictionMovementComponent>(ACharacter::CharacterMovementComponentName))
{
}
