// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/LayeredMovementMode/XMoveU_LayeredMovementMode.h"
#include "XMoveU_DashLayeredMoveMode.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_DashLayeredMoveMode : public UXMoveU_LayeredMovementMode
{
	GENERATED_BODY()
	
public:
	UXMoveU_DashLayeredMoveMode(const FObjectInitializer& ObjectInitializer);
	
public:
	virtual void OnRegistered(uint32 InModeIndex) override;
	virtual bool ShouldEnterMode(float DeltaSeconds) const override;
	virtual bool ShouldForceLeaveMode(float DeltaSeconds) const override;
protected:
	virtual bool CanDashInCurrentState(bool bIgnoreDeadZone = false) const;
	virtual bool IsInputInDeadZone() const;
	
	virtual void OnEnteredMode() override;
	virtual void UpdateMode(float DeltaSeconds) override;

protected:
	UPROPERTY(Category="Dash", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float DashVerticalImpulseSpeed;
	
	UPROPERTY(Category="Dash", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float DashHorizontalImpulseSpeed;

	UPROPERTY(Category="Dash", EditAnywhere, BlueprintReadWrite, meta=(InlineEditConditionToggle))
	bool bUseDeadZoneAngle;
	
	UPROPERTY(Category="Dash", EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bUseDeadZoneAngle", ClampMin="0", UIMin="0"))
	float DashAngleCosineDeadZone;

	UPROPERTY(Category="Dash", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float DashDuration;

	UPROPERTY(Category="Dash", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	int32 DashMaxCharges;
	
	UPROPERTY(Category="Dash", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float DashRechargeTime;

	float TimeSinceDash = 0.f;

	float DashCharge = 0.f;

	FVector CachedDashDirection;
};
