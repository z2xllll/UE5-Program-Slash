// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

void ASlashHUD::BeginPlay()
{
	Super::BeginPlay();
}

USlashOverlay* ASlashHUD::GetSlashOverlay()
{
	if (SlashOverlay)return SlashOverlay;
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			SlashOverlay = CreateWidget<USlashOverlay>(PlayerController, SlashOverlayClass);
			SlashOverlay->AddToViewport();
		}
	}
	return SlashOverlay;
}
