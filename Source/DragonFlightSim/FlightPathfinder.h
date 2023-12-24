

#pragma once
#include "MyOctree/FMyOctree.h"
#include "AStar/FAStar.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlightPathfinder.generated.h"

UCLASS()
class DRAGONFLIGHTSIM_API AFlightPathfinder : public AActor
{
	GENERATED_BODY()
	FMyOctree MyOctree;
	FAStar AStar;
public:	

	virtual void DrawPath(TArray<FVector>& Points);
	// Sets default values for this actor's properties
	AFlightPathfinder();

	FAStar* GetAStarGraph() { return &AStar; }

	TArray<FAStarNode*> FindPath(FVector StartPos, FVector EndPos);
	TArray<FAStarNode*> FindRandomPath(const FVector* OverrideStartPos = nullptr);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void DrawBBox(const FBox& BBox, float Thickness = 0, FColor Color = FColor::Emerald, bool bIsSolid = false);
	virtual void DrawOctree(FMyOctreeNode* CurrentNode);
	virtual void DrawFlightPath(TArray<FAStarNode*> Path);

};
