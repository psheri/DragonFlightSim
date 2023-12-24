

#pragma once
#include "FAStarNode.h"
#include "../Utils/LogHelper.h"
#include "../MyOctree/FMyOctreeNode.h"

#include "CoreMinimal.h"

/**
 * 
 */
class DRAGONFLIGHTSIM_API FAStar
{

public:
	int EdgeCount = 0;
	TArray<FAStarEdge*> Edges;

	// maps FMyOctreeNode::ID to FAStarNode.
	TMap<uint32_t, FAStarNode*> Nodes;	

	TSet<FUint32Point> UniqueEdgePairs;	// prevent adding duplicate edges

	~FAStar();
	FAStar();

	void AddNode(FMyOctreeNode* OctreeNode);

	void AddEdge(FMyOctreeNode* StartNode, FMyOctreeNode* EndNode);

	FAStarNode* FindNode(uint32_t OctreeNodeID);

	bool FindPath(FMyOctreeNode* Start, FMyOctreeNode* End, TArray<FAStarNode*>& OutPath);

	void ReconstructPath(FAStarNode* A, FAStarNode* B, TArray<FAStarNode*>& OutPath);
};
