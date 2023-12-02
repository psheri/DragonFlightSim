

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class DRAGONFLIGHTSIM_API FMyOctreeNode
{
public:
	FBox Bounds;

	FMyOctreeNode** Children = nullptr;

	FBox ChildBounds[8] = {};

	TArray<AActor*> ContainedActors;

	bool IsLeaf() {
		return this->Children == nullptr;
	}
	FMyOctreeNode(FBox Bounds);
};

