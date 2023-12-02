

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
	TArray<FAStarNode*> Nodes;
	TArray<FAStarNode*> PathList;

	~FAStar();
	FAStar();

	void AddNode(FMyOctreeNode* OctreeNode);

	void AddEdge(FMyOctreeNode* StartNode, FMyOctreeNode* EndNode);

	FAStarNode* FindNode(uint32_t OctreeNodeID);

	int GetPathLength();

	FMyOctreeNode* GetPathPoint(int i);

	bool FindPath(FMyOctreeNode* Start, FMyOctreeNode* End);
};
