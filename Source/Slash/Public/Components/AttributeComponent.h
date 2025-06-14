

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

private:
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health=100.f;

	UPROPERTY(EditAnywhere,Category = "Actor Attributes")
	float MaxHealth=100.f;

public:
	void ReveiveDamage(float Damage);
	FORCEINLINE float GetHealthPercent() const { 
		UE_LOG(LogTemp, Warning, TEXT("Health: %f, MaxHealth: %f"), Health, MaxHealth);
		return MaxHealth!=0?Health / MaxHealth:0.f; }
	FORCEINLINE bool IsAlive() const { return Health > 0.f; }
};
