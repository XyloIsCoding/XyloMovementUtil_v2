// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/LayeredMovementMode/XMoveU_LayeredMovementMode.h"
#include "XMoveU_SprintLayeredMoveMode.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_SprintLayeredMoveMode : public UXMoveU_LayeredMovementMode
{
	GENERATED_BODY()

public:
	UXMoveU_SprintLayeredMoveMode(const FObjectInitializer& ObjectInitializer);

public:
	virtual void ModifyMaxSpeed(float& OutMaxSpeed) const override;
	
public:
	virtual bool ShouldEnterMode(float DeltaSeconds) override;
	virtual bool ShouldLeaveMode(float DeltaSeconds) override;
	virtual bool ShouldForceLeaveMode(float DeltaSeconds) override;
protected:
	virtual bool CanSprintInCurrentState() const;

protected:
	/** The maximum ground speed when walking and Sprinting. */
	UPROPERTY(Category="Sprinting", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxWalkSpeedSprinting;

	/** The cosine of the maximum angle sprint is allowed. */
	UPROPERTY(Category="Sprinting", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MinSprintingAngleCosine;
};
