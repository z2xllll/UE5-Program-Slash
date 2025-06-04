// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HealthBarComponent.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API UHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	void SetHealthPercent(float Percent);
private:
	//避免多次类型转换
	UPROPERTY()
	class UHealthBar* HealthBarWidget;
};
