// Fill out your copyright notice in the Description page of Project Settings.

#include "BoostPad.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
ABoostPad::ABoostPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetupBoostPadComponents();
}

// Called when the game starts or when spawned
void ABoostPad::BeginPlay()
{
	Super::BeginPlay();
	//padMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//hitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//hitbox->SetGenerateOverlapEvents(true);
	//hitbox->SetCollisionProfileName("Pawn");
}

// Called every frame
void ABoostPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABoostPad::SetupBoostPadComponents()
{
	padMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Boost Pad Mesh")));
	padMesh->SetMobility(EComponentMobility::Movable);
	RootComponent = padMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> staticMeshOb_Pad(TEXT("StaticMesh'/Game/Props/Assets/Meshes/ME_Boost.ME_Boost'"));
	if (staticMeshOb_Pad.Object)
		padMesh->SetStaticMesh(staticMeshOb_Pad.Object);

	static ConstructorHelpers::FObjectFinder<UMaterial> staticMatOb_Pad(TEXT("Material'/Game/Props/Assets/Materials/M_Boost.M_Boost'"));
	if (staticMatOb_Pad.Object)
		padMesh->SetMaterial(0, staticMatOb_Pad.Object);

	hitbox = CreateDefaultSubobject<UBoxComponent>(FName(TEXT("Boost Pad Hitbox")));
	hitbox->SetupAttachment(padMesh);
	
	hitbox->SetBoxExtent(FVector(1000.0, 500.0, 200.0));
}