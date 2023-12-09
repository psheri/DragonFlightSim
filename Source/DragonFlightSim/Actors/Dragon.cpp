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
		Navigate();
	}
}


void ADragon::Navigate() {
	if (FlightPathfinder->GetAStarGraph() == nullptr || FlightPathfinder->GetAStarGraph()->Nodes.Num() == 0) {
		return;
	}

}

// Called every frame
void ADragon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//return;
	TArray<FAStarNode*> Path = FlightPathfinder->FindRandomPath();
	LogMain << "Found random path with length " << FlightPathfinder->FindRandomPath().Num();
	FlightPathfinder->DrawFlightPath(Path);

}

