// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterface.h"
#include "BreakableActor.generated.h"

class UGometryCollectionComponent;

UCLASS()
class SLASH_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	ABreakableActor();

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	// The Geometry Collection Component that will handle the breakable geometry
	UGeometryCollectionComponent* GeometryCollection;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	class UCapsuleComponent* Capsule;

public:	

	bool IsBroken = false; // Flag to check if the actor is already broken

private:

		UPROPERTY(EditAnywhere,Category="Breakable Properties")
	TArray<TSubclassOf<class ATreasure>> TreasureClasses; // Classes of treasures to spawn when broken

};
