// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_SavedMove_Character.h"

#include "GameFramework/Character.h"
#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"
#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_NetworkPredictionData_Client_Character.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"

void FXMoveU_SavedMove_Character::Clear()
{
	FSavedMove_Character::Clear();

	// Call UXMoveU_PredictionManager::Clear
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(CharacterOwner->GetCharacterMovement(),
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->Clear(*this);
	});
	
	// Call FXMoveU_PredictionProxy::Reset
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.Reset(Blackboard);
	});
}

bool FXMoveU_SavedMove_Character::IsImportantMove(const FSavedMovePtr& LastAckedMove) const
{
	// Collecting state this way so proxies and manager can implement custom functionality if a move is found to be important.
	bool bIsImportantMove = FSavedMove_Character::IsImportantMove(LastAckedMove);
	
	const FXMoveU_SavedMove_Character* XMoveU_LastAckedMove = static_cast<const FXMoveU_SavedMove_Character*>(LastAckedMove.Get());

	// Call UXMoveU_PredictionManager::IsImportantMove
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(CharacterOwner->GetCharacterMovement(),
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		bIsImportantMove |= PredictionManager->IsImportantMove(*this, XMoveU_LastAckedMove);
	});

	// Call FXMoveU_PredictionProxy::IsImportantFrame
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		bIsImportantMove |= PredictionProxy.IsImportantFrame(Blackboard, XMoveU_LastAckedMove->Blackboard);
	});

	return bIsImportantMove;
}

bool FXMoveU_SavedMove_Character::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	// Collecting state this way so proxies and manager can implement custom functionality
	bool bCanCombine = Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
	
	const FXMoveU_SavedMove_Character* XMoveU_NewMove = static_cast<const FXMoveU_SavedMove_Character*>(NewMove.Get());

	// Call UXMoveU_PredictionManager::CanCombineWith
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(CharacterOwner->GetCharacterMovement(),
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		bCanCombine &= PredictionManager->CanCombineWith(*this, *XMoveU_NewMove, InCharacter, MaxDelta);
	});

	// Call FXMoveU_PredictionProxy::CanCombineWithNewFrame
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		bCanCombine &= PredictionProxy.CanCombineWithNewFrame(Blackboard, XMoveU_NewMove->Blackboard);
	});

	return bCanCombine;
}

void FXMoveU_SavedMove_Character::CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation)
{
	FSavedMove_Character::CombineWith(OldMove, InCharacter, PC, OldStartLocation);

	const FXMoveU_SavedMove_Character* XMoveU_OldMove = static_cast<const FXMoveU_SavedMove_Character*>(OldMove);
	
	// Call UXMoveU_PredictionManager::CombineWith
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(CharacterOwner->GetCharacterMovement(),
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->CombineWith(*this, XMoveU_OldMove, InCharacter, PC, OldStartLocation);
	});
	
	// Call FXMoveU_PredictionProxy::CombineWith
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.RevertFrameChanges(XMoveU_OldMove->Blackboard);
	});
}

void FXMoveU_SavedMove_Character::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	FXMoveU_NetworkPredictionData_Client_Character& XMoveU_ClientData = static_cast<FXMoveU_NetworkPredictionData_Client_Character&>(ClientData);
	
	// Call UXMoveU_PredictionManager::SetMoveFor
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(CharacterOwner->GetCharacterMovement(),
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->SetMoveFor(*this, C, InDeltaTime, NewAccel, XMoveU_ClientData);
	});
	
	// Call FXMoveU_PredictionProxy::CollectCurrentState
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.CollectCurrentState(Blackboard);
	});

	// Call FXMoveU_PredictionProxy::BlockCombinePreSimulation
	bool bBlockCombine = false;
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		if (PredictionProxy.BlockCombinePreSimulation(Blackboard)) { bBlockCombine = true; }
	});
	bForceNoCombine |= bBlockCombine;
}

void FXMoveU_SavedMove_Character::SetInitialPosition(ACharacter* C)
{
	FSavedMove_Character::SetInitialPosition(C);

	// Call UXMoveU_PredictionManager::SetInitialPosition
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(CharacterOwner->GetCharacterMovement(),
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->SetInitialPosition(*this, C);
	});
	
	// Call FXMoveU_PredictionProxy::CollectDynamicState
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.CollectDynamicState(Blackboard);
	});
}

void FXMoveU_SavedMove_Character::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	// Call UXMoveU_PredictionManager::PrepMoveFor
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(CharacterOwner->GetCharacterMovement(),
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->PrepMoveFor(*this, C);
	});
	
	// Call FXMoveU_PredictionProxy::RollbackToFrame
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.RollbackToFrame(Blackboard);
	});
}

void FXMoveU_SavedMove_Character::PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode)
{
	FSavedMove_Character::PostUpdate(C, PostUpdateMode);

	// Call UXMoveU_PredictionManager::PostUpdate
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(CharacterOwner->GetCharacterMovement(),
	[&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->PostUpdate(*this, C, PostUpdateMode);
	});
	
	// Call FXMoveU_PredictionProxy::CollectFinalState
	UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
	[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.CollectFinalState(Blackboard, PostUpdateMode);
	});

	if (PostUpdateMode == EPostUpdateMode::PostUpdate_Record)
	{
		// Call FXMoveU_PredictionProxy::BlockCombinePostSimulation
		bool bBlockCombine = false;
		UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(CharacterOwner->GetCharacterMovement(),
		[&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
		{
			if (PredictionProxy.BlockCombinePostSimulation(Blackboard)) { bBlockCombine = true; }
		});
		bForceNoCombine |= bBlockCombine;
	}
}
