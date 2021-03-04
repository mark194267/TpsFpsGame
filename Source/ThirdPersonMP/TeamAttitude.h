// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "TeamAttitude.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FTeamAttitude
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TArray<TEnumAsByte<ETeamAttitude::Type>> Attitude;

    FTeamAttitude() {};

    FTeamAttitude(std::initializer_list<TEnumAsByte<ETeamAttitude::Type>> attitudes) :
        Attitude(std::move(attitudes))
    { };
};
