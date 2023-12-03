

#pragma once
#include "../AStar/FAStarNode.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dragon.generated.h"

UCLASS()
class DRAGONFLIGHTSIM_API ADragon : public AActor
{
	GENERATED_BODY()
	
public:	
	float Speed = 50.0f;
	float Accuracy = 100.0f;
	float RotSpeed = 50.0f;
	int CurrentWaypoint = 0;
	
	TArray<FAStarNode*> CurrentFlightPath;

	void Navigate();

	ADragon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
