// Fill out your copyright notice in the Description page of Project Settings.


#include "MyDeveloperSettings.h"

UMyDeveloperSettings::UMyDeveloperSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    typedef ETeamAttitude::Type EA;
    TeamAttitudes = {
        {EA::Friendly, EA::Neutral,  EA::Neutral,  EA::Neutral,  EA::Neutral },//Neutral
        {EA::Neutral, EA::Friendly, EA::Hostile,  EA::Friendly, EA::Hostile},//Team1
        {EA::Neutral, EA::Hostile,  EA::Friendly, EA::Hostile,  EA::Hostile},//Team2
        {EA::Neutral, EA::Friendly, EA::Hostile,  EA::Friendly, EA::Friendly },//Team3
        {EA::Neutral, EA::Hostile,  EA::Hostile,  EA::Friendly, EA::Friendly }//Team4
    };
}

const UMyDeveloperSettings* UMyDeveloperSettings::Get()
{
    return GetDefault<UMyDeveloperSettings>();
}

ETeamAttitude::Type UMyDeveloperSettings::GetAttitude(FGenericTeamId Of, FGenericTeamId Towards)
{
    auto& teamAttitudes = UMyDeveloperSettings::Get()->TeamAttitudes;
    bool ofValid = teamAttitudes.IsValidIndex(Of.GetId());
    bool towardsValid = teamAttitudes.IsValidIndex(Towards.GetId());

    if (ofValid && towardsValid)
    {
        auto& attitudes = teamAttitudes[Of.GetId()].Attitude;
        if (attitudes.IsValidIndex(Towards.GetId()))
        {
            return attitudes[Towards.GetId()];
        }
    }
    return ETeamAttitude::Neutral;
}