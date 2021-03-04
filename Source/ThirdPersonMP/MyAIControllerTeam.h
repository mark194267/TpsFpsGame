// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "MyAIControllerTeam.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSONMP_API AMyAIControllerTeam : public AAIController
{
    GENERATED_BODY()

public:
    AMyAIControllerTeam();
    // Override this function 
    ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
};

