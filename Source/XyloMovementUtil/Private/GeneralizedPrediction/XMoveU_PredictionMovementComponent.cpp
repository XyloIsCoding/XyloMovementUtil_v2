// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/XMoveU_PredictionMovementComponent.h"

#include "XyloMovementUtil.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameNetworkManager.h"
#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"
#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_CharacterMoveResponseDataContainer.h"
#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_CharacterNetworkMoveData.h"
#include "GeneralizedPrediction/MovementComponentPrediction/XMoveU_NetworkPredictionData_Client_Character.h"
#include "Misc/DefinePrivateMemberPtr.h"
#include "Net/PerfCountersHelpers.h"


// @XMoveU - @AfterUpdatingEngine: Check if engine provided an actual setter for this.
UE_DEFINE_PRIVATE_MEMBER_PTR(TWeakObjectPtr<UPrimitiveComponent>, GLastServerMovementBase, UCharacterMovementComponent, LastServerMovementBase);


// Mirrors the declarations in UCharacterMovementComponent
// @XMoveU - @AfterUpdatingEngine: check that the declarations are still the same in UCharacterMovementComponent.
static const FString PerfCounter_NumServerMoves = TEXT("NumServerMoves");
static const FString PerfCounter_NumServerMoveCorrections = TEXT("NumServerMoveCorrections");

// Caches CVars from base class and provides getters
// @XMoveU - @AfterUpdatingEngine: check that CVars did not change.
namespace CharacterMovementCVars
{
	static IConsoleVariable* CVar_ClientAuthorityThresholdOnBaseChange = IConsoleManager::Get().FindConsoleVariable(TEXT("p.ClientAuthorityThresholdOnBaseChange"));
	static IConsoleVariable* CVar_MaxFallingCorrectionLeash = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxFallingCorrectionLeash"));
	static IConsoleVariable* CVar_MaxFallingCorrectionLeashBuffer = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxFallingCorrectionLeashBuffer"));
	static IConsoleVariable* CVar_NetUseBaseRelativeVelocity = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetUseBaseRelativeVelocity"));
	static IConsoleVariable* CVar_NetShowCorrections = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetShowCorrections"));
	static IConsoleVariable* CVar_NetCorrectionLifetime = IConsoleManager::Get().FindConsoleVariable(TEXT("p.NetCorrectionLifetime"));
	
	float Get_ClientAuthorityThresholdOnBaseChange()
	{
		return CharacterMovementCVars::CVar_ClientAuthorityThresholdOnBaseChange->GetFloat();
	}

	float Get_MaxFallingCorrectionLeash()
	{
		return CharacterMovementCVars::CVar_MaxFallingCorrectionLeash->GetFloat();
	}

	float Get_MaxFallingCorrectionLeashBuffer()
	{
		return CharacterMovementCVars::CVar_MaxFallingCorrectionLeashBuffer->GetFloat();
	}

	int32 Get_NetUseBaseRelativeVelocity()
	{
		return CharacterMovementCVars::CVar_NetUseBaseRelativeVelocity->GetInt();
	}

	int32 Get_NetShowCorrections()
	{
		return CharacterMovementCVars::CVar_NetShowCorrections->GetInt();
	}

	float Get_NetCorrectionLifetime()
	{
		return CharacterMovementCVars::CVar_NetCorrectionLifetime->GetFloat();
	}
}


UXMoveU_PredictionMovementComponent::UXMoveU_PredictionMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetMoveResponseDataContainer(MoveResponseDataContainer);
	SetNetworkMoveDataContainer(NetworkMoveDataContainer);
}

FNetworkPredictionData_Client* UXMoveU_PredictionMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UXMoveU_PredictionMovementComponent* MutableThis = const_cast<UXMoveU_PredictionMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FXMoveU_NetworkPredictionData_Client_Character(*this);
	}

	return ClientPredictionData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UCharacterMovementComponent Interface
 */

void UXMoveU_PredictionMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (DefaultPredictionManager)
	{
		RegisterPredictionManager(DefaultPredictionManager);
	}
}

void UXMoveU_PredictionMovementComponent::ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData)
{
	ServerGetClientInputs(MoveData);
	Super::ServerMove_PerformMovement(MoveData);
}

void UXMoveU_PredictionMovementComponent::OnClientCorrectionReceived(class FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection)
{
	Super::OnClientCorrectionReceived(ClientData, TimeStamp, NewLocation, NewVelocity, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode, ServerGravityDirection);

	const FXMoveU_CharacterMoveResponseDataContainer& XMoveU_MoveResponse = static_cast<const FXMoveU_CharacterMoveResponseDataContainer&>(GetMoveResponseDataContainer());
	const TSharedPtr<FXMoveU_SavedMove_Character> XMoveU_LastAckedMove = StaticCastSharedPtr<FXMoveU_SavedMove_Character>(ClientData.LastAckedMove);
	FXMoveU_NetworkPredictionData_Client_Character& XMoveU_ClientData = static_cast<FXMoveU_NetworkPredictionData_Client_Character&>(ClientData);
	
	// Call UXMoveU_PredictionManager::OnClientCorrectionReceived
	ExecuteOnPredictionManagers(this, [&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->OnClientCorrectionReceived(this, XMoveU_ClientData, TimeStamp, NewLocation, NewVelocity, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode, ServerGravityDirection);
	});
	
	// Call FXMoveU_PredictionProxy::ApplyCorrectedState
	ExecuteOnPredictionProxies(this, [&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		// TODO: check if it is possible for ClientData.LastAckedMove to be nullptr. Cause that will lead to crashes inside prediction proxies
		PredictionProxy.ApplyCorrectedState(XMoveU_MoveResponse.Blackboard, XMoveU_LastAckedMove.IsValid() ? &XMoveU_LastAckedMove->Blackboard : nullptr, EXMoveU_CorrectionContext::OnReceive);
	});
}

bool UXMoveU_PredictionMovementComponent::ClientUpdatePositionAfterServerUpdate()
{
	const FXMoveU_CharacterMoveResponseDataContainer& XMoveU_MoveResponse = static_cast<const FXMoveU_CharacterMoveResponseDataContainer&>(GetMoveResponseDataContainer());
	const TSharedPtr<FXMoveU_SavedMove_Character> XMoveU_LastAckedMove = StaticCastSharedPtr<FXMoveU_SavedMove_Character>(GetPredictionData_Client_Character()->LastAckedMove);
	FXMoveU_NetworkPredictionData_Client_Character& XMoveU_ClientData = static_cast<FXMoveU_NetworkPredictionData_Client_Character&>(*GetPredictionData_Client_Character());
	
	// Call UXMoveU_PredictionManager::CorrectClientPreRollback
	ExecuteOnPredictionManagers(this, [&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->CorrectClientPreRollback(this, XMoveU_MoveResponse, XMoveU_ClientData);
	});
	
	// Call FXMoveU_PredictionProxy::ApplyCorrectedState
	ExecuteOnPredictionProxies(this, [&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		// TODO: check if it is possible for ClientData.LastAckedMove to be nullptr. Cause that will lead to crashes inside prediction proxies
		PredictionProxy.ApplyCorrectedState(XMoveU_MoveResponse.Blackboard, XMoveU_LastAckedMove.IsValid() ? &XMoveU_LastAckedMove->Blackboard : nullptr, EXMoveU_CorrectionContext::PreRollback);
	});

	GetCharacterOwner()->bClientUpdating = true;
	CacheStateBeforeReplay();
	
	bool bReplayedMoves = Super::ClientUpdatePositionAfterServerUpdate();
	GetCharacterOwner()->bClientUpdating = false; // is set back to false in Super::ClientUpdatePositionAfterServerUpdate but early return might skip it.
	
	RestoreStateAfterReplay();
	MoveReplayFinishedDelegate.Broadcast();
	return bReplayedMoves;
}

// @XMoveU - @AfterUpdatingEngine: Here we are rewriting the super function split up in smaller more manageable functions.
// We need to check that implementation did not change, and in case update this one. 
void UXMoveU_PredictionMovementComponent::ServerMoveHandleClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	// @XMoveU - @CopiedFromSuper
	if (!ShouldUsePackedMovementRPCs())
	{
		if (RelativeClientLocation == FVector(1.f,2.f,3.f)) // first part of double servermove
		{
			return;
		}
	}

	FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character();
	check(ServerData);

	// Don't prevent more recent updates from being sent if received this frame.
	// We're going to send out an update anyway, might as well be the most recent one.
	APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController());
	if( (ServerData->LastUpdateTime != GetWorld()->TimeSeconds))
	{
		const AGameNetworkManager* GameNetworkManager = (const AGameNetworkManager*)(AGameNetworkManager::StaticClass()->GetDefaultObject());
		if (GameNetworkManager->WithinUpdateDelayBounds(PC, ServerData->LastUpdateTime))
		{
			return;
		}
	}

	// Offset may be relative to base component
	FVector ClientLoc = CalculateClientWorldLocation(RelativeClientLocation, ClientMovementBase, ClientBaseBoneName); // @XMoveU - @Change: Compacted into function

	FVector ServerLoc = UpdatedComponent->GetComponentLocation();

	// Client may send a null movement base when walking on bases with no relative location (to save bandwidth).
	// In this case don't check movement base in error conditions, use the server one (which avoids an error based on differing bases). Position will still be validated.
	TryUseServerBaseForClientBase(ClientMovementBase, ClientBaseBoneName, ClientMovementMode); // @XMoveU - @Change: Compacted into function

	// If base location is out of sync on server and client, changing base can result in a jarring correction.
	// So in the case that the base has just changed on server or client, server trusts the client (within a threshold)
	UPrimitiveComponent* MovementBase = CharacterOwner->GetMovementBase();
	FName MovementBaseBoneName = CharacterOwner->GetBasedMovement().BoneName;
	const bool bServerIsFalling = IsFalling();
	const bool bClientIsFalling = ClientMovementMode == MOVE_Falling;

	FVector RelativeLocation = ServerLoc;
	FVector RelativeVelocity = Velocity;
	bool bUseLastBase = false;
	bool bFallingWithinAcceptableError = false;

	// Potentially trust the client a little when landing
	if (ShouldDeferServerCorrectionsWhenFalling()) // @XMoveU - @Change: Compacted into function
	{
		// @XMoveU - @Change: Compacted into function (this implementation assumes server falling state is not changed before being called. Which does not in engine's code)
		bFallingWithinAcceptableError = IsFallingWithinAcceptableError(ClientLoc, ClientMovementMode, bUseLastBase, ServerLoc, MovementBase, RelativeLocation, RelativeVelocity);
	}

	// Compute the client error from the server's position
	// If client has accumulated a noticeable positional error, correct them.
	bNetworkLargeClientCorrection = ServerData->bForceClientUpdate;
	// @XMoveU - @Change: added ServerCheckGenericClientError.
	bool bGenericError = ServerCheckGenericClientError(ClientTimeStamp, DeltaTime, Accel, ClientLoc, RelativeClientLocation, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, bFallingWithinAcceptableError || bIgnoreClientMovementErrorChecksAndCorrection);
	if (ServerData->bForceClientUpdate || bGenericError || (!bFallingWithinAcceptableError && ServerCheckClientError(ClientTimeStamp, DeltaTime, Accel, ClientLoc, RelativeClientLocation, ClientMovementBase, ClientBaseBoneName, ClientMovementMode)))
	{
		// @XMoveU - @Change: Compacted into function
		ServerPrepareCorrection(ClientTimeStamp, DeltaTime, Accel, ClientLoc, ServerLoc, bUseLastBase, MovementBase, MovementBaseBoneName, RelativeLocation, RelativeVelocity);
	}
	else
	{
		if (ServerShouldUseAuthoritativePosition(ClientTimeStamp, DeltaTime, Accel, ClientLoc, RelativeClientLocation, ClientMovementBase, ClientBaseBoneName, ClientMovementMode))
		{
			// @XMoveU - @Change: Compacted into function
			ServerUseAuthoritativePosition(ClientTimeStamp, DeltaTime, Accel, ClientLoc, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
		}

		// acknowledge receipt of this successful servermove()
		ServerData->PendingAdjustment.TimeStamp = ClientTimeStamp;
		ServerData->PendingAdjustment.bAckGoodMove = true;
	}

#if USE_SERVER_PERF_COUNTERS
	PerfCountersIncrement(PerfCounter_NumServerMoves);
#endif

	ServerData->bForceClientUpdate = false;

	SetLastServerMovementBase(MovementBase);
	LastServerMovementBaseBoneName = MovementBaseBoneName;
	bLastClientIsFalling = bClientIsFalling;
	bLastServerIsFalling = bServerIsFalling;
	bLastServerIsWalking = MovementMode == MOVE_Walking;
	// ~@XMoveU - @CopiedFromSuper
}

/*--------------------------------------------------------------------------------------------------------------------*/
// ServerMoveHandleClientError Helpers

bool UXMoveU_PredictionMovementComponent::IsFallingWithinAcceptableError(const FVector& ClientWorldLocation, uint8 ClientMovementMode, bool& bUseLastBase, FVector& ServerWorldLocation, UPrimitiveComponent* ServerMovementBase, FVector& RelativeLocation, FVector& RelativeVelocity)
{
	const float ClientAuthorityThreshold = CharacterMovementCVars::Get_ClientAuthorityThresholdOnBaseChange();
	const float MaxFallingCorrectionLeash = CharacterMovementCVars::Get_MaxFallingCorrectionLeash();

	const bool bServerIsFalling = IsFalling();
	const bool bClientIsFalling = ClientMovementMode == MOVE_Falling;
	const bool bServerJustLanded = bLastServerIsFalling && !bServerIsFalling;
	const bool bClientJustLanded = bLastClientIsFalling && !bClientIsFalling;
	
	bool bFallingWithinAcceptableError = false;

	// @XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
	
	// Teleports and other movement modes mean we should just trust the server like we normally would
	if (bTeleportedSinceLastUpdate || (MovementMode != MOVE_Walking && MovementMode != MOVE_Falling))
	{
		MaxServerClientErrorWhileFalling = 0.f;
		bCanTrustClientOnLanding = false;
	}

	// MaxFallingCorrectionLeash indicates we'll use a variable correction size based on the error on take-off and the direction of movement.
	// ClientAuthorityThreshold is an static client-trusting correction upon landing.
	// If both are set, use the smaller of the two. If only one is set, use that. If neither are set, we wouldn't even be inside this block.
	float MaxLandingCorrection = 0.f;
	if (ClientAuthorityThreshold > 0.f && MaxFallingCorrectionLeash > 0.f)
	{
		MaxLandingCorrection = FMath::Min(ClientAuthorityThreshold, MaxServerClientErrorWhileFalling);
	}
	else
	{
		MaxLandingCorrection = FMath::Max(ClientAuthorityThreshold, MaxServerClientErrorWhileFalling);
	}

	if (bCanTrustClientOnLanding && MaxLandingCorrection > 0.f && (bClientJustLanded || bServerJustLanded))
	{
		// no longer falling; server should trust client up to a point to finish the landing as the client sees it
		const FVector LocDiff = ServerWorldLocation - ClientWorldLocation;

		if (!LocDiff.IsNearlyZero(UE_KINDA_SMALL_NUMBER))
		{
			if (LocDiff.SizeSquared() < FMath::Square(MaxLandingCorrection))
			{
				ServerWorldLocation = ClientWorldLocation;
				UpdatedComponent->MoveComponent(ServerWorldLocation - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bJustTeleported = true;
			}
			else
			{
				const FVector ClampedDiff = LocDiff.GetSafeNormal() * MaxLandingCorrection;
				ServerWorldLocation -= ClampedDiff;
				UpdatedComponent->MoveComponent(ServerWorldLocation - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bJustTeleported = true;
			}
		}

		MaxServerClientErrorWhileFalling = 0.f;
		bCanTrustClientOnLanding = false;
	}

	// Check for lift-off, going from walking to falling.
	// Note that if bStayBasedInAir is enabled we can't rely on the walking to falling transition, instead run logic on the first tick after clearing the MovementBase
	const bool bCanLiftOffFromBase = bStayBasedInAir
		? !ServerMovementBase && GetLastServerMovementBase() // If we keep the base while in air, consider lift-off if base gets set to null and we had a base last tick
		: bLastServerIsWalking; // If walking last tick, we were can consider lift-off logic

	if (bServerIsFalling && bCanLiftOffFromBase && !bTeleportedSinceLastUpdate)
	{
		float ClientForwardFactor = 1.f;
		UPrimitiveComponent* LastServerMovementBasePtr = GetLastServerMovementBase();
		if (IsValid(LastServerMovementBasePtr) && MovementBaseUtility::IsDynamicBase(LastServerMovementBasePtr) && MaxWalkSpeed > UE_KINDA_SMALL_NUMBER)
		{
			const FVector LastBaseVelocity = MovementBaseUtility::GetMovementBaseVelocity(LastServerMovementBasePtr, LastServerMovementBaseBoneName);
			RelativeVelocity = Velocity - LastBaseVelocity;
			const FVector BaseDirection = ProjectToGravityFloor(LastBaseVelocity).GetSafeNormal();
			const FVector RelativeDirection = RelativeVelocity * (1.f / MaxWalkSpeed);

			ClientForwardFactor = FMath::Clamp(FVector::DotProduct(BaseDirection, RelativeDirection), 0.f, 1.f);

			// To improve position syncing, use old base for take-off
			if (MovementBaseUtility::UseRelativeLocation(LastServerMovementBasePtr))
			{
				// Relative Location
				MovementBaseUtility::TransformLocationToLocal(LastServerMovementBasePtr, LastServerMovementBaseBoneName, UpdatedComponent->GetComponentLocation(), RelativeLocation);
				bUseLastBase = true;
			}
		}

		if (ClientAuthorityThreshold > 0.f && ClientForwardFactor < 1.f)
		{
			const float AdjustedClientAuthorityThreshold = ClientAuthorityThreshold * (1.f - ClientForwardFactor);
			const FVector LocDiff = ServerWorldLocation - ClientWorldLocation;

			// Potentially trust the client a little when taking off in the opposite direction to the base (to help not get corrected back onto the base)
			if (!LocDiff.IsNearlyZero(UE_KINDA_SMALL_NUMBER))
			{
				if (LocDiff.SizeSquared() < FMath::Square(AdjustedClientAuthorityThreshold))
				{
					ServerWorldLocation = ClientWorldLocation;
					UpdatedComponent->MoveComponent(ServerWorldLocation - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
					bJustTeleported = true;
				}
				else
				{
					const FVector ClampedDiff = LocDiff.GetSafeNormal() * AdjustedClientAuthorityThreshold;
					ServerWorldLocation -= ClampedDiff;
					UpdatedComponent->MoveComponent(ServerWorldLocation - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
					bJustTeleported = true;
				}
			}
		}

		if (ClientForwardFactor < 1.f)
		{
			MaxServerClientErrorWhileFalling = FMath::Min((ServerWorldLocation - ClientWorldLocation).Size() * (1.f - ClientForwardFactor), MaxFallingCorrectionLeash);
			bCanTrustClientOnLanding = true;
		}
		else
		{
			MaxServerClientErrorWhileFalling = 0.f;
			bCanTrustClientOnLanding = false;
		}
	}
	else if (!bServerIsFalling && bCanTrustClientOnLanding)
	{
		MaxServerClientErrorWhileFalling = 0.f;
		bCanTrustClientOnLanding = false;
	}

	if (MaxServerClientErrorWhileFalling > 0.f && (bServerIsFalling || bClientIsFalling))
	{
		const FVector LocDiff = ServerWorldLocation - ClientWorldLocation;
		if (LocDiff.SizeSquared() <= FMath::Square(MaxServerClientErrorWhileFalling))
		{
			ServerWorldLocation = ClientWorldLocation;
			// Still want a velocity update when we first take off
			bFallingWithinAcceptableError = true;
		}
		else
		{
			// Change ServerWorldLocation to be on the edge of the acceptable error rather than doing a full correction.
			// This is not actually changing the server position, but changing it as far as corrections are concerned.
			// This means we're just holding the client on a longer leash while we're falling.
			ServerWorldLocation = ServerWorldLocation - LocDiff.GetSafeNormal() * FMath::Clamp(MaxServerClientErrorWhileFalling - CharacterMovementCVars::Get_MaxFallingCorrectionLeashBuffer(), 0.f, MaxServerClientErrorWhileFalling);
		}
	}
	// ~@XMoveU - @CopiedFromSuper::ServerMoveHandleClientError

	return bFallingWithinAcceptableError;
}

void UXMoveU_PredictionMovementComponent::ServerPrepareCorrection(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& ServerWorldLocation, bool bUseLastBase, UPrimitiveComponent* ServerMovementBase, FName ServerBaseBoneName, const FVector& RelativeLocation, const FVector& RelativeVelocity)
{
	FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character();
	bool bDeferServerCorrectionsWhenFalling = ShouldDeferServerCorrectionsWhenFalling();
	
	// @XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
	ServerData->PendingAdjustment.NewVel = Velocity;
	ServerData->PendingAdjustment.NewBase = ServerMovementBase;
	ServerData->PendingAdjustment.NewBaseBoneName = ServerBaseBoneName;
	ServerData->PendingAdjustment.NewLoc = FRepMovement::RebaseOntoZeroOrigin(ServerWorldLocation, this);
	ServerData->PendingAdjustment.NewRot = UpdatedComponent->GetComponentRotation();
	ServerData->PendingAdjustment.GravityDirection = GetGravityDirection();

	ServerData->PendingAdjustment.bBaseRelativePosition = (bDeferServerCorrectionsWhenFalling && bUseLastBase) || MovementBaseUtility::UseRelativeLocation(ServerMovementBase);
	ServerData->PendingAdjustment.bBaseRelativeVelocity = false;
	
	// Relative location?
	if (ServerData->PendingAdjustment.bBaseRelativePosition)
	{
		if (bDeferServerCorrectionsWhenFalling && bUseLastBase)
		{
			ServerData->PendingAdjustment.NewVel = RelativeVelocity;
			ServerData->PendingAdjustment.NewBase = GetLastServerMovementBase();
			ServerData->PendingAdjustment.NewBaseBoneName = LastServerMovementBaseBoneName;
			ServerData->PendingAdjustment.NewLoc = RelativeLocation;
		}
		else
		{
			ServerData->PendingAdjustment.NewLoc = CharacterOwner->GetBasedMovement().Location;
			if (CharacterMovementCVars::Get_NetUseBaseRelativeVelocity())
			{
				// Store world velocity converted to local space of movement base
				ServerData->PendingAdjustment.bBaseRelativeVelocity = true;
				const FVector CurrentVelocity = ServerData->PendingAdjustment.NewVel;
				MovementBaseUtility::TransformDirectionToLocal(ServerMovementBase, ServerBaseBoneName, CurrentVelocity, ServerData->PendingAdjustment.NewVel);
			}
		}
		
		// TODO: this could be a relative rotation, but all client corrections ignore rotation right now except the root motion one, which would need to be updated.
		//ServerData->PendingAdjustment.NewRot = CharacterOwner->GetBasedMovement().Rotation;
	}


#if !UE_BUILD_SHIPPING
	if (CharacterMovementCVars::Get_NetShowCorrections() != 0)
	{
		const FVector LocDiff = UpdatedComponent->GetComponentLocation() - ClientWorldLocation;
		const FString BaseString = ServerMovementBase ? ServerMovementBase->GetPathName(ServerMovementBase->GetOutermost()) : TEXT("None");
		UE_LOG(LogNetPlayerMovement, Warning, TEXT("*** Server: Error for %s at Time=%.3f is %3.3f LocDiff(%s) ClientLoc(%s) ServerLoc(%s) Base: %s Bone: %s Accel(%s) Velocity(%s)"),
			*GetNameSafe(CharacterOwner), ClientTimeStamp, LocDiff.Size(), *LocDiff.ToString(), *ClientWorldLocation.ToString(), *UpdatedComponent->GetComponentLocation().ToString(), *BaseString, *ServerData->PendingAdjustment.NewBaseBoneName.ToString(), *Accel.ToString(), *Velocity.ToString());
		const float DebugLifetime = CharacterMovementCVars::Get_NetCorrectionLifetime();
		DrawDebugCapsule(GetWorld(), UpdatedComponent->GetComponentLocation()	, CharacterOwner->GetSimpleCollisionHalfHeight(), CharacterOwner->GetSimpleCollisionRadius(), FQuat::Identity, FColor(100, 255, 100), false, DebugLifetime);
		DrawDebugCapsule(GetWorld(), ClientWorldLocation							, CharacterOwner->GetSimpleCollisionHalfHeight(), CharacterOwner->GetSimpleCollisionRadius(), FQuat::Identity, FColor(255, 100, 100), false, DebugLifetime);
	}
#endif

	ServerData->LastUpdateTime = GetWorld()->TimeSeconds;
	ServerData->PendingAdjustment.DeltaTime = DeltaTime;
	ServerData->PendingAdjustment.TimeStamp = ClientTimeStamp;
	ServerData->PendingAdjustment.bAckGoodMove = false;
	ServerData->PendingAdjustment.MovementMode = PackNetworkMovementMode();

#if USE_SERVER_PERF_COUNTERS
	PerfCountersIncrement(PerfCounter_NumServerMoveCorrections);
#endif
	// ~@XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
}

void UXMoveU_PredictionMovementComponent::ServerUseAuthoritativePosition(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	// @XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
	const FVector LocDiff = UpdatedComponent->GetComponentLocation() - ClientWorldLocation; //-V595
	if (!LocDiff.IsZero() || ClientMovementMode != PackNetworkMovementMode() || GetMovementBase() != ClientMovementBase || (CharacterOwner && CharacterOwner->GetBasedMovement().BoneName != ClientBaseBoneName))
	{
		// Just set the position. On subsequent moves we will resolve initially overlapping conditions.
		UpdatedComponent->SetWorldLocation(ClientWorldLocation, false); //-V595

		// Trust the client's movement mode.
		ApplyNetworkMovementMode(ClientMovementMode);

		// Update base and floor at new location.
		SetBase(ClientMovementBase, ClientBaseBoneName);
		UpdateFloorFromAdjustment();

		// Even if base has not changed, we need to recompute the relative offsets (since we've moved).
		SaveBaseLocation();

		LastUpdateLocation = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
		LastUpdateRotation = UpdatedComponent ? UpdatedComponent->GetComponentQuat() : FQuat::Identity;
		LastUpdateVelocity = Velocity;
	}
	// ~@XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
}

bool UXMoveU_PredictionMovementComponent::ShouldDeferServerCorrectionsWhenFalling()
{
	// @XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
	const float ClientAuthorityThreshold = CharacterMovementCVars::Get_ClientAuthorityThresholdOnBaseChange();
	const float MaxFallingCorrectionLeash = CharacterMovementCVars::Get_MaxFallingCorrectionLeash();
	const bool bDeferServerCorrectionsWhenFalling = ClientAuthorityThreshold > 0.f || MaxFallingCorrectionLeash > 0.f;
	// ~@XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
	
	return bDeferServerCorrectionsWhenFalling;
}

FVector UXMoveU_PredictionMovementComponent::CalculateClientWorldLocation(const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName)
{
	// @XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
	FVector ClientLoc = RelativeClientLocation;
	if (MovementBaseUtility::UseRelativeLocation(ClientMovementBase))
	{
		MovementBaseUtility::TransformLocationToWorld(ClientMovementBase, ClientBaseBoneName, RelativeClientLocation, ClientLoc);
	}
	else
	{
		ClientLoc = FRepMovement::RebaseOntoLocalOrigin(ClientLoc, this);
	}
	// ~@XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
	
	return ClientLoc;
}

void UXMoveU_PredictionMovementComponent::TryUseServerBaseForClientBase(UPrimitiveComponent*& ClientMovementBase, FName& ClientBaseBoneName, uint8 ClientMovementMode)
{
	// @XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
	if (ClientMovementBase == nullptr)
	{
		TEnumAsByte<EMovementMode> NetMovementMode(MOVE_None);
		TEnumAsByte<EMovementMode> NetGroundMode(MOVE_None);
		uint8 NetCustomMode(0);
		UnpackNetworkMovementMode(ClientMovementMode, NetMovementMode, NetCustomMode, NetGroundMode);
		if (NetMovementMode == MOVE_Walking)
		{
			ClientMovementBase = CharacterOwner->GetBasedMovement().MovementBase;
			ClientBaseBoneName = CharacterOwner->GetBasedMovement().BoneName;
		}
	}
	// ~@XMoveU - @CopiedFromSuper::ServerMoveHandleClientError
}

// ~ServerMoveHandleClientError Helpers
/*--------------------------------------------------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXMoveU_PredictionMovementComponent
 */

void UXMoveU_PredictionMovementComponent::ServerGetClientInputs(const FCharacterNetworkMoveData& MoveData)
{
	const FXMoveU_CharacterNetworkMoveData& XMoveU_ClientNetworkData = static_cast<const FXMoveU_CharacterNetworkMoveData&>(MoveData);

	// Call UXMoveU_PredictionManager::ServerGetClientInputs
	ExecuteOnPredictionManagers(this, [&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->ServerGetClientInputs(this, XMoveU_ClientNetworkData);
	});
	
	// Call FXMoveU_PredictionProxy::ApplyClientInputs
	ExecuteOnPredictionProxies(this, [&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.ApplyClientInputs(XMoveU_ClientNetworkData.Blackboard);
	});
}

void UXMoveU_PredictionMovementComponent::CacheStateBeforeReplay()
{
	// Call UXMoveU_PredictionManager::CacheStateBeforeReplay
	ExecuteOnPredictionManagers(this, [&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->CacheStateBeforeReplay(this);
	});
	
	// Call FXMoveU_PredictionProxy::CacheStatePreRollback
	ExecuteOnPredictionProxies(this, [](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.CacheStatePreRollback();
	});
}

void UXMoveU_PredictionMovementComponent::RestoreStateAfterReplay()
{
	// Call UXMoveU_PredictionManager::RestoreStateAfterReplay
	ExecuteOnPredictionManagers(this, [&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		PredictionManager->RestoreStateAfterReplay(this);
	});
	
	// Call FXMoveU_PredictionProxy::RestoreStatePostRollback
	ExecuteOnPredictionProxies(this, [](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		PredictionProxy.RestoreStatePostRollback();
	});
}

bool UXMoveU_PredictionMovementComponent::ServerCheckGenericClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode, bool bIgnorePositionErrors)
{
	bool bIsError = false;
	
	// Call UXMoveU_PredictionManager::ServerCheckClientError
	ExecuteOnPredictionManagers(this, [&](UXMoveU_PredictionManager* PredictionManager, bool& bOutStopExecution)
	{
		bIsError |= PredictionManager->ServerCheckClientError(this, ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, RelativeClientLocation, ClientMovementBase, ClientBaseBoneName, ClientMovementMode, bIgnorePositionErrors);
	});

	// Call FXMoveU_PredictionProxy::HasPredictionError
	const FXMoveU_CharacterNetworkMoveData& XMoveU_ClientNetworkData = *static_cast<const FXMoveU_CharacterNetworkMoveData*>(GetCurrentNetworkMoveData());
	ExecuteOnPredictionProxies(this, [&](FXMoveU_PredictionProxy& PredictionProxy, bool& bOutStopExecution)
	{
		// Always call the function to update the error state of the proxy.
		bool bPredictionError = PredictionProxy.HasPredictionError(XMoveU_ClientNetworkData.Blackboard);
		// Ignore error if we need to.
		bool bIgnoreError = PredictionProxy.IsPositionRelated() && bIgnorePositionErrors;
		bIsError |= (bPredictionError && !bIgnoreError);
	});
	
	return bIsError;
}

/*====================================================================================================================*/
// CustomPrediction

void UXMoveU_PredictionMovementComponent::ExecuteOnPredictionManagers(const UCharacterMovementComponent* MoveComp, TFunctionRef<void(UXMoveU_PredictionManager*, bool&)> InFunction)
{
	if (!IsValid(MoveComp))
	{
		return;
	}

	const UXMoveU_PredictionMovementComponent* PredictionMoveComp = Cast<UXMoveU_PredictionMovementComponent>(MoveComp);
	if (!PredictionMoveComp)
	{
		return;
	}
	
	bool bStopExecution = false;
	TArray<UXMoveU_PredictionManager*> PredictionManagers;
	PredictionMoveComp->GetPredictionManagers(PredictionManagers);
	for (UXMoveU_PredictionManager* PredictionManager : PredictionManagers)
	{
		if (IsValid(PredictionManager))
		{
			InFunction(PredictionManager, bStopExecution);
			if (bStopExecution)
			{
				break;
			}
		}
	}
}

void UXMoveU_PredictionMovementComponent::ExecuteOnPredictionProxies(const UCharacterMovementComponent* MoveComp, TFunctionRef<void(FXMoveU_PredictionProxy&, bool&)> InFunction)
{
	if (!IsValid(MoveComp))
	{
		return;
	}

	const UXMoveU_PredictionMovementComponent* PredictionMoveComp = Cast<UXMoveU_PredictionMovementComponent>(MoveComp);
	if (!PredictionMoveComp)
	{
		return;
	}
	
	bool bStopExecution = false;
	TArray<UXMoveU_PredictionManager*> PredictionManagers;
	PredictionMoveComp->GetPredictionManagers(PredictionManagers);
	for (UXMoveU_PredictionManager* PredictionManager : PredictionManagers)
	{
		if (IsValid(PredictionManager))
		{
			PredictionManager->ForEachProxy(InFunction, bStopExecution);
			if (bStopExecution)
			{
				break;
			}
		}
	}
}

void UXMoveU_PredictionMovementComponent::RegisterPredictionManager(UXMoveU_PredictionManager* NewPredictionManager)
{
	if (!IsValid(NewPredictionManager))
	{
		UE_LOG(LogXyloMovementUtil, Warning, TEXT("UXMoveU_PredictionMovementComponent::RegisterPredictionManager >> NewPredictionManager is not valid"))
		return;
	}

	PredictionManagers.Add(NewPredictionManager);
	NewPredictionManager->OnRegistered(this);
}

void UXMoveU_PredictionMovementComponent::GetPredictionManagers(TArray<UXMoveU_PredictionManager*>& OutPredictionManagers) const
{
	OutPredictionManagers.Add(DefaultPredictionManager);
	OutPredictionManagers.Append(PredictionManagers);
}

// ~CustomPrediction
/*====================================================================================================================*/

/*====================================================================================================================*/
// CustomAccessors

void UXMoveU_PredictionMovementComponent::SetLastServerMovementBase(UPrimitiveComponent* InMovementBase)
{
	(*this).*GLastServerMovementBase = InMovementBase;
}

// ~CustomAccessors
/*====================================================================================================================*/
