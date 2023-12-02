


#include "FMyOctree.h"

FMyOctree::FMyOctree(TArray<AActor*>& Obstacles, FAStar* AStar)
{
	this->AStar = AStar;
	// we want to make a cube that encompasses all obstacle bounds
	FVector Min = { FLT_MAX,  FLT_MAX,  FLT_MAX };
	FVector Max = {-FLT_MAX, -FLT_MAX, -FLT_MAX };

	this->Obstacles = Obstacles;

	if (this->Obstacles.Num() > 0) {
		this->UWORLD = this->Obstacles[0]->GetWorld();
	}
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

	this->Root = new FMyOctreeNode(WorldBounds, nullptr);


}

void FMyOctree::Build()
{
	for (AActor* Obstacle : this->Obstacles) {
		this->Insert(Obstacle);
	}

	GetLeafNodes(this->Root);
	LogMain << "Empty leaf node count: " << this->LeafNodes.Num();
	ProcessLinks();
	LogMain << "Total A* Edges: " << this->AStar->EdgeCount;
}

void FMyOctree::SetWorldBounds(FVector Min, FVector Max)
{
	this->WorldBounds = FBox(Min, Max);
}

void FMyOctree::Insert(AActor* Obstacle)
{
	DivideAndInsert(Root, Obstacle);
}

void FMyOctree::GetLeafNodes(FMyOctreeNode* OctreeNode) {
	if (OctreeNode == nullptr)
		return;
	if (OctreeNode->Children == nullptr) {
		if (OctreeNode->ContainedActors.Num() == 0) {
			LeafNodes.Add(OctreeNode);
			AStar->AddNode(OctreeNode);
		}
	}
	else {
		for (int i = 0; i < 8; i++) {
			GetLeafNodes(OctreeNode->Children[i]);
			// get siblings
			for (int s = 0; s < 8; s++) {
				if (s != i) {
					AStar->AddEdge(OctreeNode->Children[i], OctreeNode->Children[s]);
				}
			}
		}
	}
}

void FMyOctree::ProcessLinks()
{
	// store node IDs on a hashmap to avoid redundant entries
	TMap<uint32_t, uint32_t> SubGraphConnections;

	// bruteforce check
	for (FMyOctreeNode* n : LeafNodes) {
		for (FMyOctreeNode* m : LeafNodes) {
			if (n->ID != m->ID && n->Parent->ID != m->Parent->ID) {
				FVector StartPos = n->Bounds.GetCenter();
				FVector EndPos = m->Bounds.GetCenter();
				FCollisionQueryParams CollisionParams;
				FHitResult OutHit;

				// sphere cast
				bool bHit =  this->UWORLD->SweepSingleByChannel(
					OutHit,
					StartPos,
					EndPos,
					FQuat::Identity,
					ECC_Visibility, //ECC_GameTraceChannel11,
					FCollisionShape::MakeSphere(64.0f),
					CollisionParams
				);

				if (!bHit)
				{
					LogMain << "no hit -> link(" << n->ID << ", " << m->ID << ")";
					if (!SubGraphConnections.Contains(n->Parent->ID))
					{
						SubGraphConnections.Add(n->Parent->ID, m->Parent->ID);
						AStar->AddEdge(n, m);
					}
					
				}
			}
		}
	}
}

FMyOctree::FMyOctree()
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
			CurrentNode->Children[i] = new FMyOctreeNode(CurrentNode->ChildBounds[i], CurrentNode);
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

FMyOctree::~FMyOctree()
{
	
}

