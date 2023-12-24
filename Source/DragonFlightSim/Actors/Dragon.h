

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Params")
	float Speed = 12000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Params")
	float Accuracy = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Params")
	float RotSpeed = 50.0f;

	int CurrentWaypoint = 0;
	
	TArray<FAStarNode*> CurrentFlightPath;

	void Navigate(float DeltaTime);

	ADragon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
