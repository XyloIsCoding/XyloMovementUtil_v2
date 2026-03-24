// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "XMoveU_PredictionCharacter.generated.h"

UCLASS()
class XYLOMOVEMENTUTIL_API AXMoveU_PredictionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AXMoveU_PredictionCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
