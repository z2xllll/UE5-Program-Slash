// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	//名字必须和蓝图中绑定的名字一致，而且必须
	//先在蓝图中创建一个ProgressBar控件
	class UProgressBar* HealthBar;
	
};
