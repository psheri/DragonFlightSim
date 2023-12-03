

#pragma once

#include "CoreMinimal.h"
#include "../Utils/LogHelper.h"
#include "FMyOctreeNode.h"
#include "../AStar/FAStar.h"
/**
 * 
 */

#define MIN_NODE_DIMENSIONS 128



class DRAGONFLIGHTSIM_API FMyOctree
{
public:
	
    int MaxRecordedDepth = 0;

	FBox WorldBounds;

	FMyOctreeNode* Root = nullptr;

	TArray<AActor*> Obstacles;

	TArray<FMyOctreeNode*> LeafNodes;

	FAStar* AStar;

	FMyOctree(TArray<AActor*>& Obstacles, FAStar* AStar);

	UWorld* UWORLD;

	bool bIsBuilt = false;
	void Build();
	
	void SetWorldBounds(FVector Min, FVector Max);

	void Insert(AActor* Obstacle);

	void GetLeafNodes(FMyOctreeNode* OctreeNode);

	void LinkLeafNeighbours();

    TArray<FMyOctreeNode*> GetFaceNeighbors(FMyOctreeNode* Node);

	FMyOctreeNode* GetNodeAtPosition(const FVector &Position);

	FMyOctree();
	~FMyOctree();

	void ClearOctree();
private:
	FMyOctreeNode* GetNodeAtPosition(const FVector &Position, FMyOctreeNode* CurrentNode);
	void DeleteOctreeNode(FMyOctreeNode* CurrentNode, uint32_t &OutDeletedNodeCount);
private:
	
	void DivideAndInsert(FMyOctreeNode* CurrentNode, AActor* Obstacle, int Depth);
};
