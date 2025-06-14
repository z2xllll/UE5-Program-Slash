
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
	virtual void Tick(float DeltaTime) override;

	/*Hit Reaction*/
	virtual void GetHit_Implementation(const FVector& ImpactPoint,AActor* Hitter) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	virtual void Die() override;

	/*State Transform*/
	void StartChasing();
	void StartPatrolling();
	void ShowHealthBar();
	void HideHealthBar();

protected:
	virtual void BeginPlay() override;

	void SpawnDefaultWeapon();

	/*AI Navigation*/
	UFUNCTION()
	void PawnSeen(APawn* SeenPawn); // Called when the PawnSensing component detects a pawn
	bool InTargetRange(AActor* Target,double Radius);
	void MoveToTarget(AActor* Target);

	/*Attack */
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual void AttackEnd() override;
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();
	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void AttackEndState();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = "Combat")
	double WarpTargetDistance = 80.0f; // Distance to warp the target for attack

private:

	/*Timer*/
	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();
	void StartAttackTimer();

	UPROPERTY(VisibleAnywhere)
	class UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(VisibleAnywhere)
	class UHealthBarComponent* HealthBarWidget;


	UPROPERTY(EditAnywhere)
	double CombatRadius = 500.0f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackWaitTime = 1.f;

	class AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.0f;

};
