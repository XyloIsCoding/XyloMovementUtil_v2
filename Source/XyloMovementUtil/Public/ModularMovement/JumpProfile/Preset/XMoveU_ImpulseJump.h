// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/JumpProfile/XMoveU_JumpProfile.h"
#include "XMoveU_ImpulseJump.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_ImpulseJump : public UXMoveU_JumpProfile
{
	GENERATED_BODY()
	
public:
	UXMoveU_ImpulseJump();

public:
	virtual bool OverrideInitialImpulse() const override;
	virtual bool JumpInitialImpulse(bool bReplayingMoves, float DeltaTime) override;

	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float VerticalVelocity;

	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float HorizontalVelocity;
};
