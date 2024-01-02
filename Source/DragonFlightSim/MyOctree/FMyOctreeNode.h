

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

	enum Octant {
		LDB = 0, // left-down-back
		LDF = 1, // left-down-front
		LUB = 2, // left-up-back
		LUF = 3, // left-up-front
		RDB = 4, // right-down-back
		RDF = 5, // right-down-front
		RUB = 6, // right-up-back
		RUF = 7, // right-up-front
	};

	Octant SonType = LDB; // is this node a LDB, LDF, LUB, LUF, etc. child?

	bool IsLeaf() {
		return this->Children == nullptr;
	}
	bool IsEmptyLeaf() {
		return this->Children == nullptr && ContainedActors.Num() == 0;
	}
	FMyOctreeNode(FBox Bounds, FMyOctreeNode* Parent, int Depth, Octant SonType, uint32_t &OCTREE_NODE_ID);

	~FMyOctreeNode() {

	}

	FMyOctreeNode* GetChild(Octant Index);
};

