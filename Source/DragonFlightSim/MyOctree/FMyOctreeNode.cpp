


#include "FMyOctreeNode.h"

FMyOctreeNode::FMyOctreeNode(FBox Bounds, FMyOctreeNode* Parent, int Depth, Octant SonType, uint32_t &OCTREE_NODE_ID) {
    this->SonType = SonType;
	this->ID = OCTREE_NODE_ID++;
	this->Parent = Parent;
	this->Bounds = Bounds;

	FVector Center = Bounds.GetCenter();
	FVector Min = Bounds.Min;
	FVector Max = Bounds.Max;


    //not creating child nodes here yet, just defining their potential bounds
    ChildBounds[Octant::LDB] = FBox(
        FVector(Center.X, Min.Y, Min.Z),
        FVector(Max.X, Center.Y, Center.Z)
    );

    ChildBounds[Octant::LDF] = FBox(
        FVector(Min.X, Min.Y, Min.Z),
        FVector(Center.X, Center.Y, Center.Z)
    );

    ChildBounds[Octant::LUB] = FBox(
        FVector(Center.X, Min.Y, Center.Z ),
        FVector(Max.X, Center.Y, Max.Z)
    );

    ChildBounds[Octant::LUF] = FBox(
        FVector(Min.X, Min.Y, Center.Z),
        FVector(Center.X, Center.Y, Max.Z)
    );

    ChildBounds[Octant::RDB] = FBox( 
        FVector(Center.X, Center.Y, Min.Z),
        FVector(Max.X, Max.Y, Center.Z)
    );

    ChildBounds[Octant::RDF] = FBox(
        FVector(Min.X, Center.Y, Min.Z),
        FVector(Center.X, Max.Y, Center.Z)
    );

    ChildBounds[Octant::RUB] = FBox(
        FVector(Center.X, Center.Y, Center.Z),
        FVector(Max.X, Max.Y, Max.Z)
    );

    ChildBounds[Octant::RUF] = FBox(
        FVector(Min.X, Center.Y, Center.Z),
        FVector(Center.X, Max.Y, Max.Z)
    );
}

inline FMyOctreeNode* FMyOctreeNode::GetChild(Octant Index) {
	if (Children == nullptr)
		return nullptr;
	return Children[Index];
}
