


#include "FlightPathfinder.h"

void AFlightPathfinder::DrawWorldBounds()
{
	DrawDebugBox(GetWorld(), this->octree.WorldBounds.GetCenter(), this->octree.WorldBounds.GetExtent(), FColor::Emerald);
}

void AFlightPathfinder::DrawPath(TArray<FVector>& points)
{
	for (int i = 0; i < points.Num()-1; ++i) {
		FVector& startPos = points[i];
		FVector& endPos = points[i+1];
		DrawDebugLine(GetWorld(), startPos, endPos, FColor::Magenta, false);
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
	octree.SetWorldBounds(this->min, this->max);
}

// Called every frame
void AFlightPathfinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//test drawing some lines
	TArray<FVector> points = { 
		{-2240, 60,       170},
		{-2240, 60 + 100, 170},
		{-2240, 60 + 100, 170 + 100},
		{-2240, 60,       170 + 100},
		{-2240, 60,       170},
	};
	DrawPath(points);
	DrawWorldBounds();
}



