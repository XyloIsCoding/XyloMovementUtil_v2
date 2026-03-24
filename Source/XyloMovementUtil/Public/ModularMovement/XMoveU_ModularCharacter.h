// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/XMoveU_PredictionCharacter.h"
#include "XMoveU_ModularCharacter.generated.h"


/**
 *
 */
UCLASS()
class XYLOMOVEMENTUTIL_API AXMoveU_ModularCharacter : public AXMoveU_PredictionCharacter
{
	GENERATED_BODY()

public:
	AXMoveU_ModularCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
