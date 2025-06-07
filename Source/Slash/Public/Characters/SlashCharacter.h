
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



protected:
	UPROPERTY(BlueprintReadWrite)
	EActionState ActionState = EActionState::EAS_Unoccupied;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	virtual void BeginPlay() override;
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

	void PlayEquipMontage(const FName& SectionName);
	bool CanDisarm();//可以卸下武器
	bool CanArm();

	/**
	* Play Montage functions
	*/
	virtual void PlayAttackMontage() override;

	virtual void AttackEnd() override;

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


	/*
	* Animation Montages
	*/

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EquipMontage;

public:
	//强制内联
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	//允许外部访问 
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
