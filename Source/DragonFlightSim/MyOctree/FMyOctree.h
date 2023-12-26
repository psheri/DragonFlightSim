

#pragma once

#include "CoreMinimal.h"
#include "../Utils/LogHelper.h"
#include "FMyOctreeNode.h"
#include "../AStar/FAStar.h"
/**
 * 
 */

#define MIN_NODE_DIMENSIONS 512


class DRAGONFLIGHTSIM_API FMyOctree
{
	// increase by 1 every time a node is inserted
	uint32_t OCTREE_NODE_ID = 0;	
public:
	
	int MaxRecordedDepth = 0;

	FBox WorldBounds;

	FMyOctreeNode* Root = nullptr;

	TArray<AActor*> Obstacles;

	TArray<FMyOctreeNode*> LeafNodes;

	FAStar* AStar;

	FMyOctree(TArray<AActor*>& Obstacles, FAStar* AStar);

	bool bIsBuilt = false;

	enum CardinalDir {
		NORTH = 0,
		SOUTH = 1,
		EAST = 2,
		WEST = 3,
		UP = 4,
		DOWN = 5
	};

	void Build();
	
	void SetWorldBounds(FVector Min, FVector Max);

	void Insert(AActor* Obstacle);

	void GetLeafNodes(FMyOctreeNode* OctreeNode);

	void LinkLeafNeighbours();

	FMyOctreeNode* GetNodeAtPosition(const FVector &Position);

	FMyOctree();
	~FMyOctree();

	void ClearOctree();

	FMyOctreeNode* GetNeighbourOfGreaterOrEqualSize(FMyOctreeNode* Node, CardinalDir Direction);
	TArray<FMyOctreeNode*> GetNeighbours(FMyOctreeNode* Node);
private:
	FMyOctreeNode* RGetNeighbourOfGreaterOrEqualSize(FMyOctreeNode* Node, CardinalDir Direction);
	TArray<FMyOctreeNode*> FindNeighboursOfSmallerSize(FMyOctreeNode* Node, CardinalDir Direction);

	FMyOctreeNode* GetNodeAtPosition(const FVector &Position, FMyOctreeNode* CurrentNode);
	void DeleteOctreeNode(FMyOctreeNode* CurrentNode, uint32_t &OutDeletedNodeCount);
private:
	
	void DivideAndInsert(FMyOctreeNode* CurrentNode, AActor* Obstacle, int Depth);
};
