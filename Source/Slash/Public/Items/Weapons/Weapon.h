// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();
	void Equip(USceneComponent* InParent, FName InSocketName,AActor* NewActor,APawn* NewInstigater);
	void DeactivateEmbers();
	void DisableSphereCollision();
	void PlayEquipSound();
	void AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName);
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void BoxTrace(FHitResult& BoxHit);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage=20.f;
	
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase* EquipSound = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UBoxComponent* WeaponBox = nullptr;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceStart = nullptr;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceEnd = nullptr;

public:
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }
	//��¼���б����еĽ�ɫ��Ϣ
	TArray<AActor*> IgnoreActors;
};
