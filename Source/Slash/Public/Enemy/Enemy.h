
#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"


UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

protected:
	virtual void BeginPlay() override;

	void HideHealthBar();


	bool InTargetRange(AActor* Target,double Radius);

	void MoveToTarget(AActor* Target);

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn); // Called when the PawnSensing component detects a pawn

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite,Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	virtual void Attack() override;
	virtual void PlayAttackMontage() override;

private:


	UPROPERTY(VisibleAnywhere)
	class UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

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

	void StartChasing();

	void StartPatrolling();

	void ShowHealthBar();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Destroyed() override;

	virtual void Die() override;

};
