// Fill out your copyright notice in the Description page of Project Settings.

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
