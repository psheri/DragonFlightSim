#include "Dragon.h"
#include "LogHelper.h"

// Sets default values
ADragon::ADragon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

FVector initialWorldPos;
FVector velocity = { 0, 0, 100 };
float maxMoveDistance = 200;
// Called when the game starts or when spawned
void ADragon::BeginPlay()
{
	Super::BeginPlay();
	initialWorldPos = this->GetActorLocation();
	
}


// Called every frame
void ADragon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector displacement = velocity * DeltaTime;

	FVector newWorldPos = GetActorLocation() + displacement;

	SetActorLocation(newWorldPos);

	float distanceMoved = FVector::Dist(initialWorldPos, newWorldPos);

	if (distanceMoved > maxMoveDistance) {
		initialWorldPos = initialWorldPos + velocity.GetSafeNormal() * maxMoveDistance;	//prevent overshooting
		SetActorLocation(initialWorldPos);
		velocity *= -1;

		LogMain << "change direction: " << velocity.Z;
	}

}

