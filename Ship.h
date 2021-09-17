// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Ship.generated.h"

// Declarations
class UCameraComponent;
class USpringArmComponent;
class USphereComponent;
class UArrowComponent;


UCLASS()
class LC_VER4_API AShip : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Ship States
	enum ShipStates{INACTIVE, GRIP, UNGRIP, AIRBORNE};

	// UActor Events
	virtual void Tick(float DeltaTime) override;
	
	// Input
	virtual void Accelerate(float axisValue);
	virtual void Turn(float axisValue);
	virtual void Lean(float axisValue);
	virtual void Strafe(float axisValue);
	virtual void DebugTogglePressed();
	virtual void BrakePressed();
	virtual void BrakeReleased();
	virtual void BoostPressed();

	// Getters
	UFUNCTION(BlueprintPure)
	virtual float GetCurrEnergy();

	UFUNCTION(BlueprintPure)
	virtual float GetMaxEnergy();
	
	UFUNCTION(BlueprintPure)
	virtual float GetCurrSpd();
	
	UFUNCTION(BlueprintPure)
	virtual float GetBoostSpd();
	
	UFUNCTION(BlueprintPure)
	virtual float GetNormSpd();
	
	UFUNCTION(BlueprintPure)
	virtual bool GetDebugOn();
	
private:
	// Initialization
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void SetupShipComponents();
	virtual void SetupVariables();
	virtual void SetupCollision();
	
	// Collision
	virtual FVector GetImpactNormal(AActor* otherActor);
	UFUNCTION()
	virtual void OnShipAxisOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnShipAxisOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	virtual bool WillOverlap(FVector loc);
		
	// Processes
	virtual void CalculateHealth();
	virtual void CalculateSpeed();
	virtual void CalculateLocation();
	virtual void CalculateRotation(float axisValue);
	virtual void CalculateBoost();
	virtual void CalculateGravity();
	virtual void CalculateGrip(float axisValue);
	virtual void CalculateFov();
	virtual void OrientShipAxis();
	virtual void OrientCamAxis();
	virtual void OrientShipMesh();
	
	// Helpers

	// Components
	UPROPERTY(VisibleAnywhere)
	USceneComponent* root;
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* shipAxis;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* shipAxisViewer;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* shipMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* camAxis;
	
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* spring;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* camera;
	
	// Component variables
	static const int SPHERE_RADIUS;
	static const int SPRING_LENGTH;
	static const float SPRING_PITCH;
	static const int SPRING_ZOFFSET;

	// Process constants  // REORGANIZE THESE
	static const int MAX_ENERGY;
	static const int MIN_ENERGY;
	static const int MAX_SPD;
	static const int MAX_DECEL;
	static const int MAX_BRAKE;
	static const int BOOST_LEN;
	static const float BOOST_COST;
	static const float MAX_BOOST;
	static const float ACCEL_SPD;
	static const float DECEL_SPD;
	static const float DECEL_DAMP;
	static const float BOOST_DAMP;
	static const float TURN_SPD;
	static const float MAX_STRAFE_SPD;
	static const float CAM_SPD_GRIP;
	static const float CAM_SPD_UNGRIP;
	static const float CAM_DAMP;
	static const float GRIP_STR;
	static const float GRIP_GRADE_BONUS;
	static const float CONV_AMT;
	static const float FLOAT_HEIGHT;
	static const float MIN_FOV_NORM;
	static const float MAX_FOV_NORM;
	static const float MAX_FOV_BOOST;
	static const float TURN_ROLL;
	static const float STRAFE_ROLL;
	static const float LEAN_SPD;
	static const float REFILL_AMT;
	
	
	// Process variables REORGANIZE
	bool debugOn = false;
	
	float currEnergy = 0.0;
	float goToEnergy = 0.0;
	float currSpd = 0.0;
	float spdAccel = 0.0;
	float normSpd = 0.0;
	float currStrafeSpd = 0.0;
	float strafeRollAmt = 0.0;
	float turnRollAmt = 0.0;
	float currBrake = 0.0;
	bool boosting = false;
	int boostIter = 0;
	float boostSpd = 0.0;
	//float currGrip = 0.0;
	float currFov = 0.0;
	float currGrvForce = 0.0;
	bool refilling = false;
	ShipStates currState = ShipStates::GRIP;
};
