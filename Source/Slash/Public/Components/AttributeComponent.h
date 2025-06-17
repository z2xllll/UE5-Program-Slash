

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateStamina(float DeltaTime);

private:
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health=100.f;

	UPROPERTY(EditAnywhere,Category = "Actor Attributes")
	float MaxHealth=100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Stamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Gold=100;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Souls;

public:
	void ReveiveDamage(float Damage);
	void UseStamina(float StaminaCost);
	FORCEINLINE float GetHealthPercent() const { return MaxHealth != 0 ? Health / MaxHealth : 0.f; }
	FORCEINLINE float GetStaminaPercent() const { return MaxStamina != 0 ? Stamina / MaxStamina : 0.f; }
	FORCEINLINE bool IsAlive() const { return Health > 0.f; }

	void AddSouls(int32 Amount);
	void AddGold(int32 Amount);
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
};
