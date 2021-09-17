// Fill out your copyright notice in the Description page of Project Settings.

#include "Ship.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
//#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathVectorConstants.h"
#include "DrawDebugHelpers.h"

#include "Engine/StaticMeshActor.h"
#include "RefillStation.h"
#include "BoostPad.h"
	
// ASSIGN STATIC CONSTS
const int AShip::SPHERE_RADIUS = 96;
const int AShip::SPRING_LENGTH = 1500;
const int AShip::SPRING_ZOFFSET = 200;
const float AShip::SPRING_PITCH = -15.0;
const int AShip::MAX_ENERGY = 100;
const int AShip::MIN_ENERGY = 1;
const int AShip::MAX_SPD = 750;
const int AShip::MAX_DECEL = 50;
const int AShip::MAX_BRAKE = 5;
const int AShip::BOOST_LEN = 180; //in frames
const float AShip::BOOST_COST = 20.0;
const float AShip::MAX_BOOST = 200;
const float AShip::ACCEL_SPD = 2.0;
const float AShip::DECEL_SPD = 0.5;
const float AShip::DECEL_DAMP = 0.25;
const float AShip::BOOST_DAMP = 0.1;
const float AShip::TURN_SPD = 2.0;
const float AShip::MAX_STRAFE_SPD = 75.0;
const float AShip::CAM_SPD_GRIP = 0.1;
const float AShip::CAM_SPD_UNGRIP = 0.01;
const float AShip::CAM_DAMP = 0.75;
const float AShip::GRIP_STR = 1.5;
const float AShip::GRIP_GRADE_BONUS = 0.75;
const float AShip::CONV_AMT = 2.5;
const float AShip::FLOAT_HEIGHT = 150.0;
const float AShip::MIN_FOV_NORM = 60.0;
const float AShip::MAX_FOV_NORM = 90.0;
const float AShip::MAX_FOV_BOOST = 120.0;
const float AShip::TURN_ROLL = 15.0;
const float AShip::STRAFE_ROLL = 30.0;
const float AShip::LEAN_SPD = 0.75;
const float AShip::REFILL_AMT = 1;

// UActor Events
AShip::AShip()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	SetupShipComponents();
	SetupVariables();
}

void AShip::BeginPlay()
{
	Super::BeginPlay();
	SetupCollision();
}

void AShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CalculateHealth();
	CalculateSpeed();
	CalculateBoost();
	CalculateGravity();
	CalculateLocation();
	CalculateFov();
	OrientShipAxis();
	OrientCamAxis();
	OrientShipMesh();
}

// Initialization
void AShip::SetupShipComponents()
{
	// intializing components
	root = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Scene Root")));
	RootComponent = root;
	
	shipAxis = CreateDefaultSubobject<USphereComponent>(FName(TEXT("Ship Axis")));
	shipAxis->SetupAttachment(root);
	shipAxis->SetMobility(EComponentMobility::Movable);
	shipAxis->SetSphereRadius(96);

	shipAxisViewer = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Ship Axis Viewer")));
	shipAxisViewer->SetupAttachment(shipAxis);
	shipAxisViewer->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> staticMeshOb_ShipView(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (staticMeshOb_ShipView.Object)
		shipAxisViewer->SetStaticMesh(staticMeshOb_ShipView.Object);
	
	shipMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Ship Mesh")));
	shipMesh->SetupAttachment(root);
	shipMesh->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> staticMeshOb_Ship(TEXT("StaticMesh'/Game/Ship/Assets/Meshes/ME_shipMesh.Me_shipMesh'"));
	if (staticMeshOb_Ship.Object)
		shipMesh->SetStaticMesh(staticMeshOb_Ship.Object);
	static ConstructorHelpers::FObjectFinder<UMaterial> staticMatOb_Ship(TEXT("Material'/Game/Ship/Assets/Materials/M_Ship_Boost.M_Ship_Boost'"));
	if (staticMatOb_Ship.Object)
		shipMesh->SetMaterial(0, staticMatOb_Ship.Object);

	camAxis = CreateDefaultSubobject<USphereComponent>(FName(TEXT("Camera Axis")));
	camAxis->SetupAttachment(root);
	camAxis->SetMobility(EComponentMobility::Movable);
	camAxis->SetSphereRadius(96);
	
	spring = CreateDefaultSubobject<USpringArmComponent>(FName(TEXT("Spring Arm")));
	spring->SetupAttachment(camAxis);
	spring->TargetArmLength = SPRING_LENGTH;
	spring->bDoCollisionTest = false;
	spring->bInheritPitch = true;
	spring->bInheritRoll = true;
	spring->bInheritYaw = true;
	spring->SetRelativeRotation(FRotator(SPRING_PITCH, 0, 0));
	spring->SetRelativeLocation(FVector(0,0,SPRING_ZOFFSET));
	
	camera = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Ship Camera")));
	camera->PostProcessSettings.bOverride_MotionBlurAmount = 0.0f;
	camera->SetupAttachment(spring);

	shipAxis->SetHiddenInGame(true);
	shipAxisViewer->SetHiddenInGame(true);
	camAxis->SetHiddenInGame(true);
	shipMesh->SetHiddenInGame(false);
}

void AShip::SetupVariables()
{
	currEnergy = MAX_ENERGY;
	goToEnergy = MAX_ENERGY;
	currFov = MIN_FOV_NORM;
}

void AShip::SetupCollision()
{
	shipAxis->OnComponentBeginOverlap.AddDynamic(this, &AShip::OnShipAxisOverlapBegin);
	shipAxis->OnComponentEndOverlap.AddDynamic(this, &AShip::OnShipAxisOverlapEnd);
}

// input functions
void AShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Accelerate", this, &AShip::Accelerate);
	PlayerInputComponent->BindAxis("Turn", this, &AShip::Turn);
	PlayerInputComponent->BindAxis("Lean", this, &AShip::Lean);
	PlayerInputComponent->BindAxis("Strafe", this, &AShip::Strafe);
	PlayerInputComponent->BindAction("DebugToggle", IE_Pressed, this, &AShip::DebugTogglePressed);
	PlayerInputComponent->BindAction("Brake", IE_Pressed, this, &AShip::BrakePressed);
	PlayerInputComponent->BindAction("Brake", IE_Released, this, &AShip::BrakeReleased);
	PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &AShip::BoostPressed);
}

void AShip::Accelerate(float axisValue)
{
	spdAccel = axisValue * ACCEL_SPD;
}

void AShip::Turn(float axisValue)
{
	CalculateRotation(axisValue);
	CalculateGrip(axisValue);
	turnRollAmt = TURN_ROLL * axisValue;
}

void AShip::Lean(float axisValue)
{
	FRotator rot;
	
	if(currState == ShipStates::AIRBORNE)
	{
		// Rotation for turning
		rot = FRotator(LEAN_SPD * axisValue, 0.0f, 0.0f); // magic num
	
		shipAxis->SetWorldRotation(shipAxis->GetComponentRotation().Quaternion() * rot.Quaternion());
	}
}

void AShip::Strafe(float axisValue)
{
	currStrafeSpd = axisValue * MAX_STRAFE_SPD;
	strafeRollAmt = STRAFE_ROLL * axisValue;
}

void AShip::DebugTogglePressed()
{
	debugOn = !debugOn;
	shipAxis->SetHiddenInGame(!debugOn);
	shipAxisViewer->SetHiddenInGame(!debugOn);
	camAxis->SetHiddenInGame(!debugOn);
	shipMesh->SetHiddenInGame(debugOn);
}

void AShip::BrakePressed()
{
	currBrake = MAX_BRAKE;
}

void AShip::BrakeReleased()
{
	currBrake = 0;
}

void AShip::BoostPressed()
{
	if (!boosting && currEnergy - BOOST_COST > 0)
	{
		boosting = true;
		if (!refilling)
		{
			currEnergy = currEnergy - BOOST_COST;
		}
	}
}

// Collision

FVector AShip::GetImpactNormal(AActor* otherActor)
{
	FVector norm = GetActorLocation();
	TArray<FHitResult> outHits;
	FComponentQueryParams params;
	params.AddIgnoredActor(this);
	UWorld* const world = GetWorld();
	world->ComponentSweepMulti(outHits, shipAxis, shipAxis->GetComponentLocation() + FVector(-.1f), shipAxis->GetComponentLocation() + FVector(0.1f), shipAxis->GetComponentRotation(), params);

	for(const FHitResult& Result : outHits)
	{
		if(Result.GetActor() == otherActor)
		{
			norm = Result.ImpactNormal;
		}
	}

	return norm;
}

void AShip::OnShipAxisOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool  bFromSweep, const FHitResult& SweepResult)
{
	FVector hitNorm, hitDir;
	
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		if (OtherActor->IsA(ARefillStation::StaticClass())) // CHECKING REFILL STATION
		{
			refilling = true;
		}
		else if(OtherActor->IsA(ABoostPad::StaticClass()))
		{
			boosting = true;
		}
	}
}

void AShip::OnShipAxisOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		if (OtherActor->IsA(ARefillStation::StaticClass())) // CHECKING REFILL STATION
		{
			refilling = false;
		}
	}
}	

bool AShip::WillOverlap(FVector loc)
{
	bool willOverlap = false;
	TArray<FHitResult> outHits;
	FComponentQueryParams params;
	params.AddIgnoredActor(this);
	UWorld* const world = GetWorld();
//	while()
//	{
		world->ComponentSweepMulti(outHits, shipAxis, loc + FVector(-.1f), loc + FVector(-.1f), shipAxis->GetComponentRotation(), params);
		for(const FHitResult& Result : outHits)
		{		
			if(Result.GetComponent())
			{
				UStaticMeshComponent* compMesh = dynamic_cast<UStaticMeshComponent*>(Result.GetComponent());
				if(compMesh)
				{
					FString name = compMesh->GetStaticMesh()->GetFName().ToString();
					if(name == FString("leftRail") || name == FString("rightRail"))
					{	
						willOverlap = true;
					}
				}
			}
		}
//	}
	return willOverlap;
}

// Processes
void AShip::CalculateHealth()
{
	if (refilling)
	{
		currEnergy = FMath::Clamp((currEnergy + REFILL_AMT), 0.0f, float(MAX_ENERGY));
	}
}

void AShip::CalculateSpeed()
{
	float alpha = (DECEL_SPD + currBrake) / MAX_DECEL * DECEL_DAMP;

	normSpd = FMath::Lerp(normSpd, 0.0f, alpha);
	normSpd = FMath::Clamp((normSpd + spdAccel), 0.0f, float(MAX_SPD));
	currSpd = FMath::Clamp((normSpd + boostSpd), 0.0f, float(MAX_SPD + boostSpd));
}

void AShip::CalculateLocation()
{
	FVector dir;
	
	if(currState == ShipStates::GRIP || currState == ShipStates::AIRBORNE)
	{
		if(!WillOverlap(shipAxis->GetComponentLocation() + shipAxis->GetForwardVector() * currSpd / CONV_AMT))
		{
			AddActorWorldOffset(shipAxis->GetForwardVector() * currSpd / CONV_AMT, true);
		}
		if(!WillOverlap(shipAxis->GetComponentLocation() + shipAxis->GetRightVector() * currStrafeSpd / CONV_AMT))
		{
			AddActorWorldOffset(shipAxis->GetRightVector() * currStrafeSpd / CONV_AMT, true);
		}
	}
	else if(currState == ShipStates::UNGRIP)
	{
		//if(!WillOverlap(shipAxis->GetComponentLocation() + shipAxis->GetForwardVector() * currSpd / CONV_AMT))
		//{
			dir = (shipAxis->GetForwardVector() + camAxis->GetForwardVector()) * 0.5;
			AddActorWorldOffset(dir * currSpd / CONV_AMT, true);
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + dir*750, FColor::Red, false, 0.01f);
		//}
	}
}

void AShip::CalculateRotation(float axisValue) 
{
	FRotator rot;

	// Rotation for turning
	rot = FRotator(0.0f, TURN_SPD * axisValue, 0.0f);
	
	shipAxis->SetWorldRotation(shipAxis->GetComponentRotation().Quaternion() * rot.Quaternion());
}

void AShip::CalculateBoost()
{
	float boostAlpha;
	
	if (boosting)
	{
		boostAlpha = (normSpd / MAX_SPD) * BOOST_DAMP;
		boostSpd = FMath::Lerp(boostSpd, MAX_BOOST, boostAlpha);

		if (boostIter < BOOST_LEN)
		{
			++boostIter;
		}
		else
		{
			boostIter = 0;
			boosting = false;
		}
	}
	else
	{
		boostAlpha = (DECEL_SPD + currBrake) / MAX_DECEL * DECEL_DAMP;
		boostSpd = FMath::Lerp(boostSpd, 0.0f, boostAlpha);
	}
}

void AShip::CalculateGravity()
{
	float tv; // terminal velocity -- currently inactive
	if(currState == ShipStates::AIRBORNE)
	{
		tv = 0;
	
		currGrvForce = FMath::Lerp(currGrvForce, -300.0f, .005f);
		
		AddActorWorldOffset(root->GetUpVector() * currGrvForce, true);
	}
	else
	{
		currGrvForce = -25;
	}
}

void AShip::CalculateGrip(float axisValue)
{
	/*
	float angle;
	
	angle = shipAxis->GetRelativeRotation().Yaw - camAxis->GetRelativeRotation().Yaw;
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Angle: %f"), angle));
	
	if(currState == ShipStates::GRIP)
	{
		if(FMath::Abs(angle) > 15)
		{
			//currState = ShipStates::UNGRIP;
		}
	}
	else if(currState == ShipStates::UNGRIP)
	{
		// check if within an acceptable limit for a period of time
	}
	*/
	//use pitch rotations instead of forward vectors?
	FVector axis, aV, bV;
	float aM, bM, angle, gripStr;
	
	aV = shipAxis->GetForwardVector();
	bV = camAxis->GetForwardVector();
	aM = aV.Size();
	bM = bV.Size();
	angle = FMath::Acos(FVector::DotProduct(aV, bV) / (aM * bM)) * 180/PI;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("SaxFv: %f, %f, %f"), shipAxis->GetForwardVector().X, shipAxis->GetForwardVector().Y, shipAxis->GetForwardVector().Z));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("CamFv: %f, %f, %f"), camera->GetForwardVector().X, camera->GetForwardVector().Y, camera->GetForwardVector().Z));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Angle: %f"), angle));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("SaxFv: %f, %f, %f"), aV.X, aV.Y, aV.Z));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("CamFv: %f, %f, %f"), camera->GetForwardVector().X, camera->GetForwardVector().Y, camera->GetForwardVector().Z));
	
	
	if(currState == ShipStates::GRIP)
	{
		gripStr = 1 - currSpd/MAX_SPD + GRIP_GRADE_BONUS;
		
		if(FMath::Abs(axisValue) > gripStr)
		{
			//currState = ShipStates::UNGRIP;
		}
	}
	else if(currState == ShipStates::UNGRIP)
	{
		if(FMath::Abs(angle) < 20)
		{
			currState = ShipStates::GRIP;
		}
	}
}

void AShip::CalculateFov()
{
	// need to change to reflect the total speed of the system, not just the currSpd
	float targetFov;

	//targetFov = MAX_FOV_NORM * currSpd / MAX_SPD;
	//targetFov = FMath::Clamp(targetFov, MIN_FOV_NORM, MAX_FOV_BOOST);
	
	targetFov = MIN_FOV_NORM + 30 * currSpd/(MAX_SPD/2);
	targetFov = FMath::Clamp(targetFov, MIN_FOV_NORM, MAX_FOV_NORM);
	targetFov = targetFov + 30 * boostSpd/(MAX_BOOST/2);
	targetFov = FMath::Clamp(targetFov, MIN_FOV_NORM, MAX_FOV_BOOST);
	
	currFov = FMath::Lerp(currFov, targetFov, 0.05f);
	camera->SetFieldOfView(currFov);
}

void AShip::OrientShipAxis()
{
	FQuat deltaRot;
	FVector axis, start, end;
	FHitResult hit;
	FCollisionQueryParams traceParams;
	float angle;

	start = shipAxis->GetComponentTransform().GetTranslation();
	end = start - (shipAxis->GetUpVector() * 250); // magic num

	traceParams.bTraceComplex = true;
	GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Visibility, traceParams);

	if (debugOn)
	{
		DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 0.01f);
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + shipAxis->GetForwardVector() * 500, FColor::Blue, false, 0.01f);
	}
	
	if (hit.bBlockingHit)
	{
		if(currState == ShipStates::AIRBORNE) // if previously airborne
		{
			currState = ShipStates::GRIP;
		}
		
		axis = FVector::CrossProduct(shipAxis->GetUpVector(), hit.ImpactNormal).GetSafeNormal();
		angle = FMath::Acos(FVector::DotProduct(shipAxis->GetUpVector(), hit.ImpactNormal));
		deltaRot = FQuat(axis, angle);
		
		//shipAxis->AddRelativeRotation(deltaRot);
		shipAxis->SetWorldRotation(deltaRot * shipAxis->GetComponentRotation().Quaternion());
		root->SetWorldLocation(hit.Location + shipAxis->GetUpVector() * 100, true); //magic num
	}
	else
	{
		currState = ShipStates::AIRBORNE;
	}
}

void AShip::OrientCamAxis()
{
	FQuat applyRot, targetRot, currRot;
	float alpha;

	currRot = camAxis->GetComponentRotation().Quaternion();
	targetRot = shipAxis->GetComponentRotation().Quaternion();
	alpha = currSpd / MAX_SPD * CAM_DAMP;
	
	if(currState == ShipStates::GRIP || currState == ShipStates::AIRBORNE)
	{
		alpha = FMath::Clamp(alpha, 0.0f, CAM_SPD_GRIP);
	}
	else if(currState == ShipStates::UNGRIP)
	{
		alpha = FMath::Clamp(alpha, 0.0f, CAM_SPD_UNGRIP);
	}
	
	applyRot = FQuat::Slerp(currRot, targetRot, alpha);
	camAxis->SetWorldRotation(applyRot);
}

void AShip::OrientShipMesh()
{
	FQuat applyRot, targetRot, currRot, addRoll;
	float alpha;
	
	// Getting standard rotation
	currRot = shipMesh->GetComponentRotation().Quaternion();
	targetRot = shipAxis->GetComponentRotation().Quaternion();
	alpha = 1.0;
	applyRot = FQuat::Slerp(currRot, targetRot, alpha);
	
	targetRot = FRotator(0.0f,0.0f,turnRollAmt + strafeRollAmt).Quaternion();
	shipMesh->SetWorldRotation(applyRot * targetRot);
	
	//If you want to add this, you'll need to solve -- Interpolation above will always try to fix this roll which could lead to some undesired looking animation or slow interpolation. Instead maybe save the "actual rotation".
	// Make it so that it takes in to account turn axis input.

}

// Getters
float AShip::GetCurrEnergy()
{
	return currEnergy;
}

float AShip::GetMaxEnergy()
{
	return MAX_ENERGY;
}

float AShip::GetCurrSpd()
{
	return currSpd;
}

float AShip::GetBoostSpd()
{
	return boostSpd;
}

float AShip::GetNormSpd()
{
	return normSpd;
}

bool AShip::GetDebugOn()
{
	return debugOn;
}