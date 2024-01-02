


#include "FMyOctree.h"

typedef Octant Octant;

FMyOctree::FMyOctree(TArray<AActor*>& Obstacles, FAStar* AStar)
{
	this->AStar = AStar;
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
}

void FMyOctree::Build()
{
	this->Root = new FMyOctreeNode(WorldBounds, nullptr, 0, Octant(0), OCTREE_NODE_ID);

	for (AActor* Obstacle : this->Obstacles) {
		this->Insert(Obstacle);
	}

	GetLeafNodes(this->Root);
	LogMain << "Empty leaf node count: " << this->LeafNodes.Num();
	LinkLeafNeighbours();
	LogMain << "Total A* Edges: " << this->AStar->EdgeCount;
	
	//int Count = 0;
	//for (FMyOctreeNode* LeafNode : LeafNodes) {
	//	Count += GetNeighbours(LeafNode).Num();
	//}
	//LogMain << "Count " << Count;
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
		TArray<FMyOctreeNode*> Neighbours = GetNeighbours(LeafNodes[i]);

		if (Neighbours.Num() > 0)
			LogMain << "linked face neighbours: " << Neighbours.Num();
		for (FMyOctreeNode* CurrentNeighbour : Neighbours) {
			if (CurrentNeighbour->IsEmptyLeaf()) {
				AStar->AddEdge(LeafNodes[i], CurrentNeighbour);
			}
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
	//LogMain << "Depth: " << Depth;
	for (int i = 0; i < 8; ++i) {
		if (CurrentNode->Children[i] == nullptr) {
			CurrentNode->Children[i] = new FMyOctreeNode(CurrentNode->ChildBounds[i], CurrentNode, Depth, (Octant)i, OCTREE_NODE_ID);
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

bool FMyOctree::ADJ(DIRECTION I, Octant O) const
{
	// based on TABLE 1

	// static, so it won't reallocate memory other than for the 1st call
	static const bool AdjacencyMatrix[26][8] = {
		{ 1, 1, 1, 1, 0, 0, 0, 0 }, // L
		{ 0, 0, 0, 0, 1, 1, 1, 1 }, // R
		{ 1, 1, 0, 0, 1, 1, 0, 0 }, // D
		{ 0, 0, 1, 1, 0, 0, 1, 1 }, // U
		{ 1, 0, 1, 0, 1, 0, 1, 0 }, // B
		{ 0, 1, 0, 1, 0, 1, 0, 1 }, // F
		{ 1, 1, 0, 0, 0, 0, 0, 0 }, // LD
		{ 0, 0, 1, 1, 0, 0, 0, 0 }, // LU
		{ 1, 0, 1, 0, 0, 0, 0, 0 }, // LB
		{ 0, 1, 0, 1, 0, 0, 0, 0 }, // LF
		{ 0, 0, 0, 0, 1, 1, 0, 0 }, // RD
		{ 0, 0, 0, 0, 0, 0, 1, 1 }, // RU
		{ 0, 0, 0, 0, 1, 0, 1, 0 }, // RB
		{ 0, 0, 0, 0, 0, 1, 0, 1 }, // RF
		{ 1, 0, 0, 0, 1, 0, 0, 0 }, // DB
		{ 0, 1, 0, 0, 0, 1, 0, 0 }, // DF
		{ 0, 0, 1, 0, 0, 0, 1, 0 }, // UB
		{ 0, 0, 0, 1, 0, 0, 0, 1 }, // UF
		{ 1, 0, 0, 0, 0, 1, 0, 0 }, // LDB
		{ 0, 1, 0, 0, 1, 0, 0, 0 }, // LDF
		{ 0, 0, 1, 0, 0, 0, 1, 0 }, // LUB
		{ 0, 0, 0, 1, 0, 0, 0, 1 }, // LUF
		{ 0, 0, 0, 0, 1, 0, 0, 0 }, // RDB
		{ 0, 0, 0, 0, 0, 1, 0, 0 }, // RDF
		{ 0, 0, 0, 0, 0, 0, 1, 0 }, // RUB
		{ 0, 0, 0, 0, 0, 0, 0, 1 }  // RUF
	};
	return AdjacencyMatrix[I][O];
}

Octant FMyOctree::REFLECT(DIRECTION I, Octant O) const {
	typedef FMyOctreeNode::Octant OCT;

	// based on TABLE 2

	// static, so it won't reallocate memory other than for the 1st call
	static const OCT reflectionMatrix[26][8] = {
		//-------------------------------- OCTANT[8] --------------------------------       // DIRECTION[26]
		{ OCT::RDB, OCT::RDF, OCT::RUB, OCT::RUF, OCT::LDB, OCT::LDF, OCT::LUB, OCT::LUF }, // L
		{ OCT::RDB, OCT::RDF, OCT::RUB, OCT::RUF, OCT::LDB, OCT::LDF, OCT::LUB, OCT::LUF }, // R
		{ OCT::LUB, OCT::LUF, OCT::LDB, OCT::LDF, OCT::RUB, OCT::RUF, OCT::RDB, OCT::RDF }, // D
		{ OCT::LUB, OCT::LUF, OCT::LDB, OCT::LDF, OCT::RUB, OCT::RUF, OCT::RDB, OCT::RDF }, // U
		{ OCT::LDF, OCT::LDB, OCT::LUF, OCT::LUB, OCT::RDF, OCT::RDB, OCT::RUF, OCT::RUB }, // B
		{ OCT::LDF, OCT::LDB, OCT::LUF, OCT::LUB, OCT::RDF, OCT::RDB, OCT::RUF, OCT::RUB }, // F
		{ OCT::RUB, OCT::RUF, OCT::RDB, OCT::RDF, OCT::LUB, OCT::LUF, OCT::LDB, OCT::LDF }, // LD
		{ OCT::RUB, OCT::RUF, OCT::RDB, OCT::RDF, OCT::LUB, OCT::LUF, OCT::LDB, OCT::LDF }, // LU
		{ OCT::RDF, OCT::RDB, OCT::RUF, OCT::RUB, OCT::LDF, OCT::LDB, OCT::LUF, OCT::LUB }, // LB
		{ OCT::RDF, OCT::RDB, OCT::RUF, OCT::RUB, OCT::LDF, OCT::LDB, OCT::LUF, OCT::LUB }, // LF
		{ OCT::RUB, OCT::RUF, OCT::RDB, OCT::RDF, OCT::LUB, OCT::LUF, OCT::LDB, OCT::LDF }, // RD
		{ OCT::RUB, OCT::RUF, OCT::RDB, OCT::RDF, OCT::LUB, OCT::LUF, OCT::LDB, OCT::LDF }, // RU
		{ OCT::RDF, OCT::RDB, OCT::RUF, OCT::RUB, OCT::LDF, OCT::LDB, OCT::LUF, OCT::LUB }, // RB
		{ OCT::RDF, OCT::RDB, OCT::RUF, OCT::RUB, OCT::LDF, OCT::LDB, OCT::LUF, OCT::LUB }, // RF
		{ OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB, OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB }, // DB
		{ OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB, OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB }, // DF
		{ OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB, OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB }, // UB
		{ OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB, OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB }, // UF
		{ OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB, OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB }, // LDB
		{ OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB, OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB }, // LDF
		{ OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB, OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB }, // LUB
		{ OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB, OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB }, // LUF
		{ OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB, OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB }, // RDB
		{ OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB, OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB }, // RDF
		{ OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB, OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB }, // RUB
		{ OCT::RUF, OCT::RUB, OCT::RDF, OCT::RDB, OCT::LUF, OCT::LUB, OCT::LDF, OCT::LDB }  // RUF
	};
	return reflectionMatrix[I][O];
}

FMyOctreeNode* FMyOctree::FATHER(FMyOctreeNode* P) {
	if (P == nullptr) {
		return nullptr;
	}
	return P->Parent;
}

FMyOctreeNode* FMyOctree::SON(FMyOctreeNode* P, Octant O) {
	if (P == nullptr)
		return nullptr;
	if (P->Children == nullptr) {
		return nullptr;
	}
	return P->Children[O];
}

Octant FMyOctree::SONTYPE(FMyOctreeNode* P) {
	return P->SonType;
}

FMyOctreeNode* FMyOctree::OT_EQ_FACE_NEIGHBOUR(FMyOctreeNode* P, DIRECTION I) {
	if (P->Parent == nullptr) {	//reached Root
		return nullptr;
	}

	if (ADJ(I, SONTYPE(P))) {
		FMyOctreeNode* Neighbour = OT_EQ_FACE_NEIGHBOUR(FATHER(P), I);
		if (Neighbour == nullptr || Neighbour->IsEmptyLeaf()) {
			return Neighbour;
		}
		return SON(Neighbour, REFLECT(I, SONTYPE(P)));
	}
	else {
		return SON(FATHER(P), REFLECT(I, SONTYPE(P)));
	}
	return nullptr;
}

TArray<FMyOctreeNode*> FMyOctree::GetNeighbours(FMyOctreeNode* Node)
{
	TArray<FMyOctreeNode*> Neighbours;

	// NORTH
	{
		FMyOctreeNode* Neighbour = OT_EQ_FACE_NEIGHBOUR(Node, DIRECTION::B);
		//if (Neighbour != nullptr && Neighbour->IsEmptyLeaf())
		//	Neighbours.Push(Neighbour);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, DIRECTION::B));
	}
	// SOUTH
	{
		FMyOctreeNode* Neighbour = OT_EQ_FACE_NEIGHBOUR(Node, DIRECTION::F);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, DIRECTION::F));
	}
	// EAST
	{
		FMyOctreeNode* Neighbour = OT_EQ_FACE_NEIGHBOUR(Node, DIRECTION::R);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, DIRECTION::R));
	}
	// WEST
	{
		FMyOctreeNode* Neighbour = OT_EQ_FACE_NEIGHBOUR(Node, DIRECTION::L);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, DIRECTION::L));
	}
	// UP
	{
		FMyOctreeNode* Neighbour = OT_EQ_FACE_NEIGHBOUR(Node, DIRECTION::U);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, DIRECTION::U));
	}
	// DOWN
	{
		FMyOctreeNode* Neighbour = OT_EQ_FACE_NEIGHBOUR(Node, DIRECTION::D);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, DIRECTION::D));
	}
	return Neighbours;
}

TArray<FMyOctreeNode*> FMyOctree::FindNeighboursOfSmallerSize(FMyOctreeNode* Node, DIRECTION Direction)
{
	TArray<FMyOctreeNode*> Candidates;
	TArray<FMyOctreeNode*> Neighbours;

	if (Node != nullptr) {
		Candidates.Add(Node);
	}

	switch(Direction) 
	{
		case DIRECTION::B: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[Octant::LUF]);
					Candidates.Add(Candidates[0]->Children[Octant::RUF]);
					Candidates.Add(Candidates[0]->Children[Octant::LDF]);
					Candidates.Add(Candidates[0]->Children[Octant::RDF]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case DIRECTION::F: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[Octant::LUB]);
					Candidates.Add(Candidates[0]->Children[Octant::RUB]);
					Candidates.Add(Candidates[0]->Children[Octant::LDB]);
					Candidates.Add(Candidates[0]->Children[Octant::RDB]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case DIRECTION::R: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[Octant::LUF]);
					Candidates.Add(Candidates[0]->Children[Octant::LUB]);
					Candidates.Add(Candidates[0]->Children[Octant::LDF]);
					Candidates.Add(Candidates[0]->Children[Octant::LDB]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case DIRECTION::L: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[Octant::RUF]);
					Candidates.Add(Candidates[0]->Children[Octant::RUB]);
					Candidates.Add(Candidates[0]->Children[Octant::RDF]);
					Candidates.Add(Candidates[0]->Children[Octant::RDB]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case DIRECTION::U: {
			while (Candidates.Num() > 0) {
				//LogMain << "Candidates.Num() = " << Candidates.Num() << ", Candidates[0]->IsEmptyLeaf() = " << Candidates[0]->IsEmptyLeaf();
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[Octant::LDF]);
					Candidates.Add(Candidates[0]->Children[Octant::LDB]);
					Candidates.Add(Candidates[0]->Children[Octant::RDF]);
					Candidates.Add(Candidates[0]->Children[Octant::RDB]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case DIRECTION::D: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[Octant::LUF]);
					Candidates.Add(Candidates[0]->Children[Octant::LUB]);
					Candidates.Add(Candidates[0]->Children[Octant::RUF]);
					Candidates.Add(Candidates[0]->Children[Octant::RUB]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
	}

	return Neighbours;
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

