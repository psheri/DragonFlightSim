

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class DRAGONFLIGHTSIM_API MyOctree
{
public:
	FBox WorldBounds;

	void SetWorldBounds(FVector min, FVector max);
	MyOctree();
	~MyOctree();
};
