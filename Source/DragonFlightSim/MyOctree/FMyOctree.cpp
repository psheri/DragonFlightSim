


#include "FMyOctree.h"

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
	this->Root = new FMyOctreeNode(WorldBounds, nullptr, 0, OCTREE_NODE_ID);

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
			if (CurrentNeighbour->IsEmptyLeaf())
				AStar->AddEdge(LeafNodes[i], CurrentNeighbour);
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

FMyOctreeNode* FMyOctree::GetNeighbourOfGreaterOrEqualSize(FMyOctreeNode* Node, CardinalDir Direction)
{
	return RGetNeighbourOfGreaterOrEqualSize(Node, Direction);
}

TArray<FMyOctreeNode*> FMyOctree::GetNeighbours(FMyOctreeNode* Node)
{
	TArray<FMyOctreeNode*> Neighbours;

	// NORTH
	{
		FMyOctreeNode* Neighbour = GetNeighbourOfGreaterOrEqualSize(Node, CardinalDir::NORTH);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, CardinalDir::NORTH));
	}
	// SOUTH
	{
		FMyOctreeNode* Neighbour = GetNeighbourOfGreaterOrEqualSize(Node, CardinalDir::SOUTH);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, CardinalDir::SOUTH));
	}
	// EAST
	{
		FMyOctreeNode* Neighbour = GetNeighbourOfGreaterOrEqualSize(Node, CardinalDir::EAST);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, CardinalDir::EAST));
	}
	// WEST
	{
		FMyOctreeNode* Neighbour = GetNeighbourOfGreaterOrEqualSize(Node, CardinalDir::WEST);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, CardinalDir::WEST));
	}
	// UP
	{
		FMyOctreeNode* Neighbour = GetNeighbourOfGreaterOrEqualSize(Node, CardinalDir::UP);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, CardinalDir::UP));
	}
	// DOWN
	{
		FMyOctreeNode* Neighbour = GetNeighbourOfGreaterOrEqualSize(Node, CardinalDir::DOWN);
		Neighbours.Append(FindNeighboursOfSmallerSize(Neighbour, CardinalDir::DOWN));
	}
	return Neighbours;
}


typedef FMyOctreeNode::ChildIndex ChildIndex;

/*
* Finds the face neighbours of a given Octree Node without using intersection tests.
* Adapted from http://www.cs.umd.edu/~hjs/pubs/SameCVGIP89.pdf 
* and https://geidav.wordpress.com/2017/12/02/advanced-octrees-4-finding-neighbour-nodes/ 
*/
FMyOctreeNode* FMyOctree::RGetNeighbourOfGreaterOrEqualSize(FMyOctreeNode* Node, CardinalDir Direction)
{
	switch (Direction) {
		case CardinalDir::NORTH: {
			if (Node->Parent == nullptr) {	//reached Root
				return nullptr;
			}
			// current node == top-south-west child ? return top-north-west
			if (Node->Parent->Children[ChildIndex::TSW] == Node) { 
				return Node->Parent->Children[ChildIndex::TNW];
			}
			// current node == top-south-east child ? return top-north-east
			if (Node->Parent->Children[ChildIndex::TSE] == Node) {
				return Node->Parent->Children[ChildIndex::TNE];
			}
			// current node == bottom-south-west child ? return bottom-north-west
			if (Node->Parent->Children[ChildIndex::BSW] == Node) {
				return Node->Parent->Children[ChildIndex::BNW];
			}
			// current node == bottom-south-east child ? return bottom-north-east
			if (Node->Parent->Children[ChildIndex::BSE] == Node) {
				return Node->Parent->Children[ChildIndex::BNE];
			}
			Node = RGetNeighbourOfGreaterOrEqualSize(Node->Parent, Direction);
			if (Node == nullptr || Node->IsEmptyLeaf()) {
				return Node;
			}
			// Node is guaranteed to be a north child
			if (Node->Parent->Children[ChildIndex::TNW] == Node) {
				return Node->Children[ChildIndex::TSW];
			}
			else if (Node->Parent->Children[ChildIndex::TNE] == Node) {
				return Node->Children[ChildIndex::TSE];
			}
			else if (Node->Parent->Children[ChildIndex::BNW] == Node) {
				return Node->Children[ChildIndex::BSW];
			}
			else if (Node->Parent->Children[ChildIndex::BNE] == Node) {
				return Node->Children[ChildIndex::BSE];
			}
			return nullptr;
		}
		case CardinalDir::SOUTH: {
			if (Node->Parent == nullptr) { // reached Root
				return nullptr;
			}
			// current node == top-north-west child ? return top-south-west
			if (Node->Parent->Children[ChildIndex::TNW] == Node) {
				return Node->Parent->Children[ChildIndex::TSW];
			}
			// current node == top-north-east child ? return top-south-east
			if (Node->Parent->Children[ChildIndex::TNE] == Node) {
				return Node->Parent->Children[ChildIndex::TSE];
			}
			// current node == bottom-north-west child ? return bottom-south-west
			if (Node->Parent->Children[ChildIndex::BNW] == Node) {
				return Node->Parent->Children[ChildIndex::BSW];
			}
			// current node == bottom-north-east child ? return bottom-south-east
			if (Node->Parent->Children[ChildIndex::BNE] == Node) {
				return Node->Parent->Children[ChildIndex::BSE];
			}
			Node = RGetNeighbourOfGreaterOrEqualSize(Node->Parent, Direction);
			if (Node == nullptr || Node->IsEmptyLeaf()) {
				return Node;
			}
			// Node is guaranteed to be a south child
			if (Node->Parent->Children[ChildIndex::TSW] == Node) {
				return Node->Children[ChildIndex::TNW];
			}
			else if (Node->Parent->Children[ChildIndex::TSE] == Node) {
				return Node->Children[ChildIndex::TNE];
			}
			else if (Node->Parent->Children[ChildIndex::BSW] == Node) {
				return Node->Children[ChildIndex::BNW];
			}
			else if (Node->Parent->Children[ChildIndex::BSE] == Node) {
				return Node->Children[ChildIndex::BNE];
			}
			return nullptr;
		}
		case CardinalDir::EAST: {
			if (Node->Parent == nullptr) {  // reached Root
				return nullptr;
			}
			// current node == top-south-west child ? return top-south-east
			if (Node->Parent->Children[ChildIndex::TSW] == Node) {
				return Node->Parent->Children[ChildIndex::TSE];
			}
			// current node == top-north-west child ? return top-north-east
			if (Node->Parent->Children[ChildIndex::TNW] == Node) {
				return Node->Parent->Children[ChildIndex::TNE];
			}
			// current node == bottom-south-west child ? return bottom-south-east
			if (Node->Parent->Children[ChildIndex::BSW] == Node) {
				return Node->Parent->Children[ChildIndex::BSE];
			}
			// current node == bottom-north-west child ? return bottom-north-east
			if (Node->Parent->Children[ChildIndex::BNW] == Node) {
				return Node->Parent->Children[ChildIndex::BNE];
			}
			Node = RGetNeighbourOfGreaterOrEqualSize(Node->Parent, Direction);
			if (Node == nullptr || Node->IsEmptyLeaf()) {
				return Node;
			}
			// Node is guaranteed to be an east child
			if (Node->Parent->Children[ChildIndex::TSE] == Node) {
				return Node->Children[ChildIndex::TSW];
			}
			else if (Node->Parent->Children[ChildIndex::TNE] == Node) {
				return Node->Children[ChildIndex::TNW];
			}
			else if (Node->Parent->Children[ChildIndex::BSE] == Node) {
				return Node->Children[ChildIndex::BSW];
			}
			else if (Node->Parent->Children[ChildIndex::BNE] == Node) {
				return Node->Children[ChildIndex::BNW];
			}
			return nullptr;
		}
		case CardinalDir::WEST: {
			if (Node->Parent == nullptr) {  // reached Root
				return nullptr;
			}
			// current node == top-south-east child ? return top-south-west
			if (Node->Parent->Children[ChildIndex::TSE] == Node) {
				return Node->Parent->Children[ChildIndex::TSW];
			}
			// current node == top-north-east child ? return top-north-west
			if (Node->Parent->Children[ChildIndex::TNE] == Node) {
				return Node->Parent->Children[ChildIndex::TNW];
			}
			// current node == bottom-south-east child ? return bottom-south-west
			if (Node->Parent->Children[ChildIndex::BSE] == Node) {
				return Node->Parent->Children[ChildIndex::BSW];
			}
			// current node == bottom-north-east child ? return bottom-north-west
			if (Node->Parent->Children[ChildIndex::BNE] == Node) {
				return Node->Parent->Children[ChildIndex::BNW];
			}
			Node = RGetNeighbourOfGreaterOrEqualSize(Node->Parent, Direction);
			if (Node == nullptr || Node->IsEmptyLeaf()) {
				return Node;
			}
			// Node is guaranteed to be a west child
			if (Node->Parent->Children[ChildIndex::TNW] == Node) {
				return Node->Children[ChildIndex::TSW];
			}
			else if (Node->Parent->Children[ChildIndex::TNE] == Node) {
				return Node->Children[ChildIndex::TSE];
			}
			else if (Node->Parent->Children[ChildIndex::BNW] == Node) {
				return Node->Children[ChildIndex::BSW];
			}
			else if (Node->Parent->Children[ChildIndex::BNE] == Node) {
				return Node->Children[ChildIndex::BSE];
			}
			return nullptr;
		}
		case CardinalDir::UP: {
			if (Node->Parent == nullptr) {  // reached Root
				return nullptr;
			}
			// current node == bottom-south-west child ? return top-south-west
			if (Node->Parent->Children[ChildIndex::BSW] == Node) {
				return Node->Parent->Children[ChildIndex::TSW];
			}
			// current node == bottom-north-west child ? return top-north-west
			if (Node->Parent->Children[ChildIndex::BNW] == Node) {
				return Node->Parent->Children[ChildIndex::TNW];
			}
			// current node == bottom-south-east child ? return top-south-east
			if (Node->Parent->Children[ChildIndex::BSE] == Node) {
				return Node->Parent->Children[ChildIndex::TSE];
			}
			// current node == bottom-north-east child ? return top-north-east
			if (Node->Parent->Children[ChildIndex::BNE] == Node) {
				return Node->Parent->Children[ChildIndex::TNE];
			}
			Node = RGetNeighbourOfGreaterOrEqualSize(Node->Parent, Direction);
			if (Node == nullptr || Node->IsEmptyLeaf()) {
				return Node;
			}
			// Node is guaranteed to be an up child
			if (Node->Parent->Children[ChildIndex::TSW] == Node) {
				return Node->Children[ChildIndex::BSW];
			}
			else if (Node->Parent->Children[ChildIndex::TNW] == Node) {
				return Node->Children[ChildIndex::BNW];
			}
			else if (Node->Parent->Children[ChildIndex::TSE] == Node) {
				return Node->Children[ChildIndex::BSE];
			}
			else if (Node->Parent->Children[ChildIndex::TNE] == Node) {
				return Node->Children[ChildIndex::BNE];
			}
			return nullptr;
		}
		case CardinalDir::DOWN: {
			if (Node->Parent == nullptr) {  // reached Root
				return nullptr;
			}
			// current node == top-south-west child ? return bottom-south-west
			if (Node->Parent->Children[ChildIndex::TSW] == Node) {
				return Node->Parent->Children[ChildIndex::BSW];
			}
			// current node == top-north-west child ? return bottom-north-west
			if (Node->Parent->Children[ChildIndex::TNW] == Node) {
				return Node->Parent->Children[ChildIndex::BNW];
			}
			// current node == top-south-east child ? return bottom-south-east
			if (Node->Parent->Children[ChildIndex::TSE] == Node) {
				return Node->Parent->Children[ChildIndex::BSE];
			}
			// current node == top-north-east child ? return bottom-north-east
			if (Node->Parent->Children[ChildIndex::TNE] == Node) {
				return Node->Parent->Children[ChildIndex::BNE];
			}
			Node = RGetNeighbourOfGreaterOrEqualSize(Node->Parent, Direction);
			if (Node == nullptr || Node->IsEmptyLeaf()) {
				return Node;
			}
			// Node is guaranteed to be a down child
			if (Node->Parent->Children[ChildIndex::BSW] == Node) {
				return Node->Children[ChildIndex::TSW];
			}
			else if (Node->Parent->Children[ChildIndex::BNW] == Node) {
				return Node->Children[ChildIndex::TNW];
			}
			else if (Node->Parent->Children[ChildIndex::BSE] == Node) {
				return Node->Children[ChildIndex::TSE];
			}
			else if (Node->Parent->Children[ChildIndex::BNE] == Node) {
				return Node->Children[ChildIndex::TNE];
			}
			return nullptr;
		}
		default: return nullptr;
	}
}

TArray<FMyOctreeNode*> FMyOctree::FindNeighboursOfSmallerSize(FMyOctreeNode* Node, CardinalDir Direction)
{
	TArray<FMyOctreeNode*> Candidates;
	TArray<FMyOctreeNode*> Neighbours;

	if (Node != nullptr) {
		Candidates.Add(Node);
	}

	switch(Direction) 
	{
		case CardinalDir::NORTH: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[ChildIndex::TSW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::TSE]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BSW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BSE]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case CardinalDir::SOUTH: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[ChildIndex::TNW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::TNE]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BNW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BNE]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case CardinalDir::EAST: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[ChildIndex::TSW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::TNW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BSW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BNW]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case CardinalDir::WEST: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[ChildIndex::TSE]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::TNE]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BSE]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BNE]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case CardinalDir::UP: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
					return Neighbours;
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[ChildIndex::BSW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BNW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BSE]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::BNE]);
				}
				Candidates.RemoveAt(0);
			}
			return Neighbours;
		}
		case CardinalDir::DOWN: {
			while (Candidates.Num() > 0) {
				if (Candidates[0]->IsEmptyLeaf()) {
					Neighbours.Add(Candidates[0]);
				}
				else if (Candidates[0]->ContainedActors.Num() == 0) {
					Candidates.Add(Candidates[0]->Children[ChildIndex::TSW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::TNW]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::TSE]);
					Candidates.Add(Candidates[0]->Children[ChildIndex::TNE]);
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

