// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMainCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
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

// Sets default values
AMyMainCharacter::AMyMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPPmesh"));
	FPSMesh->SetOnlyOwnerSee(true);
	FPSMesh->SetOwnerNoSee(false);
	FPSMesh->SetupAttachment(FollowCamera);
	FPSMesh->bCastDynamicShadow = false;
	FPSMesh->CastShadow = false;
	FPSMesh->SetVisibility(false);

	//Set muzzleLocation on GunMesh
	SkeletalMeshMuzzle = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshMuzzle"));
	SkeletalMeshMuzzle->SetupAttachment(this->GetMesh(), "GunSocket");

}

// Called when the game starts or when spawned
void AMyMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyMainCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	// Handle firing projectiles
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyMainCharacter::StartFire_Auto);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMyMainCharacter::StopFire_Auto);

	PlayerInputComponent->BindAction("ChangeView", IE_Pressed, this, &AMyMainCharacter::ChangeViewPress);

}

void AMyMainCharacter::MoveForward(float Value)
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

void AMyMainCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
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

void AMyMainCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AMyMainCharacter, CurrentHealth);
}
void AMyMainCharacter::OnHealthUpdate()
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
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}
void AMyMainCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}
void AMyMainCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}
float AMyMainCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}
//////////////////////////////////////////////////
//////Fire
void AMyMainCharacter::StartFire_Implementation()
{
	if (!bIsFiringWeapon)
	{
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &AMyMainCharacter::StopFire, FireRate, false);
		//World->GetTimerManager().SetTimer(FiringTimer, this, &AMyMainCharacter::StopFire, FireRate, false);
		HandleFire_Implementation();

	}
}
void AMyMainCharacter::StopFire()
{
	bIsFiringWeapon = false;
}
/*Unreal Server rep auto suffix*/
void AMyMainCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = SkeletalMeshMuzzle->GetComponentLocation() + GetControlRotation().RotateVector(GunOffset);//GetActorLocation() + (GetControlRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetControlRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = Instigator;
	spawnParameters.Owner = this;

	AThirdPersonMPProjectile* spawnedProjectile = GetWorld()->SpawnActor<AThirdPersonMPProjectile>(spawnLocation, spawnRotation, spawnParameters);
}
void AMyMainCharacter::ChangeViewPress()
{
	Is3rdPerson = !Is3rdPerson;
	ChangeCam();
}
void AMyMainCharacter::ChangeCam()
{
	class USkeletalMeshComponent* MyMesh = this->GetMesh();

	// 3rd Person View
	if (Is3rdPerson)
	{
		//change cam position
		CameraBoom->SocketOffset.Z = TPPSocketOffSetZ;
		CameraBoom->TargetArmLength = TPPSocketLenght;
		// Stop only owner see Mesh(full body)
		MyMesh->SetOwnerNoSee(false);
		// Disable 1st person hand
		FPSMesh->SetVisibility(false,true);
		// ...and FPSGun
		//FPSGun->SetVisibility(false);
		// Change Bullet Create position
		SkeletalMeshMuzzle->AttachToComponent(this->GetMesh(),FAttachmentTransformRules::KeepRelativeTransform, "GunSocket");
		GunOffset = TPPGunOffset;
		// ......
	}
	// First Person View
	else
	{
		//change cam position
		CameraBoom->SocketOffset.Z = FPPSocketOffSetZ;
		CameraBoom->TargetArmLength = FPPSocketLenght;
		// Set only owner not see mesh
		MyMesh->SetOwnerNoSee(true);
		// Enable 1st person hand
		FPSMesh->SetVisibility(true,true);
		// ...and FPSGun
		// Change Bullet Create position
		SkeletalMeshMuzzle->AttachToComponent(FPSMesh, FAttachmentTransformRules::KeepRelativeTransform, "b_RightWeapon");
		GunOffset = FPPGunOffset;
		// ......
	}
}
//////////////////////////////////////////////////
//////Fire
void AMyMainCharacter::StartFire_Auto_Implementation()
{
	if (!bIsFiringWeapon)
	{
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(AutoFiringTimer, this, &AMyMainCharacter::HandleFire_Auto_Implementation, FireRate, true, OverHeatTime);
		bIsFiringWeapon = true;
	}
}
void AMyMainCharacter::StopFire_Auto()
{
	UWorld* World = GetWorld();
	bIsFiringWeapon = false;

	//計算放開板機後下一發可發射的延遲時間
	OverHeatTime = World->GetTimerManager().GetTimerRemaining(AutoFiringTimer);

	//World->GetTimerManager().SetTimer(AfterShoot, this, &AMyMainCharacter::StopFire_AfterFireRate, true, OverHeatTime);
	//World->GetTimerManager().ClearTimer(AutoFiringTimer);
}
void AMyMainCharacter::StopFire_AfterFireRate() 
{
	UWorld* World = GetWorld();
	OverHeatTime = World->GetTimerManager().GetTimerRemaining(AfterShoot);
	if(OverHeatTime <= 0.f)
		World->GetTimerManager().ClearTimer(AfterShoot);
}
/*Unreal Server rep auto suffix*/
void AMyMainCharacter::HandleFire_Auto_Implementation()
{
	FVector spawnLocation = SkeletalMeshMuzzle->GetComponentLocation() + GetControlRotation().RotateVector(GunOffset);//GetActorLocation() + (GetControlRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetControlRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = Instigator;
	spawnParameters.Owner = this;

	AThirdPersonMPProjectile* spawnedProjectile = GetWorld()->SpawnActor<AThirdPersonMPProjectile>(spawnLocation, spawnRotation, spawnParameters);
}