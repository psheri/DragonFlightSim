


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
}

void FMyOctree::Build()
{
	this->Root = new FMyOctreeNode(WorldBounds, nullptr, 0, OCTREE_NODE_ID);

	for (AActor* Obstacle : this->Obstacles) {
		this->Insert(Obstacle);
	}

	GetLeafNodes(this->Root);
	LogMain << "Empty leaf node count: " << this->LeafNodes.Num();
	LinkLeafNeighbours();
	LogMain << "Total A* Edges: " << this->AStar->EdgeCount;
	bIsBuilt = true;
}

void FMyOctree::SetWorldBounds(FVector Min, FVector Max)
{
	this->WorldBounds = FBox(Min, Max);
}

void FMyOctree::Insert(AActor* Obstacle)
{
	DivideAndInsert(Root, Obstacle, 0);
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

FMyOctree::FMyOctree()
{
}

void FMyOctree::LinkLeafNeighbours()
{
	for (int i = 0; i < LeafNodes.Num(); ++i) {
		TArray<FMyOctreeNode*> Neighbors = GetFaceNeighbors(LeafNodes[i]);

		if (Neighbors.Num() > 1)
			LogMain << "linked face neighbours: " << Neighbors.Num();
		for (FMyOctreeNode* CurrentNeighbor : Neighbors) {
			AStar->AddEdge(LeafNodes[i], CurrentNeighbor);
		}
	}
}

void FMyOctree::DivideAndInsert(FMyOctreeNode* CurrentNode, AActor* Obstacle, int Depth)
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
	Depth++;
	if (Depth > MaxRecordedDepth)
		MaxRecordedDepth = Depth;
	LogMain << "Depth: " << Depth;
	for (int i = 0; i < 8; ++i) {
		if (CurrentNode->Children[i] == nullptr) {
			CurrentNode->Children[i] = new FMyOctreeNode(CurrentNode->ChildBounds[i], CurrentNode, Depth, OCTREE_NODE_ID);
		}
		// if obstacle's bbox intersects with the current octant, push it down the tree and subdivide further
		if (CurrentNode->ChildBounds[i].Intersect(Obstacle->GetComponentByClass<UStaticMeshComponent>()->Bounds.GetBox())) {
			bDividing = true;
			this->DivideAndInsert(CurrentNode->Children[i], Obstacle, Depth);
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

TArray<FMyOctreeNode*> FMyOctree::GetFaceNeighbors(FMyOctreeNode* Node)
{
	TArray<FMyOctreeNode*> FaceNeighbors;

	if (Node == nullptr) {
		return FaceNeighbors;
	}

	// directional offsets for face neighbors
	TArray<FVector> NeighborOffsets = {
		FVector(1, 0, 0),
		FVector(-1, 0, 0),
		FVector(0, 1, 0),
		FVector(0, -1, 0),
		FVector(0, 0, 1),
		FVector(0, 0, -1)
	};

	FVector NodePosition = Node->Bounds.GetCenter();

	for (int i = 0; i < 6; ++i) {
		// increase offset by a small value so it overextends slightly
		FVector NeighborPosition = NodePosition + (NeighborOffsets[i] * 1.001) * Node->Bounds.GetExtent().X;

		FMyOctreeNode* NeighborNode = GetNodeAtPosition(NeighborPosition);

		// unobstructed leaf node
		if (NeighborNode != nullptr && NeighborNode->IsLeaf() && NeighborNode != Node) {
			FaceNeighbors.Add(NeighborNode);
		}
	}
	return FaceNeighbors;
}

FMyOctreeNode* FMyOctree::GetNodeAtPosition(const FVector &Position)
{
	return GetNodeAtPosition(Position, Root);
}

FMyOctreeNode* FMyOctree::GetNodeAtPosition(const FVector &Position, FMyOctreeNode* CurrentNode)
{
	if (CurrentNode == nullptr) {
		return nullptr;
	}

	FMyOctreeNode* Result = nullptr;

	// leaf node
	if (CurrentNode->Children == nullptr) {
		// check if the leaf node is empty
		if (FMath::PointBoxIntersection(Position, CurrentNode->Bounds) && CurrentNode->ContainedActors.Num() == 0)
			return CurrentNode;
	}
	else {
		for (int i = 0; i < 8; ++i) {
			Result = GetNodeAtPosition(Position, CurrentNode->Children[i]);
			if (Result != nullptr)
				break;
		}
	}
	return Result;
}

void FMyOctree::ClearOctree() {
	uint32_t OutDeletedNodeCount = 0;
	this->DeleteOctreeNode(Root, OutDeletedNodeCount);
	LogMain << "@FMyOctree::ClearOctree -> deleted " << OutDeletedNodeCount << " nodes";
}

void FMyOctree::DeleteOctreeNode(FMyOctreeNode* CurrentNode, uint32_t &OutDeletedNodeCount) {
	if (CurrentNode == nullptr)	// prevent crash if destructor is called before octree is built
		return;

	// free dynamically alloc'd mem
	if (CurrentNode->Children != nullptr) {
		for (int i = 0; i < 8; ++i) {
			if (CurrentNode->Children[i] != nullptr) {
				DeleteOctreeNode(CurrentNode->Children[i], OutDeletedNodeCount);
			}
		}
	}
	// delete the current node
	delete CurrentNode;
	CurrentNode = nullptr;

	OutDeletedNodeCount++;
}

FMyOctree::~FMyOctree()
{
	LogMain << "FMyOctree destructor called -> clear octree";
	this->ClearOctree();
}

