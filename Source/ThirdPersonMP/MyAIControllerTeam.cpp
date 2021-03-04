// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAIControllerTeam.h"

AMyAIControllerTeam::AMyAIControllerTeam()
{
    SetGenericTeamId(FGenericTeamId(5));
}


ETeamAttitude::Type AMyAIControllerTeam::GetTeamAttitudeTowards(const AActor& Other) const
{

    if (const APawn* OtherPawn = Cast<APawn>(&Other)) {

        // DEFAULT BEHAVIOR---------------------------------------------------
        if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
        {
            return Super::GetTeamAttitudeTowards(*OtherPawn->GetController());
        }

        //OR CUSTOM BEHAVIOUR--------------------------------------------------
        if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
        {
            //Create an alliance with Team with ID 10 and set all the other teams as Hostiles:
            FGenericTeamId OtherTeamID = TeamAgent->GetGenericTeamId();
            if (OtherTeamID == 10) {
                return ETeamAttitude::Neutral;
            }
            else {
                return ETeamAttitude::Hostile;
            }
        }
    }

    return ETeamAttitude::Neutral;
}