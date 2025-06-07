

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

class UAnimMontage;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
protected:
	virtual void BeginPlay() override;
	virtual void Attack();
	virtual bool CanAttack();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	/*
	* Play Montage functions
	*/
	void PlayHitReactMontage(const FName& SectionName);

	virtual void Die();

	/*Play Montage*/

	virtual void PlayAttackMontage();

	void DirectionalHitReact(const FVector& ImpactPoint);

	/*
	* Animation Montages
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class AWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* HitReactMontage;

	UPROPERTY(VisibleAnywhere)
	class UAttributeComponent* Attributes;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "VisualEffects")
	UParticleSystem* HitParticles;
};
