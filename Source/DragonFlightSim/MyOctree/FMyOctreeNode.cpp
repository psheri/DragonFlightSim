


#include "FMyOctreeNode.h"

FMyOctreeNode::FMyOctreeNode(FBox Bounds) {
	this->Bounds = Bounds;

	FVector Center = Bounds.GetCenter();
	FVector Min = Bounds.Min;
	FVector Max = Bounds.Max;

	//not creating child nodes here yet, just defining their potential bounds
	for (int i = 0; i < 8; ++i) {
		FVector ChildMin, ChildMax;

		ChildMin.X = (i & 1) ? Center.X : Min.X;
		ChildMin.Y = (i & 2) ? Center.Y : Min.Y;
		ChildMin.Z = (i & 4) ? Center.Z : Min.Z;

		ChildMax.X = (i & 1) ? Max.X : Center.X;
		ChildMax.Y = (i & 2) ? Max.Y : Center.Y;
		ChildMax.Z = (i & 4) ? Max.Z : Center.Z;

		ChildBounds[i] = FBox(ChildMin, ChildMax);

	}
}
