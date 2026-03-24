// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"
#include "XMoveU_ModularMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYLOMOVEMENTUTIL_API UXMoveU_ModularMovementComponent : public UXMoveU_PredictionMovementComponent
{
	GENERATED_BODY()

public:
	UXMoveU_ModularMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
