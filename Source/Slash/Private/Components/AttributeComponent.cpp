// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAttributeComponent::UpdateStamina(float DeltaTime)
{
	Stamina = FMath::Clamp(Stamina + 5.f * DeltaTime, 0.f, MaxStamina); // Regenerate stamina
}

void UAttributeComponent::ReveiveDamage(float Damage)
{
	//打印血量
	//UE_LOG(LogTemp, Warning, TEXT("Health before damage: %f"), Health);
	Health = Health - Damage > 0 ? Health - Damage : 0.f; // Ensure health does not go below zero
}

void UAttributeComponent::UseStamina(float StaminaCost)
{
	//打印体力
	//UE_LOG(LogTemp, Warning, TEXT("Stamina before use: %f"), Stamina);
	Stamina = Stamina - StaminaCost > 0 ? Stamina - StaminaCost : 0.f; // Ensure stamina does not go below zero
}

void UAttributeComponent::AddSouls(int32 Amount)
{
	Souls += Amount;
	// Optionally, you can log the new amount of souls
	// UE_LOG(LogTemp, Warning, TEXT("Souls after addition: %d"), Souls);
}

void UAttributeComponent::AddGold(int32 Amount)
{
	Gold += Amount;
	// Optionally, you can log the new amount of gold
	// UE_LOG(LogTemp, Warning, TEXT("Gold after addition: %d"), Gold);
}

