// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"
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
	PawnSensing->SightRadius = 2000.f; //���ø�֪�뾶Ϊ1000��λ
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	HideHealthBar(); //��ʼ����Ѫ��
	EnemyController = Cast<AAIController>(GetController()); //��ȡAI������
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = 150.f;
	if(PatrolTarget)
	MoveToTarget(PatrolTarget); //��ʼѲ��
	Tags.Add(FName("Enemy")); //��ӵ��˱�ǩ, ��������ϵͳʶ��

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen); //�󶨸�֪�¼�
	}

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
	if (HealthBarWidget)HealthBarWidget->SetVisibility(false); //��ʼ����Ѫ��
}

bool AEnemy::InTargetRange(AActor* Target,double Radius)
{
	if (!Target) return false; //���Ŀ��Ϊ��, �򷵻�false
	const double DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	return DistanceToTarget <= Radius; //���Ŀ�����С�ڵ��ڰ뾶, �򷵻�true
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!Target||EnemyState==EEnemyState::EES_Engaged||EnemyState==EEnemyState::EES_Attacking) return; //���Ŀ��Ϊ��, �򷵻�
	if (EnemyController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target); //����Ѳ��Ŀ��
		MoveRequest.SetAcceptanceRadius(40.f); //���ý��ܰ뾶
		EnemyController->MoveTo(MoveRequest); //��ʼ�ƶ���Ѳ��Ŀ��
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (EnemyState != EEnemyState::EES_Patrolling)return; //�������״̬����Ѳ��, �򲻴����֪�¼�
	if (SeenPawn->ActorHasTag(FName("SlashCharacter"))&&!SeenPawn->ActorHasTag(FName("Dead")))
	{
		CombatTarget = SeenPawn; //����ս��Ŀ��Ϊ����֪�Ľ�ɫ
	}
}

void AEnemy::StartAttackTimer()
{
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
	if (EnemyController)
	{
		EnemyController->StopMovement(); // ����ֹͣ��ǰ�ƶ�
	}
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
		&&EnemyState!= EEnemyState::EES_Engaged; // ����Ƿ���Թ���
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget); //Ѳ�߼�ʱ��������, �ƶ���Ѳ��Ŀ��
}

void AEnemy::Tick(float DeltaTime)
{
	//��ӡ״̬
	//UE_LOG(LogTemp, Warning, TEXT("Enemy State: %s"), *UEnum::GetValueAsString(EnemyState));
	Super::Tick(DeltaTime);
	if (EnemyState==EEnemyState::EES_Dead) return; //��������Ѿ�����, ��ִ�������߼�
	if (CombatTarget)
	{
		if (!InTargetRange(CombatTarget,CombatRadius))
		{
			HideHealthBar(); //����Ѫ��
			StartPatrolling();
		}
		else if(!InTargetRange(CombatTarget,AttackRadius)&&InTargetRange(CombatTarget,CombatRadius))
		{
			GetWorldTimerManager().ClearTimer(PatrolTimer); //���Ѳ�߼�ʱ��
			ShowHealthBar();
			StartChasing();
		}
		else if (CanAttack()) 
		{
			GetWorldTimerManager().ClearTimer(PatrolTimer); //���Ѳ�߼�ʱ��
			GetCharacterMovement()->MaxWalkSpeed = 0.f;
			ShowHealthBar();
			StartAttackTimer(); //��ʼ������ʱ��
		}
	}
	else
	{
		StartPatrolling(); //���û��ս��Ŀ��, ��ʼѲ��
		HideHealthBar(); //����Ѫ��
	}
}

void AEnemy::StartChasing()
{
	EnemyState = EEnemyState::EES_Chasing; //���õ���״̬Ϊ׷��
	GetCharacterMovement()->MaxWalkSpeed = 300.f; // ����׷���ٶ�
	MoveToTarget(CombatTarget); //�ƶ���ս��Ŀ��
}

void AEnemy::StartPatrolling()
{
	CombatTarget = nullptr; 
	EnemyState = EEnemyState::EES_Patrolling; 
	GetCharacterMovement()->MaxWalkSpeed = 150.f; 
	if (PatrolTargets.Num() == 0) return;
	// ����Ƿ���Ҫѡ���µ�Ѳ��Ŀ��,�����ƶ���
	if ((!PatrolTarget || !GetWorldTimerManager().IsTimerActive(PatrolTimer))&&GetVelocity().Size() <= 5.0f)
	{
		const int32 TargetIndex = FMath::RandRange(0, PatrolTargets.Num() - 1);
		PatrolTarget = PatrolTargets[TargetIndex];
		GetWorldTimerManager().SetTimer(
			PatrolTimer,
			this,
			&AEnemy::PatrolTimerFinished,
			3.f
		);

	}
	else 
	{
		MoveToTarget(PatrolTarget); //���Ѳ��Ŀ�����, ���ƶ���Ѳ��Ŀ��
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)HealthBarWidget->SetVisibility(true); //��ʾѪ��
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (EnemyState == EEnemyState::EES_Dead) return; //��������Ѿ�����, �򲻴����ܻ��¼�
	AttackEnd(); //��������״̬
	GetWorldTimerManager().ClearTimer(AttackTimer); //���������ʱ��
	EnemyState = EEnemyState::EES_Idle; //���õ���״̬Ϊ����
	ShowHealthBar();
	PlayHitSound(ImpactPoint); //�����ܻ���Ч
	PlayHitParticles(ImpactPoint); //�����ܻ�����Ч��
	if (IsAlive()&&Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount); //���ø��ദ���˺�����
	if(HealthBarWidget)HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent()); //����Ѫ���ٷֱ�
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount); //�����˺�
	CombatTarget = EventInstigator->GetPawn(); //����ս��Ŀ��Ϊ������
	return DamageAmount;
}


void AEnemy::Destroyed()
{
	if (EquippedWeapon)EquippedWeapon->Destroy();
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
		FTimerHandle SoulSpawnTimer;
		UWorld* World = GetWorld();
		if(World)
			World->GetTimerManager().SetTimer(SoulSpawnTimer, this, &AEnemy::SpawnSoul, 2.f, false); //2����������
		
	}
}
void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if (World && SoulClass)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 50.f); //����λ���ڵ����Ϸ�
		ASoul* SpawnSoul = World->SpawnActor<ASoul>(SoulClass, SpawnLocation, FRotator::ZeroRotator); //�������
		if (!SpawnSoul) return; //�������ʧ��, �򷵻�
		SpawnSoul->SetSouls(Attributes->GetSouls()); //�����������
	}
}
void AEnemy::Attack()
{
	if (EnemyState == EEnemyState::EES_Dead) return;
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr; //���ս��Ŀ���Ѿ�����, �����ս��Ŀ��
		return;
	}
	EnemyState = EEnemyState::EES_Engaged; //���õ���״̬Ϊ��ս
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
	if (EnemyController)
	{
		EnemyController->StopMovement(); // ����ֹͣ��ǰ�ƶ�
	}
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EquippedWeapon->IgnoreActors.Empty();//��պ��ԵĽ�ɫ�б�
}

FVector AEnemy::GetRotationWarpTarget()
{
	if (CombatTarget == nullptr)return FVector();
	const FVector TargetLocation = CombatTarget->GetActorLocation();
	const FVector EnemyLocation = GetActorLocation();

	FVector Direction = (TargetLocation - EnemyLocation).GetSafeNormal(); //��ȡĿ�귽��
	return TargetLocation - Direction * WarpTargetDistance; //��������λ��-����*����
}

FVector AEnemy::GetTranslationWarpTarget()
{
	if (CombatTarget)return CombatTarget->GetActorLocation();
	return FVector();
}

void AEnemy::AttackEndState()
{
	EnemyState = EEnemyState::EES_Idle; //���õ���״̬Ϊ����
}
