// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RefillStation.generated.h"

// Declarations
class UBoxComponent;

UCLASS()
class LC_VER4_API ARefillStation : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARefillStation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	virtual void SetupRefillStationComponents();

	// Components
	UPROPERTY(VisibleAnywhere)
	USceneComponent* root;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* stationMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* hitbox;
};
