// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/XMoveU_PredictionCharacter.h"

#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"


AXMoveU_PredictionCharacter::AXMoveU_PredictionCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UXMoveU_PredictionMovementComponent>(ACharacter::CharacterMovementComponentName))
{
}
