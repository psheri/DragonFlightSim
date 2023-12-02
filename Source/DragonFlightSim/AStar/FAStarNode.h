

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
