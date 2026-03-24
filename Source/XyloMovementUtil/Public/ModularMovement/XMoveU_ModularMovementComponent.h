// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"
#include "XMoveU_ModularMovementComponent.generated.h"


/**
 * Adds modular system for CustomMovementModes, LayeredMoves, and JumpProfiles
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYLOMOVEMENTUTIL_API UXMoveU_ModularMovementComponent : public UXMoveU_PredictionMovementComponent
{
	GENERATED_BODY()

public:
	UXMoveU_ModularMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
