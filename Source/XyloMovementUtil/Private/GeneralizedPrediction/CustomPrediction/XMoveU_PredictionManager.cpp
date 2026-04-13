// Copyright (c) 2026, XyloIsCoding. All rights reserved.


#include "GeneralizedPrediction/CustomPrediction/XMoveU_PredictionManager.h"

#include "GameFramework/CharacterMovementComponent.h"

UXMoveU_PredictionManager::UXMoveU_PredictionManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/*====================================================================================================================*/
// PredictionProxies

void UXMoveU_PredictionManager::RegisterPredictionProxy(const TSharedPtr<FXMoveU_PredictionProxy>& NewProxy)
{
	PredictionProxies.Add(NewProxy);
}

void UXMoveU_PredictionManager::ForEachProxy(TFunctionRef<void(FXMoveU_PredictionProxy& Proxy, bool& bStopExecution)> InFunction, bool& bOutExecutionStopped)
{
	bOutExecutionStopped = false;

	// Loop between all prediction proxies and execute the function passing them as param
	bool bStopExecution = false;
	for (TSharedPtr<FXMoveU_PredictionProxy>& Proxy : PredictionProxies)
	{
		if (FXMoveU_PredictionProxy* PredictionProxy = Proxy.Get())
		{
			InFunction(*PredictionProxy, bStopExecution);

			// Stop loop if explicitly requested
			if (bStopExecution)
			{
				bOutExecutionStopped = true;
				break;
			}
		}
	}
}

// ~PredictionProxies
/*====================================================================================================================*/

