


#include "FAStar.h"

FAStar::~FAStar()
{
	LogMain << "FAStar destructor called";
	for (int i = 0; i < Nodes.Num(); ++i) {
		delete Nodes[i];
		Nodes[i] = nullptr;
	}

	for (int i = 0; i < Edges.Num(); ++i) {
		delete Edges[i];
		Edges[i] = nullptr;
	}
}

FAStar::FAStar() {

}

void FAStar::AddNode(FMyOctreeNode* OctreeNode) {
	if (FindNode(OctreeNode->ID) == nullptr) {
		Nodes.Add(new FAStarNode(OctreeNode));
	}
}

void FAStar::AddEdge(FMyOctreeNode* StartNode, FMyOctreeNode* EndNode) {
	EdgeCount++;
	FAStarNode* Start = FindNode(StartNode->ID);
	FAStarNode* End = FindNode(EndNode->ID);

	if (Start != nullptr && End != nullptr) {
		// bidirectional edges
		FAStarEdge* Dir1 = new FAStarEdge(Start, End);
		FAStarEdge* Dir2 = new FAStarEdge(End, Start);

		Edges.Add(Dir1);
		Edges.Add(Dir2);
		Start->EdgeList.Add(Dir1);
		End->EdgeList.Add(Dir2);

	}
}

FAStarNode* FAStar::FindNode(uint32_t OctreeNodeID) {
	// later: use a hashmap for fast retrieval
	for (auto& Node : Nodes) {
		if (Node->GetNode()->ID == OctreeNodeID) {
			return Node;
		}
	}
	return nullptr;
}

// implement A* pathfinding
// to do: optimize
bool FAStar::FindPath(FMyOctreeNode* Start, FMyOctreeNode* End, TArray<FAStarNode*>& OutPath) {
	if (Start == End) {
		return false;
	}

	FAStarNode* A = FindNode(Start->ID);
	FAStarNode* B = FindNode(End->ID);

	if (A == nullptr || B == nullptr) {
		// to do: handle case where start or end are obstructed
		// could find the closest neighbor and start from there
		return false;
	}

	TArray<FAStarNode*> OpenList;
	TArray<FAStarNode*> ClosedList;
	float TempG = 0;
	bool bIsTempBetter = false;
	// G = movement cost
	// H = heuristic cost
	// F = G + H
	A->G = 0; 
	A->H = (Start->Bounds.GetCenter() - End->Bounds.GetCenter()).SquaredLength();
	A->F = A->H;
	
	OpenList.Add(A);

	while (OpenList.Num() > 0) {
		int OutIndex = -1;
		FAStarNode* Current = FindLowestF(OpenList, OutIndex);

		if (Current == B) {
			//we found the path
			ReconstructPath(A, B, OutPath);
			return true;
		}

		OpenList.RemoveAt(OutIndex);
		ClosedList.Add(Current);

		FAStarNode* Neighbor;
		for (FAStarEdge* Edge : Current->EdgeList) {
			Neighbor = Edge->End;
			Neighbor->G = Current->G + (Current->OctreeNode->Bounds.GetCenter() - Neighbor->OctreeNode->Bounds.GetCenter()).SquaredLength();

			if (ClosedList.Contains(Neighbor)) {
				continue;
			}
	
			TempG = Current->G + (Current->OctreeNode->Bounds.GetCenter() - Neighbor->OctreeNode->Bounds.GetCenter()).SquaredLength();
			if (!OpenList.Contains(Neighbor)) {
				OpenList.Add(Neighbor);
				bIsTempBetter = true;
			}
			else if (TempG < Neighbor->G) {
				bIsTempBetter = true;
			}
			else {
				bIsTempBetter = false;
			}

			if (bIsTempBetter) {
				Neighbor->CameFrom = Current;
				Neighbor->G = TempG;
				Neighbor->H = (Current->OctreeNode->Bounds.GetCenter() - End->Bounds.GetCenter()).SquaredLength();
				Neighbor->F = Neighbor->G + Neighbor->H;
			}

		}
		

	}

	return false;
}

FAStarNode* FAStar::FindLowestF(TArray<FAStarNode*>& OpenList, int &OutIndex) {
	float LowestF = 0;
	int Count = 0;
	int IteratorCount = 0;

	if (OpenList.Num() == 0) {
		return nullptr;
	}

	for (int i = 0; i < OpenList.Num(); i++, Count++) {
		if (i == 0) {
			LowestF = OpenList[i]->F;
			IteratorCount = 0;
		}
		else if (OpenList[i]->F <= LowestF) {
			LowestF = OpenList[i]->F;
			IteratorCount = Count;
		}
	}
	OutIndex = IteratorCount;
	return OpenList[IteratorCount];
}

void FAStar::ReconstructPath(FAStarNode* A, FAStarNode* B, TArray<FAStarNode*>& OutPath)
{
	OutPath.Add(B);

	FAStarNode* P = B->CameFrom;
	while (P != nullptr && P != A) {
		OutPath.Insert(P, 0);
		P = P->CameFrom;
	}
	OutPath.Insert(A, 0);
}

