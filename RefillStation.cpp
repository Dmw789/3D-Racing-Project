// Fill out your copyright notice in the Description page of Project Settings.


#include "RefillStation.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
ARefillStation::ARefillStation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetupRefillStationComponents();
}

// Called when the game starts or when spawned
void ARefillStation::BeginPlay()
{
	Super::BeginPlay();
	stationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	hitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	hitbox->SetGenerateOverlapEvents(true);
	hitbox->SetCollisionProfileName("Pawn");
}

// Called every frame
void ARefillStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARefillStation::SetupRefillStationComponents()
{
	root = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Scene Root")));
	RootComponent = root;
	stationMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Station Mesh")));
	stationMesh->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> staticMeshOb_Station(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (staticMeshOb_Station.Object)
		stationMesh->SetStaticMesh(staticMeshOb_Station.Object);

	static ConstructorHelpers::FObjectFinder<UMaterial> staticMatOb_Station(TEXT("Material'/DatasmithContent/Materials/AliasMaster.AliasMaster'"));
	if (staticMatOb_Station.Object)
		stationMesh->SetMaterial(0, staticMatOb_Station.Object);
	
	stationMesh->SetupAttachment(root);

	hitbox = CreateDefaultSubobject<UBoxComponent>(FName(TEXT("Station Hitbox")));
	hitbox->SetupAttachment(stationMesh);
	
	hitbox->SetBoxExtent(FVector(50.0, 50.0, 150.0));
}