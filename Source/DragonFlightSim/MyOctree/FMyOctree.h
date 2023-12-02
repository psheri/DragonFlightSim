

#pragma once

#include "CoreMinimal.h"
#include "../Utils/LogHelper.h"
#include "FMyOctreeNode.h"

/**
 * 
 */

#define MIN_NODE_DIMENSIONS 128



class DRAGONFLIGHTSIM_API FMyOctree
{
public:
	FBox WorldBounds;

	FMyOctreeNode* Root;

	TArray<AActor*> Obstacles;
	FMyOctree(TArray<AActor*>& Obstacles);


	void Build();
	
	void SetWorldBounds(FVector Min, FVector Max);

	void Insert(AActor* Obstacle);

	FMyOctree();
	~FMyOctree();

private:
	void DivideAndInsert(FMyOctreeNode* CurrentNode, AActor* Obstacle);
};
