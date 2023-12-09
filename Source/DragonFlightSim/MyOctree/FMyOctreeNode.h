

#pragma once

#include "CoreMinimal.h"

/**
 * array indices of child nodes:
 *
 *         down octants:             up octants:
 *            [+x]
 *              N
 *           1  |  3		          5  |  7		 
 *   [-y] W  ---|---  E [+y]		  ---|---
 *           0  |  2				  4  |  6
 *              S
 *            [-x]   
 * 
 *       +z up | -z down
 */
class DRAGONFLIGHTSIM_API FMyOctreeNode
{
public:
	uint32_t ID;
	int Depth;
	FBox Bounds;
	FMyOctreeNode** Children = nullptr;
	FBox ChildBounds[8] = {};
	TArray<AActor*> ContainedActors;
	FMyOctreeNode* Parent = nullptr;

	enum ChildIndex {
		BSW = 0, // bottom south-west
		BNW = 1, // bottom north-west
		BSE = 2, // bottom south-east
		BNE = 3, // bottom north-east
		TSW = 4, // top south-west
		TNW = 5, // top north-west
		TSE = 6, // top south-east
		TNE = 7, // top north-east
	};

	bool IsLeaf() {
		return this->Children == nullptr;
	}
	bool IsEmptyLeaf() {
		return this->Children == nullptr && ContainedActors.Num() == 0;
	}
	FMyOctreeNode(FBox Bounds, FMyOctreeNode* Parent, int Depth, uint32_t &OCTREE_NODE_ID);

	~FMyOctreeNode() {

	}

	FMyOctreeNode* GetChild(ChildIndex Index);
};

