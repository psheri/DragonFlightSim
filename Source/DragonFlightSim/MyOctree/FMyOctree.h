

#pragma once

#include "CoreMinimal.h"
#include "../Utils/LogHelper.h"
#include "FMyOctreeNode.h"
#include "../AStar/FAStar.h"

/**
 * 
 */

#define MIN_NODE_DIMENSIONS 512

typedef FMyOctreeNode::Octant Octant;

class DRAGONFLIGHTSIM_API FMyOctree
{
	// increase by 1 every time a node is inserted
	uint32_t OCTREE_NODE_ID = 0;	
public:
	
	int MaxRecordedDepth = 0;

	FBox WorldBounds;

	FMyOctreeNode* Root = nullptr;

	TArray<AActor*> Obstacles;

	TArray<FMyOctreeNode*> LeafNodes;

	FAStar* AStar;

	FMyOctree(TArray<AActor*>& Obstacles, FAStar* AStar);

	bool bIsBuilt = false;

	// 26 directions
	enum DIRECTION {
		// 6 face directions
		L, R, D, U, B, F, 

		// 12 edge directions
		LD, LU, LB, LF, RD, RU, RB, RF, DB, DF, UB, UF,
		
		// 8 vertex directions
		LDB, LDF, LUB, LUF, RDB, RDF, RUB, RUF
	};


	/*
	* The functions in this region have been adapted from:
	* COMPUTER VISION, GRAPHICS, AND IMAGE PROCESSING 46, 367-386 (1989)
	* Hanan Samet
	* http://www.cs.umd.edu/~hjs/pubs/SameCVGIP89.pdf */

#pragma region H. Samet Neighbour Finding Algorithms

private:
	bool ADJ(DIRECTION I, Octant O) const;
	
	Octant REFLECT(DIRECTION I, Octant O) const;

	FMyOctreeNode* FATHER(FMyOctreeNode* P);

	FMyOctreeNode* SON(FMyOctreeNode* P, Octant O);
	
	Octant SONTYPE(FMyOctreeNode* P);

	FMyOctreeNode* OT_EQ_FACE_NEIGHBOUR(FMyOctreeNode* P, DIRECTION I);

#pragma endregion

public:
	void Build();
	
	void SetWorldBounds(FVector Min, FVector Max);

	void Insert(AActor* Obstacle);

	void GetLeafNodes(FMyOctreeNode* OctreeNode);

	void LinkLeafNeighbours();

	FMyOctreeNode* GetNodeAtPosition(const FVector &Position);

	FMyOctree();
	
	~FMyOctree();

	void ClearOctree();
	
	TArray<FMyOctreeNode*> GetNeighbours(FMyOctreeNode* Node);
private:
	
	TArray<FMyOctreeNode*> FindNeighboursOfSmallerSize(FMyOctreeNode* Node, DIRECTION Direction);

	FMyOctreeNode* GetNodeAtPosition(const FVector &Position, FMyOctreeNode* CurrentNode);
	
	void DeleteOctreeNode(FMyOctreeNode* CurrentNode, uint32_t &OutDeletedNodeCount);

	void DivideAndInsert(FMyOctreeNode* CurrentNode, AActor* Obstacle, int Depth);
};
