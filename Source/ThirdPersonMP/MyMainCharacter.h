// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "TimerManager.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyMainCharacter.generated.h"

UCLASS()
class THIRDPERSONMP_API AMyMainCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	// Sets default values for this character's properties
	AMyMainCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	/* Invoke when press "ChangeView" button */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		bool Is3rdPerson = true;
	/////////////////////////////////////////////
///Fps Camera
/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
		class USkeletalMeshComponent* FPSMesh;
	///////////////////////////////
	///TPS Camera
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);
	/** The player's maximum health. This is the highest that their health can be, and the value that their health starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();
	void ChangeViewPress();
	void ChangeCam();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	UPROPERTY(VisibleAnywhere, Category = "Gameplay|Projectile")
		TSubclassOf<class AThirdPersonMPProjectile> ProjectileClass;

	/** Delay between shots in seconds. Used to control fire rate for our test projectile, but also to prevent an overflow of server functions from binding SpawnProjectile directly to input.*/
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float FireRate;

	/** If true, we are in the process of firing projectiles. */
	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly, Category = "Gameplay")
		bool bIsFiringWeapon;

	/** A timer handle used for providing the fire rate delay in-between spawns.*/
	FTimerHandle FiringTimer;
	FTimerHandle AutoFiringTimer;
	FTimerHandle AfterShoot;

	float OverHeatTime;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
		float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	////////////////////////////////////
	///FPS Bullet
	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		USkeletalMeshComponent* SkeletalMeshMuzzle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPP/TPP")
		FVector GunOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPP/TPP")
		FVector TPPGunOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPP/TPP")
		FVector FPPGunOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPP/TPP")
		float TPPSocketOffSetZ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPP/TPP")
		float FPPSocketOffSetZ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPP/TPP")
		float TPPSocketLenght;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPP/TPP")
		float FPPSocketLenght;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		USceneComponent* FP_MuzzleLocation;
	///** Server function for spawning projectiles.**/
	UFUNCTION(Server, Reliable)
		void HandleFire();
	/** Function for beginning weapon fire.**/
	UFUNCTION(Server, Reliable)
		virtual void StartFire();
	/** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopFire();

	/** Function for beginning weapon fire.**/
	UFUNCTION(Server, Reliable)
		virtual void StartFire_Auto();
	UFUNCTION(Server, Reliable)
		void HandleFire_Auto();
	/** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopFire_Auto();
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopFire_AfterFireRate();

};
