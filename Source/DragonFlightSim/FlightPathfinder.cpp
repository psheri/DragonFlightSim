


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

// Called when the game starts or when spawned
void AFlightPathfinder::BeginPlay()
{
	Super::BeginPlay();
	//this->MyOctree.SetWorldBounds(this->min, this->max);
	TArray<AActor*> Obstacles;
	UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), FName("Obstacle"), Obstacles);

	this->MyOctree = FMyOctree(Obstacles, &AStar);
	this->MyOctree.Build();
}

void AFlightPathfinder::DrawOctree(FMyOctreeNode* CurrentNode) {
	
	DrawBBox(CurrentNode->Bounds);

	for (AActor* Obstacle : CurrentNode->ContainedActors) {
		//LogMain << "Here";
		DrawBBox(Obstacle->GetComponentByClass<UStaticMeshComponent>()->Bounds.GetBox(), 5, FColor::Magenta);
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

// Called every frame
void AFlightPathfinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	
	DrawBBox(this->MyOctree.WorldBounds, 10);
	//DrawOctree(this->MyOctree.Root);
	
	for (int i = 0; i < AStar.Edges.Num(); i++) {
		DrawDebugLine(GetWorld(),
			AStar.Edges[i]->Start->OctreeNode->Bounds.GetCenter(),
			AStar.Edges[i]->End->OctreeNode->Bounds.GetCenter(),
			FColor::Cyan
		);
	}
	//LogMain << "@AStar  Nodes.Num() = " << Nodes.Num();
	for (int i = 0; i < AStar.Nodes.Num(); i++) {
		DrawDebugSphere(GetWorld(),
			AStar.Nodes[i]->OctreeNode->Bounds.GetCenter(),
			64,	//radius
			8, //segments
			FColor::Blue
		);
	}

}





