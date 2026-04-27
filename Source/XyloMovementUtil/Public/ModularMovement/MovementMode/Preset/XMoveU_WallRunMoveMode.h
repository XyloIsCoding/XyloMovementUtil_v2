// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/MovementMode/XMoveU_MovementMode.h"
#include "XMoveU_WallRunMoveMode.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_WallRunMoveMode : public UXMoveU_MovementMode
{
	GENERATED_BODY()

public:
	UXMoveU_WallRunMoveMode(const FObjectInitializer& ObjectInitializer);

public:
	virtual bool ShouldEnterMode() override;
	virtual void OnEnteredMovementMode(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void OnLeftMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode) override;
	virtual void UpdateMode(float DeltaTime) override;
	virtual void PhysUpdate(float DeltaTime, int32 Iterations) override;
	
	virtual float GetModeMaxSpeed() const override { return MaxWallRunningSpeed; }
	virtual float GetModeMinAnalogSpeed() const override { return MinAnalogWallRunningSpeed; }
	virtual float GetModeMaxBrakingDeceleration() const override { return MaxBrakingDecelerationWallRunning; }
	virtual float GetModeBrakingFriction() const override { return WallRunningFriction; }

protected:
	virtual bool CanWallRunInCurrentState() const;
	virtual bool FindWall(FHitResult& OutWallHit, const FVector& Direction, float Distance);
	virtual void MaintainWallPlaneVelocity();

public:
	UPROPERTY(Category = "WallRun", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxWallRunningSpeed;
	
	UPROPERTY(Category = "WallRun", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxBrakingDecelerationWallRunning;

	UPROPERTY(Category = "WallRun", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MinAnalogWallRunningSpeed;

	UPROPERTY(Category = "WallRun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float WallRunningFriction;

	UPROPERTY(Category = "WallRun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MaxWallDistance;

	UPROPERTY(Category = "WallRun", EditAnywhere, meta=(ForceUnits="cm/s"))
	float WallRunMinEnterVerticalSpeed;
	
	UPROPERTY(Category = "WallRun", EditAnywhere, meta=(ForceUnits="cm/s"))
	float WallRunVerticalSpeedDetachThreshold;

	UPROPERTY(Category = "WallRun", EditAnywhere)
	float WallRunLeaveAngleCosine;

	UPROPERTY(Category = "WallRun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallAttractionForce;

	UPROPERTY(Category = "WallRun|Gravity", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float WallRunDescendingGravityScale;

	UPROPERTY(Category = "WallRun|Gravity", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float WallRunMinAscendingGravityScale;

	UPROPERTY(Category = "WallRun|Gravity", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float WallRunMaxAscendingGravityScale;

	UPROPERTY(Category = "WallRun|Gravity", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ClampMax="1", UIMax="1"))
	float AccelerationMultiplierWhenFacingWall;

	UPROPERTY(Category = "WallRun|Climb", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ClampMax="1", UIMax="1"))
	float MinClimbWallAngleCosine;
	
	UPROPERTY(Category = "WallRun|Climb", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallClimbVelocity;

	UPROPERTY(Category = "WallRun|Climb", EditAnywhere, BlueprintReadWrite, meta=(ForceUnits="cm/s"))
	float ClimbMinVelocityZ;
	
public:
	UPROPERTY(Category="WallRun", VisibleInstanceOnly, BlueprintReadOnly)
	FHitResult CurrentWall;
};
