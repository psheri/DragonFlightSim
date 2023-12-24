


#include "FAStar.h"
#include "../Utils/TPriorityQueue.h"

FAStar::~FAStar()
{
	LogMain << "FAStar destructor called";
	for (auto& Elem : Nodes) {
		delete Elem.Value;
		Elem.Value = nullptr;
	}

	for (int i = 0; i < Edges.Num(); ++i) {
		delete Edges[i];
		Edges[i] = nullptr;
	}
}

FAStar::FAStar() {

}

void FAStar::AddNode(FMyOctreeNode* OctreeNode) {
	if (OctreeNode != nullptr && FindNode(OctreeNode->ID) == nullptr) {
		Nodes.Add(OctreeNode->ID, new FAStarNode(OctreeNode));
	}
}

void FAStar::AddEdge(FMyOctreeNode* StartNode, FMyOctreeNode* EndNode)
{
	FAStarNode* Start = FindNode(StartNode->ID);
	FAStarNode* End = FindNode(EndNode->ID);

	if (Start != nullptr && End != nullptr)
	{
		// check for duplicate edge
		FUint32Point EdgePair(StartNode->ID, EndNode->ID);
		// check both directions
		if (!UniqueEdgePairs.Contains(EdgePair) && !UniqueEdgePairs.Contains(FUint32Point(EdgePair.Y, EdgePair.X)))
		{
			// bidirectional edges
			FAStarEdge* Dir1 = new FAStarEdge(Start, End);
			FAStarEdge* Dir2 = new FAStarEdge(End, Start);

			Edges.Add(Dir1);
			Edges.Add(Dir2);
			Start->EdgeList.Add(Dir1);
			End->EdgeList.Add(Dir2);

			UniqueEdgePairs.Add(EdgePair);
			UniqueEdgePairs.Add(FUint32Point(EdgePair.Y, EdgePair.X));

			EdgeCount += 2;
		}
	}
}

FAStarNode* FAStar::FindNode(uint32_t OctreeNodeID) {
	FAStarNode** Node = Nodes.Find(OctreeNodeID);
	if (Node == nullptr || *Node == nullptr) {
		return nullptr;
	}
	return *Node;
}

// to do: implement heuristics that reduce the amount of nodes searched & bias the search to larger nodes.
bool FAStar::FindPath(FMyOctreeNode* Start, FMyOctreeNode* End, TArray<FAStarNode*>& OutPath) {
	if (Start == End) {
		return false;
	}
	if (Start == nullptr || End == nullptr) {
		// to do: handle case where start or end are obstructed
		// could find the closest neighbor and start from there
		return false;
	}
	FAStarNode* A = FindNode(Start->ID);
	FAStarNode* B = FindNode(End->ID);

	TPriorityQueue<FAStarNode*> OpenList;
	TSet<FAStarNode*> OpenListSet;
	TSet<FAStarNode*> ClosedList;

	float TempG = 0;
	bool bIsTempBetter = false;

	A->G = 0;
	A->H = (Start->Bounds.GetCenter() - End->Bounds.GetCenter()).SquaredLength();
	A->F = A->H;

	OpenList.Push(A, A->F);
	OpenListSet.Add(A);

	while (!OpenList.IsEmpty()) {
		FAStarNode* Current = OpenList.Pop();
		OpenListSet.Remove(Current);

		if (Current == B) {
			// we found the path
			//LogMain << "closedList size = " << ClosedList.Num();
			ReconstructPath(A, B, OutPath);
			return true;
		}

		ClosedList.Add(Current);

		FAStarNode* Neighbor;
		for (FAStarEdge* Edge : Current->EdgeList) {
			Neighbor = Edge->End;
			Neighbor->G = Current->G + (Current->OctreeNode->Bounds.GetCenter() - Neighbor->OctreeNode->Bounds.GetCenter()).SquaredLength();

			if (ClosedList.Contains(Neighbor)) {
				continue;
			}

			TempG = Current->G + (Current->OctreeNode->Bounds.GetCenter() - Neighbor->OctreeNode->Bounds.GetCenter()).SquaredLength();
			if (!OpenListSet.Contains(Neighbor)) {
				OpenList.Push(Neighbor, Neighbor->F);
				OpenListSet.Add(Neighbor);
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

