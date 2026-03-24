// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "XMoveU_SimplePredictionProxy.h"
#include "GeneralizedPrediction/CustomPrediction/Helpers/XMoveU_BlackboardHelper.h"
#include "GeneralizedPrediction/CustomPrediction/Helpers/XMoveU_ProxyVar.h"


namespace XMoveU
{
	/**
	 * Template class to help building a PredictionProxy for a type or a specific variable.
	 */
	template <typename T, template <typename> typename Traits>
	struct TSimplePredictionProxy : public FXMoveU_SimplePredictionProxy
	{
		using FTraits = Traits<T>;
		using ParamType = typename FTraits::ParamType;
		using FProxyVarType = TProxyVarInterface<T, Traits>;
		
		TSimplePredictionProxy() {}
		virtual ~TSimplePredictionProxy() {}

	public:
		void SetName(FName ProxyVarName)
		{
			BlackboardHelper.VarName = ProxyVarName;
			BlackboardHelperPostSim.VarName = FName("END_" + ProxyVarName.ToString());
		}

		void SetProxyVariable(TSharedPtr<FProxyVarType> InProxyVar)
		{
			ProxyVariable = InProxyVar;
		}

		template <typename ProxyType> requires TIsDerivedFrom<ProxyType, FProxyVarType>::Value
		void SetProxyVariable(ProxyType&& InProxyVar)
		{
			ProxyVariable = MakeShared<ProxyType>(InProxyVar);
		}
		
	protected:
		/** Helper struct to access the variable tracked by this proxy. */
		TSharedPtr<FProxyVarType> ProxyVariable;
		/** Only valid on client during rollback ONLY IF bRestoreAfterRollback */
		T CachedValue;
	private:
		/** Helper to access the value on blackboard objects. */
		TBlackboardHelper<T> BlackboardHelper;
		/** Helper to access the PostSim value on blackboard objects (ONLY USED for PredictionFrame!!!). */
		TBlackboardHelper<T> BlackboardHelperPostSim;
		/** Only updated in HasPredictionError on server. Client receives it in
		 * SerializeCorrectedStates ONLY UNDER SPECIFIC CONDITIONS. */
		bool bHasPredictionErrorThisFrame = false;

		
	/*----------------------------------------------------------------------------------------------------------------*/
		// Interface
	
	protected:
		/** Provide the default value for this type. */
		virtual T MakeDefaulted() = 0;

		/** Serializes the value sent to server. */
		virtual bool SerializeInputsAndCorrectionStates_Internal(T& Value, FArchive& Ar, UPackageMap* PackageMap) = 0;

		/** Checks the client value for discrepancies with the server value. */
		virtual bool HasPredictionError_Internal(ParamType ClientPredictedValue) = 0;
		/** Serializes the value to send to client as the correct authoritative value. */
		virtual bool SerializeCorrectedStates_Internal(T& Value, FArchive& Ar, UPackageMap* PackageMap) = 0;

		/** Respond to an authoritative correction being received. Can prevent the proxy from automatically applying the correction value */
		virtual void OnCorrectionReceived(ParamType AuthoritativeValue, ParamType PredictedValue, EXMoveU_CorrectionContext Context, bool& bOutApplyCorrection) {}
		
		/** Called when the proxy is asked to roll back the value to a specific frame.
		 * The proxy only rollbacks the value if CorrectionMode == EXMoveU_CorrectionMode::None, so this can be used
		 * to implement extra logic. */
		virtual void OnFrameRollback(ParamType RollbackValue) {}
		/** Called after a rollback before applying back the pre-rollback value.  */
		virtual void OnRestoreStatePostRollback(bool& bOutRestoreFromCache) {}
		
		/** Checks if the value at the beginning of the simulation frame already prevents combining by itself.
		 * An Engine example is having RootMotion or PendingLaunchVelocity. */
		virtual bool IsFrameNonCombinablePreSim(ParamType Value) { return false; }
		/** Checks if the changes in the value during the simulation made this frame not combinable, or if we want
		 * this frame to be immediately sent to server instead of waiting next frame to check if it can be combined. */
		virtual bool IsFrameNonCombinablePostSim(ParamType PreSimValue, ParamType PostSimValue) { return false; }
		/** Checks if the value at the start of the old frame and the next one allow combining the frames. */
		virtual bool CanCombineWithNewFrame_Internal(ParamType OldFrameValue, ParamType NewFrameValue) { return true; }
		/** Checks if the value changed outside the simulation, which then makes this frame not able to be combined. */
		virtual bool HasNonSimulatedChange(ParamType LastPostSimValue, ParamType NewPreSimValue) { return false; }

		/** Checks if this frame has any important changes relative to the last frame that got acknowledge by the server. */
		virtual bool IsImportantFrame_Internal(ParamType PreSimValue, ParamType PostSimValue, ParamType LastAckedPreSimValue, ParamType LastAckedPostSimValue) { return false; }
		
		// ~Interface
	/*----------------------------------------------------------------------------------------------------------------*/

		
	private:
		// PredictionFrame uses both BlackboardHelper and BlackboardHelperPostSim.
		// NetworkFrame and AuthoritativeFrame always use BlackboardHelper to store whatever is passed in it.
		
		virtual void CollectInputsAndCorrectionStates(const FBlackboard& CurrentPredictionFrame, FBlackboard& NetworkFrame) override final
		{
			if (bIsInput)
			{
				// If this var is an input, then we need to pass the PreSim value to the NetworkFrame, so the server can use it.
				BlackboardHelper.Transfer(CurrentPredictionFrame, NetworkFrame);

				// TODO: some stuff is an input but generated during the client simulation (eg grapple point) so we need the PostSim value
				return;
			}

			if (bCheckForError)
			{
				// If this var is to be checked for error, we need to send the value we got PostSim, this way the server
				// Can compare it to the value it has at the end of its simulation.
				BlackboardHelper.Set(NetworkFrame, BlackboardHelperPostSim.Get(CurrentPredictionFrame));
				return;
			}
		}
		
		virtual bool SerializeInputsAndCorrectionStates(FBlackboard& NetworkFrame, FArchive& Ar, UPackageMap* PackageMap) override final
		{
			// Only serialize if we actually collected data in CollectInputsAndCorrectionStates.
			if (!bIsInput && !bCheckForError) { return true; }

			// When loading we need to make sure we have some valid memory to deserialize the value on.
			if (Ar.IsLoading())
			{
				BlackboardHelper.Set(NetworkFrame, MakeDefaulted());
			}
			// Delegate serialization to subclass.
			return SerializeInputsAndCorrectionStates_Internal(BlackboardHelper.Get(NetworkFrame), Ar, PackageMap);
		}
		
		virtual void ApplyClientInputs(const FBlackboard& ClientNetworkFrame) override final
		{
			if (bIsInput)
			{
				// If this is an input, apply it to server.
				ProxyVariable->Set(BlackboardHelper.Get(ClientNetworkFrame));
			}
		}
		
		virtual bool HasPredictionError(const FBlackboard& ClientNetworkFrame) override final
		{
			if (!bCheckForError) { return false; }

			// If we need to check the client data for error, then delegate the check to subclass.
			bHasPredictionErrorThisFrame = HasPredictionError_Internal(BlackboardHelper.Get(ClientNetworkFrame));
			return bHasPredictionErrorThisFrame;
		}
		
		virtual void CollectCorrectedStates(FBlackboard& AuthoritativeFrame) override final
		{
			if (CorrectionMode == EXMoveU_CorrectionMode::Authoritative)
			{
				// Collect this value if it needs to be sent as correction.
				BlackboardHelper.Set(AuthoritativeFrame, ProxyVariable->Get());
			}
		}
		
		virtual bool SerializeCorrectedStates(FBlackboard& AuthoritativeFrame, FArchive& Ar, UPackageMap* PackageMap) override final
		{
			// Only serialize if we actually collected data in CollectCorrectedStates.
			if (CorrectionMode != EXMoveU_CorrectionMode::Authoritative) { return true; }

			// We only serialize error state if necessary (error is checked, and we actually need to pass the state).
			bool bSerializeErrorState = bCheckForError && bUseLocalCorrectionIfNoError;
			if (bSerializeErrorState)
			{
				Ar.SerializeBits(&bHasPredictionErrorThisFrame, 1);
			}

			// If we serialized the error state, and there was no error, we are done here, since local correction
			// is going to be applied instead of Auth value.
			if (bSerializeErrorState && !bHasPredictionErrorThisFrame)
			{
				return true;
			}
			
			// When loading we need to make sure we have some valid memory to deserialize the value on.
			if (Ar.IsLoading())
			{
				BlackboardHelper.Set(AuthoritativeFrame, MakeDefaulted());
			}
			// Delegate serialization to subclass.
			return SerializeCorrectedStates_Internal(BlackboardHelper.Get(AuthoritativeFrame), Ar, PackageMap);
		}
		
		virtual void ApplyCorrectedState(const FBlackboard& AuthoritativeFrame, const FBlackboard* TargetPredictionFrame, EXMoveU_CorrectionContext Context) override final
		{
			bool bWantsAuthCorrection = CorrectionMode == EXMoveU_CorrectionMode::Authoritative;
			bool bHasValidErrorState = bCheckForError && bUseLocalCorrectionIfNoError;
			bool bForceLocalCorrection = bWantsAuthCorrection && bHasValidErrorState && !bHasPredictionErrorThisFrame;
			
			if ((bWantsAuthCorrection && !bForceLocalCorrection) && Context == AuthCorrectionContext)
			{
				// If a correction value was sent from server, lets ask the subclass if we want to apply it, while
				// also giving the opportunity to perform some cleanup, and check if the corrected value was actually
				// different from the predicted one.
				bool bApplyCorrection = true;
				OnCorrectionReceived(BlackboardHelper.Get(AuthoritativeFrame), BlackboardHelperPostSim.Get(*TargetPredictionFrame), Context, bApplyCorrection);
				if (!bApplyCorrection) { return; }

				// Apply the correction if the subclass approved.
				ProxyVariable->Set(BlackboardHelper.Get(AuthoritativeFrame));
				return;
			}

			bool bWantsLocalCorrection = CorrectionMode == EXMoveU_CorrectionMode::Local;
			if ((bWantsLocalCorrection || bForceLocalCorrection) && Context == EXMoveU_CorrectionContext::PreRollback)
			{
				// Sometimes we want the rollback to start with the value we had locally at the frame that got corrected.
				// In this case apply the PostSim value.
				ProxyVariable->Set(BlackboardHelperPostSim.Get(*TargetPredictionFrame));
				return;
			}
		}

		virtual void Reset(FBlackboard& PredictionFrame) override final
		{
			// Set the default values for this frame so it can be reused
			BlackboardHelper.Set(PredictionFrame, MakeDefaulted());
			BlackboardHelperPostSim.Set(PredictionFrame, MakeDefaulted());
		}
		
		virtual void CollectCurrentState(FBlackboard& PredictionFrame) override final
		{
			if (!bAffectedBySimulation)
			{
				// If the value is not affected by the simulation, for example an external "setting", we collect it here.
				BlackboardHelper.Set(PredictionFrame, ProxyVariable->Get());
			}
		}
		
		virtual void CollectDynamicState(FBlackboard& PredictionFrame) override final
		{
			if (bAffectedBySimulation)
			{
				// If the value is affected by the simulation, for example "stamina", we collect it here.
				BlackboardHelper.Set(PredictionFrame, ProxyVariable->Get());
			}
		}
		
		virtual bool BlockCombinePreSimulation(FBlackboard& PredictionFrame) override final
		{
			return IsFrameNonCombinablePreSim(BlackboardHelper.Get(PredictionFrame));
		}
		
		virtual void CollectFinalState(FBlackboard& PredictionFrame, FSavedMove_Character::EPostUpdateMode PostUpdateMode) override final
		{
			// Collect the PostSim value.
			BlackboardHelperPostSim.Set(PredictionFrame, ProxyVariable->Get());
		}
		
		virtual bool BlockCombinePostSimulation(FBlackboard& PredictionFrame) override final
		{
			return IsFrameNonCombinablePostSim(BlackboardHelper.Get(PredictionFrame), BlackboardHelperPostSim.Get(PredictionFrame));
		}

		virtual bool IsImportantFrame(const FBlackboard& PredictionFrame, const FBlackboard& LastAckedPredictionFrame) override final
		{
			return IsImportantFrame_Internal(BlackboardHelper.Get(PredictionFrame), BlackboardHelperPostSim.Get(PredictionFrame), BlackboardHelper.Get(LastAckedPredictionFrame), BlackboardHelperPostSim.Get(LastAckedPredictionFrame));
		}
		
		virtual bool CanCombineWithNewFrame(const FBlackboard& PredictionFrame, const FBlackboard& NewPredictionFrame) override final
		{
			bool bCanCombineStartWithStart = CanCombineWithNewFrame_Internal(BlackboardHelper.Get(PredictionFrame), BlackboardHelper.Get(NewPredictionFrame));
			bool bHasNonSimulatedChanges = HasNonSimulatedChange(BlackboardHelperPostSim.Get(PredictionFrame), BlackboardHelper.Get(NewPredictionFrame));
			return bCanCombineStartWithStart && !bHasNonSimulatedChanges;
		}
		
		virtual void RevertFrameChanges(const FBlackboard& OldPredictionFrame) override final
		{
			if (bAffectedBySimulation)
			{
				// If the value is affected by the simulation, we revert its changes.
				ProxyVariable->Set(BlackboardHelper.Get(OldPredictionFrame));
			}
		}

		virtual void CacheStatePreRollback() override final
		{
			if (bRestoreAfterRollback)
			{
				// Cache value before performing rollback and resimulation.
				CachedValue = ProxyVariable->Get();
			}
		}
		
		virtual void RollbackToFrame(const FBlackboard& PredictionFrame) override final
		{
			// Give the opportunity to subclass to perform some operations before we roll back the value.
			OnFrameRollback(BlackboardHelper.Get(PredictionFrame));

			// We only need to rollback if CorrectionMode is None, otherwise we would be overwriting the correction.
			if (CorrectionMode == EXMoveU_CorrectionMode::None)
			{
				// TODO: some times we might need to have available a value that was generated during the simulation to resimulate the frame properly (eg grapple point).
				ProxyVariable->Set(BlackboardHelper.Get(PredictionFrame));
			}
		}
		
		virtual void RestoreStatePostRollback() override final
		{
			if (bRestoreAfterRollback)
			{
				// If we need to restore the value that the client had before the rollback, we first give the subclass
				// an opportunity to respond to changes.
				bool bRestoreFromCache = true;
				OnRestoreStatePostRollback(bRestoreFromCache);
				if (!bRestoreFromCache) { return; }

				// Apply the cahced value if the subclass approved.
				ProxyVariable->Set(CachedValue);
			}
		}
	};
}



