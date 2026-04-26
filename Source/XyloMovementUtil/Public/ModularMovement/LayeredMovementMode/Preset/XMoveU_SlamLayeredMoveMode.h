// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularMovement/LayeredMovementMode/XMoveU_LayeredMovementMode.h"
#include "XMoveU_SlamLayeredMoveMode.generated.h"

/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_SlamLayeredMoveMode : public UXMoveU_LayeredMovementMode
{
	GENERATED_BODY()

public:
	UXMoveU_SlamLayeredMoveMode(const FObjectInitializer& ObjectInitializer);

public:
	virtual void OnRegistered(uint32 InModeIndex) override;
	
public:
	virtual bool ShouldEnterMode(float DeltaSeconds) override;
	virtual bool ShouldForceLeaveMode(float DeltaSeconds) override;
protected:
	virtual bool CanSlamInCurrentState(bool bIgnoreMinHeight = false) const;
	
	virtual void OnEnteredMode() override;
	virtual void UpdateMode(float DeltaSeconds) override;

	virtual void OnImpact(const FHitResult& Impact);
	virtual void OnLeftModeAfterImpact(const FHitResult& Impact);

protected:
	UPROPERTY(Category="Slam", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float SlamGravityMultiplier;

	UPROPERTY(Category="Slam", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float SlamHorizontalVelocity;

	UPROPERTY(Category="Slam", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SlamMinHeight;
};
