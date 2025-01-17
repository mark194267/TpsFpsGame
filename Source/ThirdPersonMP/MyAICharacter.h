// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Perception/PawnSensingComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyAICharacter.generated.h"

UCLASS()
class THIRDPERSONMP_API AMyAICharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPawnSensingComponent* PawnSensing;

public:
	// Sets default values for this character's properties
	AMyAICharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
};
