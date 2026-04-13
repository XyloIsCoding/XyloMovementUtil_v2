// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/JumpProfile/XMoveU_JumpProfile.h"
#include "XMoveU_ThrusterJump.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_ThrusterJump : public UXMoveU_JumpProfile
{
	GENERATED_BODY()
	
public:
	UXMoveU_ThrusterJump(const FObjectInitializer& ObjectInitializer);

public:
	virtual bool OverrideInitialImpulse() const override;
	virtual bool JumpInitialImpulse(bool bReplayingMoves, float DeltaTime) override;

	virtual bool OverrideSustainImpulse() const override;
	virtual bool JumpSustainImpulse(bool bReplayingMoves, float DeltaTime) override;
	
public:
	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float InitialVerticalVelocity;

	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float InitialHorizontalVelocity;
	
	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float VerticalAcceleration;

	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float MaxVerticalVelocity;

	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float MinVerticalVelocity;
	
	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float HorizontalAcceleration;

	UPROPERTY(Category = "Jump", EditAnywhere, BlueprintReadWrite)
	float MaxHorizontalVelocity;
};
