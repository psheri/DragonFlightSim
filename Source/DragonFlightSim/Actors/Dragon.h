

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dragon.generated.h"

UCLASS()
class DRAGONFLIGHTSIM_API ADragon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADragon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
