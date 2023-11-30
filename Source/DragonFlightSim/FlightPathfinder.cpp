


#include "FlightPathfinder.h"
#include <Kismet/GameplayStatics.h>

void AFlightPathfinder::DrawBBox(const FBox &bbox)
{
	DrawDebugBox(GetWorld(), bbox.GetCenter(), bbox.GetExtent(), FColor::Emerald);
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
	//octree.SetWorldBounds(this->min, this->max);
	TArray<AActor*> obstacles;
	UGameplayStatics::GetAllActorsWithTag(this->GetWorld(), FName("Obstacle"), obstacles);

	octree = MyOctree(obstacles);
}

// Called every frame
void AFlightPathfinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int i = 0; i < 20; ++i) {
		//test drawing some lines
		TArray<FVector> points = {
			{-2300+15*(float)i, 60,       170},
			{-2300+15*(float)i, 60 + 100, 170},
			{-2300+15*(float)i, 60 + 100, 170 + 100},
			{-2300+15*(float)i, 60,       170 + 100},
			{-2300+15*(float)i, 60,       170},
		};

		DrawPath(points);
	}

	
	DrawBBox(this->octree.WorldBounds);
}



