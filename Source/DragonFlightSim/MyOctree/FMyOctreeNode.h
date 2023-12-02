

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class DRAGONFLIGHTSIM_API FMyOctreeNode
{
public:
	uint32_t ID;
	FBox Bounds;
	FMyOctreeNode** Children = nullptr;
	FBox ChildBounds[8] = {};
	TArray<AActor*> ContainedActors;
	FMyOctreeNode* Parent;

	bool IsLeaf() {
		return this->Children == nullptr;
	}
	FMyOctreeNode(FBox Bounds, FMyOctreeNode* Parent);

	~FMyOctreeNode() {

	}
};

