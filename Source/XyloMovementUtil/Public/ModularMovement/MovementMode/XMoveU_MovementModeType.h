// Copyright (c) 2026, XyloIsCoding. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "XMoveU_MovementModeType.generated.h"

/**
 * 
 */
UENUM()
enum class EXMoveU_MovementModeType : uint8
{
	None,
	Ground,
	Falling,
	Flying,
	Swimming,
};
