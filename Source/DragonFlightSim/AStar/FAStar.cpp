


#include "FAStar.h"

FAStar::~FAStar()
{
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

int FAStar::GetPathLength() {
	return PathList.Num();
}

FMyOctreeNode* FAStar::GetPathPoint(int i) {
	return PathList[i]->OctreeNode;
}

// implement A* pathfinding
bool FAStar::FindPath(FMyOctreeNode* Start, FMyOctreeNode* End) {
	PathList = TArray<FAStarNode*>();
	return true;
}
