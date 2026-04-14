// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XMoveU_LayeredMove.generated.h"

class UXMoveU_PredictionManager;
class AXMoveU_ModularCharacter;
class UXMoveU_ModularMovementComponent;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class XYLOMOVEMENTUTIL_API UXMoveU_LayeredMove : public UObject
{
	GENERATED_BODY()

public:
	UXMoveU_LayeredMove(const FObjectInitializer& ObjectInitializer);
protected:
	UXMoveU_ModularMovementComponent* GetOwningMoveComp() const;
	AXMoveU_ModularCharacter* GetOwningCharacter() const;

public:
	UXMoveU_PredictionManager* GetPredictionManager() const { return PredictionManager; }
protected:
	UPROPERTY(Category="Networking", EditDefaultsOnly, Instanced)
	TObjectPtr<UXMoveU_PredictionManager> PredictionManager;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXMoveU_LayeredMove
	 */

	
};
