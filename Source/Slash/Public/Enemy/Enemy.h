
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UAnimMontage;

UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();

protected:
	virtual void BeginPlay() override;

	/*
	* Play Montage functions
	*/
	void PlayHitReactMontage(const FName& SectionName);

	bool InTargetRange(AActor* Target,double Radius);

	void MoveToTarget(AActor* Target);

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn); // Called when the PawnSensing component detects a pawn

	UPROPERTY(BlueprintReadOnly)
	bool bIsAlive = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite,Category = "AI Navigation")
	AActor* PatrolTarget;

	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

private:

	UPROPERTY(VisibleAnywhere)
	class UAttributeComponent* Attributes;

	UPROPERTY(VisibleAnywhere)
	class UPawnSensingComponent* PawnSensing;

	/*
	* Animation Montage for the enemy's Hit Reaction
	*/

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "VisualEffects")
	UParticleSystem* HitParticles;

	UPROPERTY(VisibleAnywhere)
	class UHealthBarComponent* HealthBarWidget;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.0f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	/*
	*Navigation
	*/

	class AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.0f;

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void DirectionalHitReact(const FVector& ImpactPoint);

	void Die();

};
