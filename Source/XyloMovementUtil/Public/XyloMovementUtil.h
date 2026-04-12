// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

XYLOMOVEMENTUTIL_API DECLARE_LOG_CATEGORY_EXTERN(LogXyloMovementUtil, Log, All);

class FXyloMovementUtilModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
