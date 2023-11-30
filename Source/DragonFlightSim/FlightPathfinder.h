

#pragma once
#include "MyOctree.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlightPathfinder.generated.h"


UCLASS()
class DRAGONFLIGHTSIM_API AFlightPathfinder : public AActor
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, Category = "Initial World Bounds")
	//FVector min;
	//
	//UPROPERTY(EditAnywhere, Category = "Initial World Bounds")
	//FVector max;

private:
	virtual void DrawBBox(const FBox &bbox);
public:	

	MyOctree octree;
	virtual void DrawPath(TArray<FVector>& points);
	// Sets default values for this actor's properties
	AFlightPathfinder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
