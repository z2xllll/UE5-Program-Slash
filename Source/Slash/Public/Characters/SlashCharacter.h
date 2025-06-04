
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterTypes.h"
#include "SlashCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class AItem;
class UAnimMontage;
class AWeapon;

UCLASS()
class SLASH_API ASlashCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 
	*Enable/Disable weapon collision. This is used to prevent the weapon from colliding with the character's own body when attacking.
	*/

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);


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
	void Attack();
	bool CanAttack();

	void PlayEquipMontage(const FName& SectionName);
	bool CanDisarm();//可以卸下武器
	bool CanArm();

	/**
	* Play Montage functions
	*/
	void PlayAttackMontage();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

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

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	AWeapon* EquippedWeapon;

	/*
	* Animation Montages
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EquipMontage;

public:
	//强制内联
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	//允许外部访问 
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
