// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USlashOverlay::SetStaminaPercent(float Percent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetGoldAmount(int32 Amount)
{
	if(GoldText)
	{
		GoldText->SetText(FText::AsNumber(Amount));
	}
}

void USlashOverlay::SetSoulAmount(int32 Amount)
{
	if(SoulText)
	{
		SoulText->SetText(FText::AsNumber(Amount));
	}
}

void USlashOverlay::SetHealthPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
	}
}



