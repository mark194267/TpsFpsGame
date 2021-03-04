// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ThirdPersonMPCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"
#include "ThirdPersonMPProjectile.h"
#include "UObject/ConstructorHelpers.h"

//////////////////////////////////////////////////////////////////////////
// AThirdPersonMPCharacter

AThirdPersonMPCharacter::AThirdPersonMPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetActive(false);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	//Initialize projectile class
	//if(ProjectileClass == NULL)
	//	ProjectileClass = AThirdPersonMPProjectile::StaticClass();
	//Initialize fire rate
	FireRate = 0.25f;
	bIsFiringWeapon = false;
	
	/*FPS section*/

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPPmesh"));
	FPSMesh->SetOnlyOwnerSee(true);
	FPSMesh->SetOwnerNoSee(false);
	FPSMesh->SetupAttachment(FirstPersonCameraComponent);
	FPSMesh->bCastDynamicShadow = false;
	FPSMesh->CastShadow = false;
	//FPSMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	//FPSMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	FPSMesh->SetVisibility(false);

	//Create A "FPSGun"
	//this->FPSGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSGun"));
	//this->SetRootComponent(this->FP_FPSGun);
	//static ConstructorHelpers::FObjectFinder<UStaticMesh>CubeMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	//this->FP_FPSGun->SetStaticMesh(CubeMeshAsset.Object);
	//FP_FPSGun->SetRelativeLocation
	//FPSGun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	//FPSGun->bCastDynamicShadow = false;
	//FPSGun->CastShadow = false;
	//FPSGun->SetupAttachment(FPSMesh);
	//FVector transVec = FVector(1.f, .3f, .3f);
	//FPSGun->RelativeScale3D = transVec;
	//FPSGun->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 0.f), FQuat(FVector(0.f, 0.f, 0.f), 0.f));
	//FPSGun->SetVisibility(false);
	//ApplyMuzzleToFP_FPSGun
	//FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	//FP_MuzzleLocation->SetupAttachment(FPSGun);
	//FP_MuzzleLocation->SetRelativeLocation(FVector(1.f, 0.f, 0.f));

}

//////////////////////////////////////////////////////////////////////////
// Input

void AThirdPersonMPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AThirdPersonMPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AThirdPersonMPCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AThirdPersonMPCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AThirdPersonMPCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AThirdPersonMPCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AThirdPersonMPCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AThirdPersonMPCharacter::OnResetVR);
	// Handle firing projectiles
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AThirdPersonMPCharacter::StartFire);
	PlayerInputComponent->BindAction("ChangeView", IE_Pressed, this, &AThirdPersonMPCharacter::ChangeViewPress);

}


void AThirdPersonMPCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AThirdPersonMPCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AThirdPersonMPCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AThirdPersonMPCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AThirdPersonMPCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AThirdPersonMPCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AThirdPersonMPCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

//////////////////////////////////////////////////////////////////////////
// Replicated Properties

void AThirdPersonMPCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AThirdPersonMPCharacter, CurrentHealth);
}
void AThirdPersonMPCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (Role == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}
void AThirdPersonMPCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}
void AThirdPersonMPCharacter::SetCurrentHealth(float healthValue)
{
	if (Role == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}
float AThirdPersonMPCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}
//////////////////////////////////////////////////
//////Fire
void AThirdPersonMPCharacter::StartFire_Implementation()
{
	if (!bIsFiringWeapon)
	{
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &AThirdPersonMPCharacter::StopFire, FireRate, false);
		//World->GetTimerManager().SetTimer(FiringTimer, this, &AThirdPersonMPCharacter::StopFire, FireRate, false);
		HandleFire_Implementation();

	}
}
void AThirdPersonMPCharacter::StopFire()
{
	bIsFiringWeapon = false;
}
/*Unreal Server rep auto suffix*/
void AThirdPersonMPCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + (GetControlRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetControlRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = Instigator;
	spawnParameters.Owner = this;

	AThirdPersonMPProjectile* spawnedProjectile = GetWorld()->SpawnActor<AThirdPersonMPProjectile>(spawnLocation, spawnRotation, spawnParameters);
}
void AThirdPersonMPCharacter::ChangeViewPress()
{
	Is3rdPerson = !Is3rdPerson;
	ChangeCam();
}
void AThirdPersonMPCharacter::ChangeCam() 
{
	class USkeletalMeshComponent* MyMesh = this->GetMesh();

	// 3rd Person View
	if (Is3rdPerson) 
	{
		// Toggle 3rd cam
		FollowCamera->SetActive(true);
		// Toggle 1st cam
		FirstPersonCameraComponent->SetActive(false);
		// Stop only owner see Mesh(full body)
		MyMesh->SetOwnerNoSee(false);
		// Disable 1st person hand 
		FPSMesh->SetVisibility(false);
		// ...and FPSGun
		//FPSGun->SetVisibility(false);
		// Change Bullet Create position
		// ......
	}
	// First Person View
	else
	{
		// Inactive 3rd cam
		FollowCamera->SetActive(false);
		// Active 1st cam
		FirstPersonCameraComponent->SetActive(true);
		// Set only owner not see mesh
		MyMesh->SetOwnerNoSee(true);
		// Enable 1st person hand
		FPSMesh->SetVisibility(true);
		// ...and FPSGun
		//FPSGun->SetVisibility(true);
		// Change Bullet Create position
		// ......
	}
}