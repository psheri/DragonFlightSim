

#pragma once

#include "../MyOctree/FMyOctreeNode.h"

#include "CoreMinimal.h"

class FAStarEdge;

/**
 * 
 */
class DRAGONFLIGHTSIM_API FAStarNode
{
public:
	TArray<FAStarEdge*> EdgeList;
	FAStarNode* Path = nullptr;

	FMyOctreeNode* OctreeNode;

	float F;
	float G;
	float H;

	// store the node we came from during pathfinding
	FAStarNode* CameFrom;



	FAStarNode(FMyOctreeNode* OctreeNode) {
		this->OctreeNode = OctreeNode;
	}

	FMyOctreeNode* GetNode() {
		return this->OctreeNode;
	}

	~FAStarNode() {

	}
};

class DRAGONFLIGHTSIM_API FAStarEdge
{
public:
	FAStarNode* Start;
	FAStarNode* End;

	FAStarEdge(FAStarNode* Start, FAStarNode* End)
	{
		this->Start = Start;
		this->End = End;
	}
	~FAStarEdge() {
	
	}
};
