


#include "FMyOctree.h"

FMyOctree::FMyOctree(TArray<AActor*>& Obstacles)
{
	// we want to make a cube that encompasses all obstacle bounds
	FVector Min = { FLT_MAX,  FLT_MAX,  FLT_MAX };
	FVector Max = {-FLT_MAX, -FLT_MAX, -FLT_MAX };

	this->Obstacles = Obstacles;

	for (AActor* Obstacle : this->Obstacles) {
		FBox obstacleBounds = Obstacle->GetComponentByClass<UStaticMeshComponent>()->Bounds.GetBox();
		Min.X = FMath::Min(Min.X, obstacleBounds.Min.X);
		Min.Y = FMath::Min(Min.Y, obstacleBounds.Min.Y);
		Min.Z = FMath::Min(Min.Z, obstacleBounds.Min.Z);
								  
		Max.X = FMath::Max(Max.X, obstacleBounds.Max.X);
		Max.Y = FMath::Max(Max.Y, obstacleBounds.Max.Y);
		Max.Z = FMath::Max(Max.Z, obstacleBounds.Max.Z);
	}

	//ensure the world bounds form a cube shape for even subdivisions. clamp to nearest power of 2.
	FBox TempBounds = FBox(Min, Max);
	float MaxSize = FMath::Max<double>({ TempBounds.GetSize().X, TempBounds.GetSize().Y,  TempBounds.GetSize().Z })/2;
	int32 NearestPowerOf2 = FMath::RoundToInt(FMath::Pow(2, FMath::CeilToFloat(FMath::Log2(MaxSize))));
	
	FVector WorldSize = { (float)NearestPowerOf2, (float)NearestPowerOf2, (float)NearestPowerOf2 };

	this->SetWorldBounds(TempBounds.GetCenter() - WorldSize, TempBounds.GetCenter() + WorldSize);

	this->Root = new FMyOctreeNode(WorldBounds);


}

void FMyOctree::Build()
{
	for (AActor* Obstacle : this->Obstacles) {
		this->Insert(Obstacle);
	}
}

void FMyOctree::SetWorldBounds(FVector Min, FVector Max)
{
	this->WorldBounds = FBox(Min, Max);
}

void FMyOctree::Insert(AActor* Obstacle)
{
	DivideAndInsert(Root, Obstacle);
}

FMyOctree::FMyOctree()
{
}

FMyOctree::~FMyOctree()
{
}

void FMyOctree::DivideAndInsert(FMyOctreeNode* CurrentNode, AActor* Obstacle)
{
	// end recursion here
	if (CurrentNode->Bounds.GetSize().X <= MIN_NODE_DIMENSIONS) {
		CurrentNode->ContainedActors.Add(Obstacle);
		return;
	}
	if (CurrentNode->Children == nullptr) {
		CurrentNode->Children = new FMyOctreeNode * [8] { nullptr }; //initialize all children to nullptr
	}
	bool bDividing = false;
	for (int i = 0; i < 8; ++i) {
		if (CurrentNode->Children[i] == nullptr) {
			CurrentNode->Children[i] = new FMyOctreeNode(CurrentNode->ChildBounds[i]);
		}
		// if obstacle's bbox intersects with the current octant, push it down the tree and subdivide further
		if (CurrentNode->ChildBounds[i].Intersect(Obstacle->GetComponentByClass<UStaticMeshComponent>()->Bounds.GetBox())) {
			bDividing = true;
			this->DivideAndInsert(CurrentNode->Children[i], Obstacle);
		}
	}
	if (!bDividing) {
		CurrentNode->ContainedActors.Add(Obstacle);
		// delete each individual FMyOctreeNode
		for (int i = 0; i < 8; ++i) {
			if (CurrentNode->Children[i] != nullptr) {
				delete CurrentNode->Children[i];
				CurrentNode->Children[i] = nullptr;
			}
		}
		// delete the array
		delete[] CurrentNode->Children;
	}
}


