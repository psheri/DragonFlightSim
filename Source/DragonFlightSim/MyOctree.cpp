


#include "MyOctree.h"

MyOctree::MyOctree(TArray<AActor*>& obstacles)
{
	FVector min = { FLT_MAX,  FLT_MAX,  FLT_MAX };
	FVector max = {-FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (AActor* obstacle : obstacles) {
		FBox obstacleBounds = obstacle->GetComponentByClass<UStaticMeshComponent>()->Bounds.GetBox();
		min.X = FMath::Min(min.X, obstacleBounds.Min.X);
		min.Y = FMath::Min(min.Y, obstacleBounds.Min.Y);
		min.Z = FMath::Min(min.Z, obstacleBounds.Min.Z);
								  
		max.X = FMath::Max(max.X, obstacleBounds.Max.X);
		max.Y = FMath::Max(max.Y, obstacleBounds.Max.Y);
		max.Z = FMath::Max(max.Z, obstacleBounds.Max.Z);
	}

	this->SetWorldBounds(min, max);
	this->root = new OctreeNode(WorldBounds);
}

void MyOctree::SetWorldBounds(FVector min, FVector max)
{
	this->WorldBounds = FBox(min, max);
}

MyOctree::MyOctree()
{
}

MyOctree::~MyOctree()
{
}
