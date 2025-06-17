
#include "Characters/SlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Item.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;//ʹ��ɫ��������ƶ�����ı�
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1000.f, 0.f);//�ı���ת�ٶ�

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);//���ý�ɫ������ײ����
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);//����������ײ
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);//���ÿɼ���ͨ��Ϊ�赲
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECR_Overlap);//�������綯̬ͨ��Ϊ�ص�
	//�����ص��¼�
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

}


void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("ASlashCharacter::BeginPlay called."));
	Tags.Add(FName("SlashCharacter"));//��ӱ�ǩ���������ɫ����

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	InitializeSlashOverlay(PlayerController);
}

void ASlashCharacter::InitializeSlashOverlay(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthPercent(Attributes->GetHealthPercent());//����Ѫ���ٷֱ�
				SlashOverlay->SetStaminaPercent(1.f);//�����������ٷֱ�
				SlashOverlay->SetGoldAmount(100);//���ý������
				SlashOverlay->SetSoulAmount(0);//�����������
			}
		}
	}
}

void ASlashCharacter::MoveForward(float Value)
{
	if (ActionState != EActionState::EAS_Unoccupied)return;
	if (Controller && Value != 0.f)
	{
		// find out which way is forward
		const FRotator ControllerRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControllerRotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASlashCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ASlashCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ASlashCharacter::MoveRight(float Value)
{
	if (ActionState != EActionState::EAS_Unoccupied)return;
	if (Controller && Value != 0.f)
	{
		// find out which way is right
		const FRotator ControllerRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControllerRotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"),this,this);
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		OverlappingItem = nullptr;
		EquippedWeapon = OverlappingWeapon;
	}
	else
	{
		if (CanDisarm())
		{
			PlayEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_Equipping;
		}
		else if (CanArm())
		{
			PlayEquipMontage(FName("Equip"));
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			ActionState = EActionState::EAS_Equipping;
		}
	}
}

void ASlashCharacter::DodgeKeyPressed()
{
	PlayDodgeMontage();
}

void ASlashCharacter::Attack()
{
	if(CanAttack())
	{
		ActionState = EActionState::EAS_Attacking;
		PlayAttackMontage();
	}
}


bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanDisarm()
{
	return CharacterState != ECharacterState::ECS_Unequipped
		&& ActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanArm()
{
	return CharacterState == ECharacterState::ECS_Unequipped
		&& ActionState == EActionState::EAS_Unoccupied
		&& EquippedWeapon;
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
	EquippedWeapon->IgnoreActors.Empty();//��պ��ԵĽ�ɫ�б�
}

void ASlashCharacter::Arm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::Disarm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::FinishedEquip()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::DodgeEnd()
{
	ActionState = EActionState::EAS_Unoccupied; //���ܽ���������״̬Ϊ����
}


void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Attributes && SlashOverlay)
	{
		Attributes->UpdateStamina(DeltaTime); //��������ֵ
		SlashOverlay->SetStaminaPercent(Attributes->GetStaminaPercent()); //�����������ٷֱ�
	}
}

void ASlashCharacter::PlayAttackMontage()
{
	Super::PlayAttackMontage();
}

void ASlashCharacter::PlayDodgeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (ActionState != EActionState::EAS_Unoccupied) return; //�����ǰ״̬���ǿ���״̬����ִ�����ܶ���
	if (Attributes && Attributes->GetStaminaPercent() <= 0.f) return; //�������ֵС�ڵ���0����ִ�����ܶ���
	if (AnimInstance && DodgeMontage)
	{
		ActionState = EActionState::EAS_Dodging; //���õ�ǰ״̬Ϊ������
		AnimInstance->Montage_Play(DodgeMontage,1.5f);
		if(Attributes)
		{
			Attributes->UseStamina(20.f); //��������
		}
	}
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	PlayHitSound(ImpactPoint);//�����ܻ���Ч
	PlayHitParticles(ImpactPoint);//�����ܻ���Ч
	AttackEnd(); //��������״̬
	ActionState = EActionState::EAS_Unoccupied; //���õ�ǰ״̬Ϊ����״̬
	if (IsAlive() && Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();
}


void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ASlashCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &ASlashCharacter::Turn);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ASlashCharacter::LookUp);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ASlashCharacter::MoveRight);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &ASlashCharacter::EKeyPressed);
	PlayerInputComponent->BindAction(FName("Attack"), IE_Pressed, this, &ASlashCharacter::Attack);
	PlayerInputComponent->BindAction(FName("Dodge"), IE_Pressed, this, &ASlashCharacter::DodgeKeyPressed);
}

float ASlashCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//UE_LOG(LogTemp, Warning, TEXT("ASlashCharacter::TakeDamage called with DamageAmount: %f"), DamageAmount);
	HandleDamage(DamageAmount); //�����˺�
	if (SlashOverlay)
	{
		SlashOverlay->SetHealthPercent(Attributes->GetHealthPercent()); //����Ѫ���ٷֱ�
	}
	return DamageAmount;
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item; //�����ص���Ʒ
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes&&SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSoulAmount(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes&&SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGoldAmount(Attributes->GetGold());
	}
}

void ASlashCharacter::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount); //���ø��ദ���˺�����
}

void ASlashCharacter::Destroyed()
{
	if (EquippedWeapon)EquippedWeapon->Destroy();
}

void ASlashCharacter::Die()
{
	//������������
	Tags.Add(FName("Dead")); //��ӱ�ǩ���������ɫ�Ƿ�����
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		CharacterState = ECharacterState::ECS_Dead; //���ý�ɫ״̬Ϊ����
		PlayDeathMontage(); //������������
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //������ײ
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //����������ײ
	}
}

