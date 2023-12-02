#include "Dragon.h"
#include "../Utils/LogHelper.h"

// Sets default values
ADragon::ADragon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

FVector InitialWorldPos;
FVector Velocity = { 100, 0, 0 };
float MaxMoveDistance = 200;
// Called when the game starts or when spawned
void ADragon::BeginPlay()
{
	Super::BeginPlay();
	InitialWorldPos = this->GetActorLocation();
	
}


// Called every frame
void ADragon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector displacement = Velocity * DeltaTime;

	FVector newWorldPos = GetActorLocation() + displacement;

	SetActorLocation(newWorldPos);

	float distanceMoved = FVector::Dist(InitialWorldPos, newWorldPos);

	if (distanceMoved > MaxMoveDistance) {
		InitialWorldPos = InitialWorldPos + Velocity.GetSafeNormal() * MaxMoveDistance;	//prevent overshooting
		SetActorLocation(InitialWorldPos);
		Velocity *= -1;

		LogMain << "change direction: " << Velocity.X;
	}

}

