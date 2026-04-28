// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XMoveU_JumpProfile.generated.h"

class AXMoveU_ModularCharacter;
class UXMoveU_ModularMovementComponent;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class XYLOMOVEMENTUTIL_API UXMoveU_JumpProfile : public UObject
{
	GENERATED_BODY()

public:
	UXMoveU_JumpProfile(const FObjectInitializer& ObjectInitializer);
public:
	virtual UXMoveU_ModularMovementComponent* GetOwningMovementComponent() const;
	virtual AXMoveU_ModularCharacter* GetOwningCharacter() const;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXMoveU_JumpProfile
	 */
	
public:
	virtual void ApplyJumpProfile();
	virtual void RemoveJumpProfile();

public:
	virtual bool OverrideInitialImpulse() const { return false; }
	virtual bool JumpInitialImpulse(bool bReplayingMoves, float DeltaTime) { return true; }

	virtual bool OverrideSustainImpulse() const { return false; }
	virtual bool JumpSustainImpulse(bool bReplayingMoves, float DeltaTime) { return true; }
	
protected:
	UPROPERTY(Category = "Character", EditAnywhere, BlueprintReadWrite, Meta=(ClampMin=0.0, UIMin=0.0))
	float JumpMaxHoldTime = 0.f;

	UPROPERTY(Category = "Character", EditAnywhere, BlueprintReadWrite)
	int32 JumpMaxCount = 0;
};
