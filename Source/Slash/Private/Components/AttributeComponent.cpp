// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAttributeComponent::ReveiveDamage(float Damage)
{
	Health = Health - Damage > 0 ? Health - Damage : 0.f; // Ensure health does not go below zero
}

