

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
	

	FBox WorldBounds;

	FMyOctreeNode* Root = nullptr;

	TArray<AActor*> Obstacles;

	TArray<FMyOctreeNode*> LeafNodes;

	FAStar* AStar;

	FMyOctree(TArray<AActor*>& Obstacles, FAStar* AStar);

	UWorld* UWORLD;

	void Build();
	
	void SetWorldBounds(FVector Min, FVector Max);

	void Insert(AActor* Obstacle);

	void GetLeafNodes(FMyOctreeNode* OctreeNode);

	void ProcessLinks();

	FMyOctree();
	~FMyOctree();

private:
	
	void DivideAndInsert(FMyOctreeNode* CurrentNode, AActor* Obstacle);
};
