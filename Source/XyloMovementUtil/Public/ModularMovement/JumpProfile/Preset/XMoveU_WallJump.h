// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/JumpProfile/XMoveU_JumpProfile.h"
#include "XMoveU_WallJump.generated.h"

/**
 * Can only be used in combination with WallRunMoveMode
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_WallJump : public UXMoveU_JumpProfile
{
	GENERATED_BODY()

public:
	virtual bool OverrideInitialImpulse() const override;
	virtual bool JumpInitialImpulse(bool bReplayingMoves, float DeltaTime) override;

	UPROPERTY(Category="Jump", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallPushVelocity;

	UPROPERTY(Category="Jump", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallPushVelocityClimbing;
};
