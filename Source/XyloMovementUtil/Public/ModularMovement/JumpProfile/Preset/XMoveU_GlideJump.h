// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/JumpProfile/XMoveU_JumpProfile.h"
#include "XMoveU_GlideJump.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_GlideJump : public UXMoveU_JumpProfile
{
	GENERATED_BODY()
	
public:
	UXMoveU_GlideJump();

public:
	virtual bool OverrideInitialImpulse() const override;
	virtual bool JumpInitialImpulse(bool bReplayingMoves, float DeltaTime) override;

	virtual bool OverrideSustainImpulse() const override;
	virtual bool JumpSustainImpulse(bool bReplayingMoves, float DeltaTime) override;
	
public:
	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float InitialVelocity;
	
	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite, Meta=(ClampMin=0.0, UIMin=0.0, ClampMax=1.0, UIMax=1.0))
	float GravityCompensation;
	
	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float HorizontalAcceleration;

	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float MaxHorizontalVelocity;
};
