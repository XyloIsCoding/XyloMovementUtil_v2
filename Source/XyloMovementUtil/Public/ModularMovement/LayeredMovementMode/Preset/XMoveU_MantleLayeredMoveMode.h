// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RootMotionSource.h"
#include "ModularMovement/LayeredMovementMode/XMoveU_LayeredMovementMode.h"
#include "XMoveU_MantleLayeredMoveMode.generated.h"

/**
 *
 */
USTRUCT()
struct XYLOMOVEMENTUTIL_API FXMoveU_RootMotionSource_Mantle : public FRootMotionSource
{
	GENERATED_USTRUCT_BODY()

	FXMoveU_RootMotionSource_Mantle();

	virtual ~FXMoveU_RootMotionSource_Mantle() {}

	virtual void CalculateMantleTrajectory(const FVector& Start, const FVector& Mid, const FVector& End);

	UPROPERTY()
	FInterpCurveVector MantlePathCurve;

	UPROPERTY()
	bool bRestrictSpeedToExpected;

	virtual FRootMotionSource* Clone() const override;

	virtual bool Matches(const FRootMotionSource* Other) const override;

	virtual bool MatchesAndHasSameState(const FRootMotionSource* Other) const override;

	virtual bool UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup = false) override;

	virtual void SetTime(float NewTime) override;

	virtual void PrepareRootMotion(
		float SimulationTime, 
		float MovementTickTime,
		const ACharacter& Character, 
		const UCharacterMovementComponent& MoveComponent
		) override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	virtual UScriptStruct* GetScriptStruct() const override;

	virtual FString ToSimpleString() const override;

	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
};

template<>
struct TStructOpsTypeTraits< FXMoveU_RootMotionSource_Mantle > : public TStructOpsTypeTraitsBase2< FXMoveU_RootMotionSource_Mantle >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*====================================================================================================================*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * 
 */
UCLASS()
class XYLOMOVEMENTUTIL_API UXMoveU_MantleLayeredMoveMode : public UXMoveU_LayeredMovementMode
{
	GENERATED_BODY()

public:
	UXMoveU_MantleLayeredMoveMode(const FObjectInitializer& ObjectInitializer);

public:
	virtual bool ShouldReplaceJump(float DeltaSeconds) override;
	virtual bool ShouldForceLeaveMode(float DeltaSeconds) override;
	virtual void OnEnteredMode() override;
	virtual void OnLeftMode() override;

protected:
	virtual bool FindWall(const FVector& Direction, FHitResult& OutWallHit) const;
	virtual bool FindLedge(const FVector& Direction, const FHitResult& WallHit, FHitResult& OutLedgeHit) const;
	virtual bool IsLedgeValid(const FHitResult& WallHit, const FHitResult& LedgeHit) const;

public:
	/** The cosine of the maximum allowed angle between wall normal and both acceleration and view direction. */
	UPROPERTY(Category="Mantle", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MinMantleAngleCosine;
	
	UPROPERTY(Category="Mantle", EditAnywhere, meta=(ClampMin="0", UIMin="0"))
	float MaxHorizontalWallReach;
	
	UPROPERTY(Category="Mantle", EditAnywhere, meta=(ClampMin="0", UIMin="0"))
	float MaxVerticalWallReach;

	UPROPERTY(Category="Mantle", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MantleSpeed;
	
	UPROPERTY(Category="Mantle", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float PostMantleHorizontalVelocity;
	
	UPROPERTY(Category="Mantle", EditAnywhere, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float PostMantleVerticalVelocity;

protected:
	TSharedPtr<FXMoveU_RootMotionSource_Mantle> MantleRMS;
	uint16 MantleRMS_ID = 0;
	
};
