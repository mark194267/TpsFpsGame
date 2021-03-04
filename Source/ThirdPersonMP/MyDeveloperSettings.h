// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TeamAttitude.h"
#include "GenericTeamAgentInterface.h"
#include "MyDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig)
class THIRDPERSONMP_API UMyDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:

    UPROPERTY(Category = "Artificial Intelligence", EditAnywhere, BlueprintReadOnly, Config)
        TArray<FTeamAttitude> TeamAttitudes;

public:

    UMyDeveloperSettings(const FObjectInitializer& ObjectInitializer);

    static const UMyDeveloperSettings* Get();

    UFUNCTION(Category = "Artificial Intelligence", BlueprintPure)
        static ETeamAttitude::Type GetAttitude(FGenericTeamId Of, FGenericTeamId Towards);
};
