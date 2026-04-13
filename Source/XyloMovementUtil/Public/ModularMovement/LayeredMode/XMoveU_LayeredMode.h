// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XMoveU_LayeredMode.generated.h"

class AXMoveU_ModularCharacter;
class UXMoveU_ModularMovementComponent;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class XYLOMOVEMENTUTIL_API UXMoveU_LayeredMode : public UObject
{
	GENERATED_BODY()

public:
	UXMoveU_LayeredMode(const FObjectInitializer& ObjectInitializer);
protected:
	UXMoveU_ModularMovementComponent* GetOwningMoveComp() const;
	AXMoveU_ModularCharacter* GetOwningCharacter() const;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXMoveU_LayeredMode
	 */

	
};
