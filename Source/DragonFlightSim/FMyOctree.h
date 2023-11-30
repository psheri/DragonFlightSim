

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

#define MIN_NODE_DIMENSIONS 1.0f;

struct FOctreeNode {
	bool bIsLeaf;
	FBox Bounds;
	
	FOctreeNode(FBox Bounds) {
		this->Bounds = Bounds;
	}
};

class DRAGONFLIGHTSIM_API FMyOctree
{
public:
	FBox WorldBounds;

	FOctreeNode* Root;

	FMyOctree(TArray<AActor*>& Obstacles);


	void SetWorldBounds(FVector Min, FVector Max);
	FMyOctree();
	~FMyOctree();
};
