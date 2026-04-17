// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "XMoveU_MovementModeType.h"
#include "UObject/Object.h"
#include "XMoveU_MovementMode.generated.h"

class UXMoveU_PredictionManager;
class AXMoveU_ModularCharacter;
class UXMoveU_ModularMovementComponent;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class XYLOMOVEMENTUTIL_API UXMoveU_MovementMode : public UObject
{
	GENERATED_BODY()

public:
	UXMoveU_MovementMode(const FObjectInitializer& ObjectInitializer);
protected:
	UXMoveU_ModularMovementComponent* GetOwningMoveComp() const;
	AXMoveU_ModularCharacter* GetOwningCharacter() const;

public:
	UXMoveU_PredictionManager* GetPredictionManager() const { return PredictionManager; }
protected:
	UPROPERTY(Category="Networking", EditDefaultsOnly, Instanced)
	TObjectPtr<UXMoveU_PredictionManager> PredictionManager;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXMoveU_MovementMode
	 */

public:
	virtual void OnRegistered() {}
	virtual bool IsInMode() const;

/*====================================================================================================================*/
	// MovementModeType
	
public:
	virtual bool IsFlyingMode() const { return MovementModeType == EXMoveU_MovementModeType::Flying; }
	virtual bool IsFallingMode() const { return MovementModeType == EXMoveU_MovementModeType::Falling; }
	virtual bool IsGroundMode() const { return MovementModeType == EXMoveU_MovementModeType::Ground; }
	virtual bool IsSwimmingMode() const { return MovementModeType == EXMoveU_MovementModeType::Swimming; }
	
protected:
	UPROPERTY(Category="MovementModeType", EditDefaultsOnly, BlueprintReadWrite)
	EXMoveU_MovementModeType MovementModeType = EXMoveU_MovementModeType::None;

	// ~MovementModeType
/*====================================================================================================================*/

/*====================================================================================================================*/
	// MovementModeInterface
	
public:
	virtual float GetModeMaxSpeed() const { return 0.f; }
	virtual float GetModeMaxBrakingDeceleration() const { return 0.f; }
	virtual float GetModeBrakingFriction() const { return 0.f; }
	virtual float GetModeMinAnalogSpeed() const { return 0.f; }

public:
	virtual bool CanJumpInCurrentMode() const { return true; }
	virtual bool CanCrouchInCurrentMode() const { return true; }

public:
	virtual bool ShouldEnterMode() const { return false; }
	virtual bool ShouldEnterModePostLanded(const FHitResult& Hit) { return false; }
	virtual void OnEnteredMovementMode(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) {}
	virtual void OnLeftMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode) {}
	virtual void UpdateMode(float DeltaTime) {}
	virtual void PhysUpdate(float DeltaTime, int32 Iterations) {}

	// ~MovementModeInterface
/*====================================================================================================================*/
	
};
