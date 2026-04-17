// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XMoveU_LayeredMovementMode.generated.h"

class UXMoveU_PredictionManager;
class AXMoveU_ModularCharacter;
class UXMoveU_ModularMovementComponent;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class XYLOMOVEMENTUTIL_API UXMoveU_LayeredMovementMode : public UObject
{
	GENERATED_BODY()

public:
	UXMoveU_LayeredMovementMode(const FObjectInitializer& ObjectInitializer);
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
	 * UXMoveU_LayeredMovementMode
	 */

public:
	virtual void OnRegistered(uint32 InModeIndex);

public:
	virtual void RequestMode(bool bWantsToEnterMode);
	virtual bool WantsToBeInMode() const;
	virtual void SetModeState(bool bIsInMode);
	virtual bool IsInMode() const;
	virtual void ReplicateStateToSimProxies();
	virtual bool ShouldCancelRequestAfterTransitionCheck() const { return bCancelRequestAfterTransitionCheck; }
	
protected:
	UPROPERTY(Category="Settings", EditDefaultsOnly)
	bool bCancelRequestAfterTransitionCheck;
	
	bool bModeRequested = false;
	
	/** The index this mode was registered with. Used to access the mode state on the character */
	uint32 ModeIndex = 0;
	bool bRegistered = false;

/*====================================================================================================================*/
	// LayeredMovementModeInterface

public:
	virtual void ModifyMaxSpeed(float& OutMaxSpeed) const {}
	virtual void ModifyBrakingFriction(float& OutBrakingFriction) const {}
	virtual void ModifyBrakingDeceleration(float& OutBrakingDeceleration) const {}
	
public:
	virtual bool ShouldEnterMode(float DeltaSeconds) const { return false; }
	virtual bool ShouldLeaveMode(float DeltaSeconds) const { return false; }
	virtual bool ShouldForceLeaveMode(float DeltaSeconds) const { return false; }

public:
	virtual void EnterMode(bool bClientSimulation);
	virtual void LeaveMode(bool bClientSimulation);
	virtual void UpdateMode(float DeltaSeconds) {}
	
public:
	virtual void OnEnteredMode() {}
	virtual void OnLeftMode() {}

	// ~LayeredMovementModeInterface
/*====================================================================================================================*/
	
};
