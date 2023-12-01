

#pragma once

#include "CoreMinimal.h"
#include "LogHelper.h"

/**
 * 
 */

#define MIN_NODE_DIMENSIONS 128

struct FOctreeNode {
	bool bIsLeaf;
	FBox Bounds;

	FOctreeNode** Children = nullptr;

	FBox ChildBounds[8] = {};
	FOctreeNode(FBox Bounds) {
		this->bIsLeaf = false;
		this->Bounds = Bounds;

		FVector Center = Bounds.GetCenter();
		FVector Min = Bounds.Min;
		FVector Max = Bounds.Max;

		//not creating child nodes here yet, just defining their potential bounds
		for (int i = 0; i < 8; ++i) {
			FVector ChildMin, ChildMax;

			ChildMin.X = (i & 1) ? Center.X : Min.X;
			ChildMin.Y = (i & 2) ? Center.Y : Min.Y;
			ChildMin.Z = (i & 4) ? Center.Z : Min.Z;

			ChildMax.X = (i & 1) ? Max.X : Center.X;
			ChildMax.Y = (i & 2) ? Max.Y : Center.Y;
			ChildMax.Z = (i & 4) ? Max.Z : Center.Z;

			ChildBounds[i] = FBox(ChildMin, ChildMax);

		}
	}
};

class DRAGONFLIGHTSIM_API FMyOctree
{
public:
	FBox WorldBounds;

	FOctreeNode* Root;

	TArray<AActor*> Obstacles;
	FMyOctree(TArray<AActor*>& Obstacles);


	void Build();
	
	void SetWorldBounds(FVector Min, FVector Max);

	void Insert(AActor* Obstacle);

	FMyOctree();
	~FMyOctree();

private:
	void DivideAndInsert(FOctreeNode* CurrentNode, AActor* Obstacle);
};
