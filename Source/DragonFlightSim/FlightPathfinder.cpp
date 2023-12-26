


#include "FlightPathfinder.h"
#include <Kismet/GameplayStatics.h>

void AFlightPathfinder::DrawBBox(const FBox& BBox, float Thickness, FColor Color, bool bIsSolid)
{
	if (bIsSolid) {
		DrawDebugSolidBox(GetWorld(), BBox.GetCenter(), BBox.GetExtent(), Color, false, -1, 0U);
	}
	else {
		DrawDebugBox(GetWorld(), BBox.GetCenter(), BBox.GetExtent(), Color, false, -1, 0U, Thickness);
	}
}

void AFlightPathfinder::DrawPath(TArray<FVector>& Points)
{
	for (int i = 0; i < Points.Num()-1; ++i) {
		FVector& StartPos = Points[i];
		FVector& EndPos = Points[i+1];
		DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Magenta, false);
	}
}

// Sets default values
AFlightPathfinder::AFlightPathfinder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

TArray<FAStarNode*> AFlightPathfinder::FindPath(FVector StartPos, FVector EndPos)
{
	return TArray<FAStarNode*>();
}

TArray<FAStarNode*> AFlightPathfinder::FindRandomPath(const FVector* OverrideStartPos)
{
	TArray<FAStarNode*> OutPath;
	if (MyOctree.LeafNodes.Num() == 0)
		return OutPath;
	int StartIndex = FMath::RandRange(0, MyOctree.LeafNodes.Num()-1);
	int EndIndex = FMath::RandRange(0, MyOctree.LeafNodes.Num() - 1);
	//LogMain << "StartIndex = " << StartIndex << ", EndIndex = " << EndIndex;

	FMyOctreeNode* StartNode = MyOctree.LeafNodes[StartIndex];
	FMyOctreeNode* EndNode = MyOctree.LeafNodes[EndIndex];

	if (OverrideStartPos != nullptr) {
		StartNode = MyOctree.GetNodeAtPosition(*OverrideStartPos);
		if (StartNode == nullptr || !StartNode->IsEmptyLeaf()) {
			// need to look for closest neighbour leaf
			LogMain << "@AFlightPathfinder::FindRandomPath: unhandled case; path will be empty.";
		}
	}
	bool result = AStar.FindPath(StartNode, EndNode, OutPath);

	return OutPath;
}

AActor* Dragon;
// Called when the game starts or when spawned
void AFlightPathfinder::BeginPlay()
{
	Super::BeginPlay();
	//this->MyOctree.SetWorldBounds(this->min, this->max);
	TArray<AActor*> Obstacles;
	UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), FName("Obstacle"), Obstacles);
	
	this->MyOctree = FMyOctree(Obstacles, &AStar);
	this->MyOctree.Build();
	//Destroy();

	TArray<AActor*> Dragons;
	UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), FName("Dragon"), Dragons);
	if (Dragons.Num() > 0)
		Dragon = Dragons[0];
}

void AFlightPathfinder::DrawOctree(FMyOctreeNode* CurrentNode) {
	
	DrawBBox(CurrentNode->Bounds);

	for (AActor* Obstacle : CurrentNode->ContainedActors) {
		//LogMain << "Here";
		//DrawBBox(Obstacle->GetComponentByClass<UStaticMeshComponent>()->Bounds.GetBox(), 5, FColor::Magenta);
	}

	if (CurrentNode->Children != nullptr) {
		for (int i = 0; i < 8; i++) {
			if (CurrentNode->Children[i] != nullptr) {
				DrawOctree(CurrentNode->Children[i]);
			}

		}
	}
	else if (CurrentNode->IsLeaf() && CurrentNode->ContainedActors.Num() != 0) {
		//LogMain << "here";
		FColor solidColor = FColor(0, 0, 255, 32);
		//DrawBBox(CurrentNode->Bounds, 5, solidColor, true);
	}
}

void AFlightPathfinder::DrawFlightPath(TArray<FAStarNode*> Path)
{
	for (int i = 0; i < Path.Num()-1; i++) {
		DrawDebugLine(
			GetWorld(),
			Path[i]->OctreeNode->Bounds.GetCenter(),
			Path[i+1]->OctreeNode->Bounds.GetCenter(),
			FColor::Yellow,
			false, 
			0.25,
			0,
			100  // Thickness of the line
		);
	}
}

// Called every frame
void AFlightPathfinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DrawBBox(this->MyOctree.WorldBounds, 500);

	TArray<AActor*> Dragons;
	UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), FName("Dragon"), Dragons);
	if (Dragons.Num() > 0)
		Dragon = Dragons[0];
	
	if (this->bDrawAgentNeighbours) {
		if (Dragon != nullptr && Dragon->IsValidLowLevel()) {
			FBox DragonBounds = Dragon->GetComponentByClass<UStaticMeshComponent>()->Bounds.GetBox();
			FVector DragonPos = Dragon->GetActorLocation();
			FMyOctreeNode* DragonNode = MyOctree.GetNodeAtPosition(DragonPos);
			if (DragonNode != nullptr) {
				//FColor solidColor = FColor(0, 0, 255, 32);
				DrawBBox(DragonNode->Bounds, 0, FColor::Black, true);

				TArray<FMyOctreeNode*> Neighbours = MyOctree.GetNeighbours(DragonNode);
				for (int i = 0; i < Neighbours.Num(); i++) {
					DrawBBox(Neighbours[i]->Bounds, 100, FColor::Magenta, false);
				}
			}
		}
	}

	//// color octree root children
	//DrawBBox(MyOctree.Root->Children[4]->Bounds, 0, FColor::Red, true);
	//DrawBBox(MyOctree.Root->Children[5]->Bounds, 0, FColor::Green, true);
	//DrawBBox(MyOctree.Root->Children[6]->Bounds, 0, FColor::Blue, true);
	//DrawBBox(MyOctree.Root->Children[7]->Bounds, 0, FColor::Yellow, true);

	for (int i = 0; i < 20; ++i) {
		//test drawing some lines
		TArray<FVector> Points = {
			{-2300+15*(float)i, 60,       170},
			{-2300+15*(float)i, 60 + 100, 170},
			{-2300+15*(float)i, 60 + 100, 170 + 100},
			{-2300+15*(float)i, 60,       170 + 100},
			{-2300+15*(float)i, 60,       170},
		};
	
		DrawPath(Points);
	}
	
	if (this->bDrawOctree) {
		DrawOctree(this->MyOctree.Root);
	}

	if (this->bDrawEdges) {
		for (int i = 0; i < AStar.Edges.Num(); i++) {
			DrawDebugLine(GetWorld(),
				AStar.Edges[i]->Start->OctreeNode->Bounds.GetCenter(),
				AStar.Edges[i]->End->OctreeNode->Bounds.GetCenter(),
				FColor::Cyan
			);
		}
	}

	if (this->bDrawNodes) {
		for (auto& Elem : AStar.Nodes) {
			FAStarNode* Node = Elem.Value;
			DrawDebugSphere(GetWorld(),
				Node->OctreeNode->Bounds.GetCenter(),
				64,	//radius
				8, //segments
				FColor::Blue
			);
		}
	}


}





