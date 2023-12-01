


#include "FlightPathfinder.h"
#include <Kismet/GameplayStatics.h>

void AFlightPathfinder::DrawBBox(const FBox &BBox, float Thickness)
{
	DrawDebugBox(GetWorld(), BBox.GetCenter(), BBox.GetExtent(), FColor::Emerald, false, -1, 0U, Thickness);
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

	this->MyOctree = FMyOctree(Obstacles);
	this->MyOctree.Build();
}

void AFlightPathfinder::DrawOctree(FOctreeNode* CurrentNode) {
	DrawBBox(CurrentNode->Bounds);
	if (CurrentNode->Children != nullptr) {
		for (int i = 0; i < 8; i++) {
			if (CurrentNode->Children[i] != nullptr) {
				DrawOctree(CurrentNode->Children[i]);
			}
		}
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
	DrawOctree(this->MyOctree.Root);

}





