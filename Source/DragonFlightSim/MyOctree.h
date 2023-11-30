

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

#define MIN_NODE_DIMENSIONS 1.0f;

struct OctreeNode {
	bool isLeaf;
	FBox bounds;
	
	OctreeNode(FBox bounds) {
		this->bounds = bounds;	
	}
};

class DRAGONFLIGHTSIM_API MyOctree
{
public:
	FBox WorldBounds;

	OctreeNode* root;

	MyOctree(TArray<AActor*>& obstacles);


	void SetWorldBounds(FVector min, FVector max);
	MyOctree();
	~MyOctree();
};
