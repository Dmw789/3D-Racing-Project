// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoostPad.generated.h"

// Declarations
class UBoxComponent;

UCLASS()
class LC_VER4_API ABoostPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoostPad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	virtual void SetupBoostPadComponents();
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* padMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* hitbox;
};
