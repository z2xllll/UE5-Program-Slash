
#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.h"
#include "BaseCharacter.h"
#include "SlashCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class AItem;
class UAnimMontage;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint,AActor* Hitter) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(BlueprintReadWrite)
	EActionState ActionState = EActionState::EAS_Unoccupied;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	virtual void BeginPlay() override;
	void InitializeSlashOverlay(APlayerController* PlayerController);
	/**
	* Callbacks for input
	*/
	void MoveForward(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void MoveRight(float Value);
	void EKeyPressed();
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void PlayAttackMontage() override;
	virtual void HandleDamage(float DamageAmount) override;

	void PlayEquipMontage(const FName& SectionName);
	bool CanDisarm();//����ж������
	bool CanArm();

	/**
	* Play Montage functions
	*/
	virtual void AttackEnd() override;
	virtual void Destroyed() override;
	virtual void Die() override;

	UFUNCTION(BlueprintCallable)
	void Arm();

	UFUNCTION(BlueprintCallable)
	void Disarm();

	UFUNCTION(BlueprintCallable)
	void FinishedEquip();

private:

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;	//Overlapped Item

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EquipMontage;

	UPROPERTY()
	class USlashOverlay* SlashOverlay;

public:
	//ǿ������
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	//�����ⲿ���� 
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
