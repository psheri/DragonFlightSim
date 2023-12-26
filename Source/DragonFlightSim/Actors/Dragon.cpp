#include "Dragon.h"
#include "../Utils/LogHelper.h"
#include <Kismet/GameplayStatics.h>
#include "../FlightPathfinder.h"



// Sets default values
ADragon::ADragon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AFlightPathfinder* FlightPathfinder;
// Called when the game starts or when spawned
void ADragon::BeginPlay()
{
	Super::BeginPlay();

	AActor* ActorFlightPathfinder = UGameplayStatics::GetActorOfClass(this->GetWorld(), AFlightPathfinder::StaticClass());
	if (ActorFlightPathfinder != nullptr) {
		FlightPathfinder = Cast<AFlightPathfinder>(ActorFlightPathfinder);
		LogMain << "Got the FlightPathfinder ref";
	}
}


void ADragon::Navigate(float DeltaTime) {
	if (FlightPathfinder->GetAStarGraph() == nullptr || FlightPathfinder->GetAStarGraph()->Nodes.Num() == 0) {
		return;
	}

	// do we need to generate a new path?
	if (this->CurrentWaypoint >= this->CurrentFlightPath.Num() || this->CurrentFlightPath.Num() == 0) {
		FVector ActorLocation = this->GetActorLocation();
		this->CurrentFlightPath = FlightPathfinder->FindRandomPath(&ActorLocation);
		this->CurrentWaypoint = 0;
		LogMain << "Generated a new path of length " << this->CurrentFlightPath.Num();
	}

	if (this->CurrentFlightPath.Num() == 0) {
		LogMain << "xD";
		return;
	}

	// do we need to increase the waypoint?
	FVector Direction = this->CurrentFlightPath[this->CurrentWaypoint]->GetNode()->Bounds.GetCenter() - this->GetActorLocation();
	float WaypointDistance = Direction.Length();

	if (WaypointDistance <= this->Accuracy) {
		this->CurrentWaypoint++;
		LogMain << "Waypoint: [" << this->CurrentWaypoint << "/" << this->CurrentFlightPath.Num() << "]";
		return;
	}

	// push the actor in the direction of the current waypoint
	Direction.Normalize();
	FRotator TargetRotation = Direction.Rotation();
	FRotator TickRotation = FQuat::Slerp(GetActorRotation().Quaternion(), TargetRotation.Quaternion(), DeltaTime * RotSpeed).Rotator();
	SetActorRotation(TickRotation);
	FVector TickPosition = GetActorLocation() + GetActorForwardVector() * Speed * DeltaTime;
	SetActorLocation(TickPosition);
}

// Called every frame
void ADragon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//return;
	//TArray<FAStarNode*> Path = FlightPathfinder->FindRandomPath();
	//LogMain << "Found random path with length " << FlightPathfinder->FindRandomPath().Num();

	if (bShouldFly)
		this->Navigate(DeltaTime);

	FlightPathfinder->DrawFlightPath(this->CurrentFlightPath);

}

