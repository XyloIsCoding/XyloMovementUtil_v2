// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/MovementMode/XMoveU_MovementMode.h"
#include "XMoveU_WallRunMoveMode.generated.h"

class UXMoveU_JumpProfile;

USTRUCT(BlueprintType)
struct XYLOMOVEMENTUTIL_API FXMoveU_WallData
{
	GENERATED_BODY()

	/** Last valid wall normal. */
	FVector LastWallNormal;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FHitResult WallHit;
};

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
	
	virtual float GetModeMaxSpeed() const override;
	virtual float GetModeMinAnalogSpeed() const override { return MinAnalogWallRunningSpeed; }
	virtual float GetModeMaxBrakingDeceleration() const override { return MaxBrakingDecelerationWallRunning; }
	virtual float GetModeBrakingFriction() const override { return WallRunningFriction; }

	virtual UXMoveU_JumpProfile* GetJumpProfileOverride() const override { return JumpProfile; }

public:
	virtual bool CanWallRunInCurrentState() const;
	virtual bool FindWall(FXMoveU_WallData& OutWallData, const FVector& Direction, float Distance);
	virtual bool IsClimbing() const;

	/** Get the height the hans are at relative to capsule center. This is just to calculate how high onto the
	 * wall we can climb / run. A wall lower than CapsuleHalfHeight + HandsHeight is not suitable for wall run. */
	virtual float GetHandsHeight() const;
	/** Checks if there is still a wall where we need to place our hands. Requires a valid CurrentWall. */
	virtual bool FindWallAtHandsHeight(FHitResult& OutWallHit, const FVector& PositionOffset = FVector::ZeroVector);
	
protected:
	virtual void MaintainWallPlaneVelocity();
	virtual void OnWallEnded(float remainingTime, int32 Iterations);

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

	UPROPERTY(Category = "WallRun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ClampMax="1", UIMax="1"))
	float WallAccelerationDeadZoneAngleCosine;

	/** Relative to capsule center */
	UPROPERTY(Category = "WallRun", EditAnywhere, BlueprintReadWrite)
	float WallRunHandsHeight;

	/** Relative to capsule bottom */
	UPROPERTY(Category = "WallRun", EditAnywhere, BlueprintReadWrite)
	float WallRunFeetHeight;
	
	UPROPERTY(Category = "WallRun", EditAnywhere)
	float WallRunReentryTime;

	UPROPERTY(Category = "WallRun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallAttractionForce;

	UPROPERTY(Category = "WallRun|Gravity", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float WallRunDescendingGravityScale;

	UPROPERTY(Category = "WallRun|Gravity", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float WallRunMinAscendingGravityScale;

	UPROPERTY(Category = "WallRun|Gravity", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float WallRunMaxAscendingGravityScale;

	UPROPERTY(Category = "WallRun|Climb", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ClampMax="1", UIMax="1"))
	float MinClimbWallAngleCosine;
	
	UPROPERTY(Category = "WallRun|Climb", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallClimbLateralVelocity;

	UPROPERTY(Category = "WallRun|Climb", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallClimbVerticalVelocity;

	UPROPERTY(Category = "WallRun|Climb", EditAnywhere, BlueprintReadWrite, meta=(ForceUnits="cm/s"))
	float ClimbMinVelocityZ;

	/** Relative to capsule center */
	UPROPERTY(Category = "WallRun|Climb", EditAnywhere, BlueprintReadWrite)
	float ClimbHandsHeight;
	
	UPROPERTY(Category="WallRun|Jump", EditDefaultsOnly, Instanced)
	TObjectPtr<UXMoveU_JumpProfile> JumpProfile;
	
public:
	UPROPERTY(Category="WallRun", VisibleInstanceOnly, BlueprintReadOnly)
	FXMoveU_WallData CurrentWall;

	UPROPERTY(Category="WallRun", VisibleInstanceOnly, BlueprintReadOnly)
	float WallRunReentryLockTimeRemaining = 0.f;
};
