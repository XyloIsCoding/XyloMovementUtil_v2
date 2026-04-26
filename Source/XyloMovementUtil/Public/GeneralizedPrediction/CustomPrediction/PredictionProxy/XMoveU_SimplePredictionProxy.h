// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "XMoveU_PredictionProxy.h"
#include "XMoveU_SimplePredictionProxy.generated.h"


UENUM()
enum class EXMoveU_CorrectionMode : uint8
{
	// The value is only changed by simulation logic during rollbacks, with no external interference from rollback system.
	None,
	// For each resimulated frame of a rollback, the value is reapplied as it was during the resimulated prediction frame.
	Replay,
	// The correction packet contains the authoritative value, which is going to be applied to the client.
	Authoritative,
	// Apply the value the client had at the end of the frame that received the correction.
	Local,
};

/**
 * Non template base class to be able to have template child classes that can be used as USTRUCT.
 * Does not provide any functionality over FXMoveU_PredictionProxy.
 *	@code
 *	USTRUCT(BlueprintType)
 *	struct FXMoveU_PredictionProxy_MyType :
 *	#if CPP
 *		public XMoveU::TSimplePredictionProxy<float, XMoveU::ProxyVar::Traits::ByValue>
 *	#else
 *		public FXMoveU_SimplePredictionProxy
 *	#endif
 *	{
 *		GENERATED_BODY()
 *		...
 *	}
 *	@endcode
 */
USTRUCT()
struct XYLOMOVEMENTUTIL_API FXMoveU_SimplePredictionProxy : public FXMoveU_PredictionProxy
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInput = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCheckForError = false;

	/** How this value should be treated during server corrections. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EXMoveU_CorrectionMode CorrectionMode = EXMoveU_CorrectionMode::Replay;

	/** When the correction from server is applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "CorrectionMode == EXMoveU_CorrectionMode::Authoritative", EditConditionHides))
	EXMoveU_CorrectionContext AuthCorrectionContext = EXMoveU_CorrectionContext::OnReceive;

	/** This has pros and cons. The advantage is that if no error is found the value is not going be sent back to
	 *  client, which will spare some bandwidth. Once the correction is received the client is going to use the value 
	 *  it had at the end of the corrected move. The disadvantage is that no matter what an extra single bit is getting
	 *  serialized to tell the client if the correction value was serialized or not. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bCheckForError", EditConditionHides))
    bool bUseLocalCorrectionIfNoError = false;

	/** Has to be set to true if this value changes during the simulation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectedBySimulation = false;

	/** If true, the pre-rollback value if restored after the rollback is done. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRestoreAfterRollback = false;

	/** Error in position related variables can be ignored while transitioning from falling to walking, in order to
	 * prevent corrections when transitioning to and from moving platforms. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPositionRelated = false;

public:
	virtual bool IsPositionRelated() const override { return bIsPositionRelated; }
};

