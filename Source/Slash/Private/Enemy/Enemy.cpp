// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	//�����ڵ����
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//�����ص��¼�
	GetMesh()->SetGenerateOverlapEvents(true);
	//�����ڵ����
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);


	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true; //��ɫ�����ƶ�����

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SetPeripheralVisionAngle(45.f); //���ø�֪�Ƕ�Ϊ90��
	PawnSensing->SightRadius = 4000.f; //���ø�֪�뾶Ϊ1000��λ
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	HideHealthBar(); //��ʼ����Ѫ��
	EnemyController = Cast<AAIController>(GetController()); //��ȡAI������
	EnemyState = EEnemyState::EES_Patrolling;
	if(PatrolTarget)
	MoveToTarget(PatrolTarget); //��ʼѲ��

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen); //�󶨸�֪�¼�
	}
	GetCharacterMovement()->MaxWalkSpeed = 150.f;

	SpawnDefaultWeapon();
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false); //��ʼ����Ѫ��
	}
}

bool AEnemy::InTargetRange(AActor* Target,double Radius)
{
	if (!Target) return false; //���Ŀ��Ϊ��, �򷵻�false
	const double DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	return DistanceToTarget <= Radius; //���Ŀ�����С�ڵ��ڰ뾶, �򷵻�true
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!Target) return; //���Ŀ��Ϊ��, �򷵻�
	if (EnemyController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target); //����Ѳ��Ŀ��
		MoveRequest.SetAcceptanceRadius(50.f); //���ý��ܰ뾶
		EnemyController->MoveTo(MoveRequest); //��ʼ�ƶ���Ѳ��Ŀ��
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (EnemyState != EEnemyState::EES_Patrolling) return; //�������״̬����Ѳ��, �򲻴����֪�¼�
	if (SeenPawn->ActorHasTag(FName("SlashCharacter")))
	{
		EnemyState = EEnemyState::EES_Chasing; //���õ���״̬Ϊ׷��
		GetWorldTimerManager().ClearTimer(PatrolTimer); //���Ѳ�߼�ʱ��
		CombatTarget = SeenPawn; //����ս��Ŀ��Ϊ����֪�Ľ�ɫ
		MoveToTarget(CombatTarget); //�ƶ���ս��Ŀ��
	}
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking; //���õ���״̬Ϊ����
	GetWorldTimerManager().SetTimer(
		AttackTimer,
		this,
		&AEnemy::Attack,
		AttackWaitTime//������ʱ��
	);
}

bool AEnemy::CanAttack()
{
	return InTargetRange(CombatTarget, AttackRadius)&&
		EnemyState != EEnemyState::EES_Attacking
		&& EnemyState != EEnemyState::EES_Dead
		&& EnemyState != EEnemyState::EES_Engaged; // ����Ƿ���Թ���
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget); //Ѳ�߼�ʱ��������, �ƶ���Ѳ��Ŀ��
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (EnemyState==EEnemyState::EES_Dead||EnemyState==EEnemyState::EES_Engaged) return; //��������Ѿ�����, ��ִ�������߼�

	if (CombatTarget)
	{
		if (!InTargetRange(CombatTarget,CombatRadius)&&EnemyState!=EEnemyState::EES_Patrolling)
		{
			GetWorldTimerManager().ClearTimer(AttackTimer); //���������ʱ��
			HideHealthBar(); //����Ѫ��
			if(EnemyState!=EEnemyState::EES_Engaged)StartPatrolling();
		}
		else if(!InTargetRange(CombatTarget,AttackRadius)&&InTargetRange(CombatTarget,CombatRadius)&& EnemyState != EEnemyState::EES_Chasing)
		{
			GetWorldTimerManager().ClearTimer(AttackTimer); //���������ʱ��
			ShowHealthBar();
			if (EnemyState != EEnemyState::EES_Engaged)StartChasing();
		}
		else if (CanAttack()) 
		{
			StartAttackTimer(); //��ʼ������ʱ��
		}
	}
	else if(EnemyState!= EEnemyState::EES_Patrolling)
	{
		StartPatrolling(); //���û��ս��Ŀ��, ��ʼѲ��
		HideHealthBar(); //����Ѫ��
	}
	if (EnemyState == EEnemyState::EES_Patrolling)
	{
		if (PatrolTargets.Num() == 0) return;

		// ����Ƿ���Ҫѡ���µ�Ѳ��Ŀ��
		if ((!PatrolTarget || !GetWorldTimerManager().IsTimerActive(PatrolTimer))&& !(EnemyController->GetMoveStatus() == EPathFollowingStatus::Moving))
		{
			const int32 TargetIndex = FMath::RandRange(0, PatrolTargets.Num() - 1);
			PatrolTarget = PatrolTargets[TargetIndex];
			GetWorldTimerManager().SetTimer(
				PatrolTimer,
				this,
				&AEnemy::PatrolTimerFinished,
				2.f
			);

		}
	}
}

void AEnemy::StartChasing()
{
	if (EnemyController)
	{
		EnemyController->StopMovement();
	}
	EnemyState = EEnemyState::EES_Chasing; //���õ���״̬Ϊ׷��
	GetCharacterMovement()->MaxWalkSpeed = 300.f; // ����׷���ٶ�
	MoveToTarget(CombatTarget); //�ƶ���ս��Ŀ��
}

void AEnemy::StartPatrolling()
{
	if (EnemyController)
	{
		EnemyController->StopMovement();
	}
	CombatTarget = nullptr; 
	EnemyState = EEnemyState::EES_Patrolling; 
	GetCharacterMovement()->MaxWalkSpeed = 150.f; 
	MoveToTarget(PatrolTarget);
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true); //��ʾѪ��
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	if (EnemyState == EEnemyState::EES_Dead) return; //��������Ѿ�����, �򲻴����ܻ��¼�
	ShowHealthBar();
	if (IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	}
	else Die();

	PlayHitSound(ImpactPoint); //�����ܻ���Ч
	PlayHitParticles(ImpactPoint); //�����ܻ�����Ч��
	
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount); //���ø��ദ���˺�����
	if(HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent()); //����Ѫ���ٷֱ�
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount); //�����˺�
	CombatTarget = EventInstigator->GetPawn(); //����ս��Ŀ��Ϊ������
	return DamageAmount;
}


void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::Die()
{
	//������������
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		HideHealthBar(); //����Ѫ��
		EnemyState = EEnemyState::EES_Dead; //���õ���״̬Ϊ����
		PlayDeathMontage(); //������������
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //������ײ
		SetLifeSpan(5.f); //����������, 5�������
	}
}
void AEnemy::Attack()
{
	if (EnemyState == EEnemyState::EES_Dead) return;
	EnemyState = EEnemyState::EES_Engaged; //���õ���״̬Ϊ��ս
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_Idle; //���õ���״̬Ϊ����
}