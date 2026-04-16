// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/MovementMode/XMoveU_MovementMode.h"
#include "XMoveU_SlideMoveMode.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_SlideMoveMode : public UXMoveU_MovementMode
{
	GENERATED_BODY()
	
public:
	UXMoveU_SlideMoveMode(const FObjectInitializer& ObjectInitializer);
	
public:
	virtual void OnRegistered() override;
	virtual bool ShouldEnterMode() const override;
	virtual bool ShouldEnterModePostLanded(const FHitResult& Hit) override;
	virtual void OnEnteredMovementMode(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void OnLeftMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode) override;
	virtual void UpdateMode(float DeltaTime) override;
	virtual void PhysUpdate(float DeltaTime, int32 Iterations) override;

	virtual float GetModeMaxSpeed() const override { return MaxSlidingSpeed; }
	virtual float GetModeMinAnalogSpeed() const override { return MinAnalogSlidingSpeed; }
	virtual float GetModeMaxBrakingDeceleration() const override { return MaxBrakingDecelerationSliding; }

	virtual void MaintainFloorPlaneGroundVelocity();

public:
	UPROPERTY(Category = "Slide", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSlidingSpeed;
	
	UPROPERTY(Category = "Slide", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxBrakingDecelerationSliding;

	UPROPERTY(Category = "Slide", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MinAnalogSlidingSpeed;
	
	UPROPERTY(Category = "Slide", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float SlideEnterSpeed;

	UPROPERTY(Category = "Slide", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float SlideExitSpeed;
	
	UPROPERTY(Category = "Slide", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float InitialVelocityBoost;

	UPROPERTY(Category = "Slide", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float InitialVelocityBoostFalling;

	UPROPERTY(Category = "Slide", EditAnywhere, meta=(ClampMin="0", UIMin="0"))
	float SlideGravityScale;
	
	UPROPERTY(Category = "Slide", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SlidingFriction;

	UPROPERTY(Category = "Slide", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SlideCooldown;
	
	float TimeSinceLastSlide = 0.f;
};
