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
	//敌人遮挡相机
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//产生重叠事件
	GetMesh()->SetGenerateOverlapEvents(true);
	//胶囊遮挡相机
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);


	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true; //角色朝向移动方向

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SetPeripheralVisionAngle(45.f); //设置感知角度为90度
	PawnSensing->SightRadius = 2000.f; //设置感知半径为1000单位
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	HideHealthBar(); //初始隐藏血条
	EnemyController = Cast<AAIController>(GetController()); //获取AI控制器
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = 150.f;
	if(PatrolTarget)
	MoveToTarget(PatrolTarget); //开始巡逻
	Tags.Add(FName("Enemy")); //添加敌人标签, 方便其他系统识别

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen); //绑定感知事件
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
	if (HealthBarWidget)HealthBarWidget->SetVisibility(false); //初始隐藏血条
}

bool AEnemy::InTargetRange(AActor* Target,double Radius)
{
	if (!Target) return false; //如果目标为空, 则返回false
	const double DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	return DistanceToTarget <= Radius; //如果目标距离小于等于半径, 则返回true
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!Target||EnemyState==EEnemyState::EES_Engaged||EnemyState==EEnemyState::EES_Attacking) return; //如果目标为空, 则返回
	if (EnemyController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target); //设置巡逻目标
		MoveRequest.SetAcceptanceRadius(40.f); //设置接受半径
		EnemyController->MoveTo(MoveRequest); //开始移动到巡逻目标
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (EnemyState != EEnemyState::EES_Patrolling)return; //如果敌人状态不是巡逻, 则不处理感知事件
	if (SeenPawn->ActorHasTag(FName("SlashCharacter"))&&!SeenPawn->ActorHasTag(FName("Dead")))
	{
		CombatTarget = SeenPawn; //设置战斗目标为被感知的角色
	}
}

void AEnemy::StartAttackTimer()
{
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
	if (EnemyController)
	{
		EnemyController->StopMovement(); // 立即停止当前移动
	}
	EnemyState = EEnemyState::EES_Attacking; //设置敌人状态为攻击
	GetWorldTimerManager().SetTimer(
		AttackTimer,
		this,
		&AEnemy::Attack,
		AttackWaitTime//攻击计时器
	);
}

bool AEnemy::CanAttack()
{
	return InTargetRange(CombatTarget, AttackRadius)&&
		EnemyState != EEnemyState::EES_Attacking
		&& EnemyState != EEnemyState::EES_Dead
		&&EnemyState!= EEnemyState::EES_Engaged; // 检查是否可以攻击
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget); //巡逻计时器结束后, 移动到巡逻目标
}

void AEnemy::Tick(float DeltaTime)
{
	//打印状态
	//UE_LOG(LogTemp, Warning, TEXT("Enemy State: %s"), *UEnum::GetValueAsString(EnemyState));
	Super::Tick(DeltaTime);
	if (EnemyState==EEnemyState::EES_Dead) return; //如果敌人已经死亡, 则不执行其他逻辑
	if (CombatTarget)
	{
		if (!InTargetRange(CombatTarget,CombatRadius))
		{
			HideHealthBar(); //隐藏血条
			StartPatrolling();
		}
		else if(!InTargetRange(CombatTarget,AttackRadius)&&InTargetRange(CombatTarget,CombatRadius))
		{
			GetWorldTimerManager().ClearTimer(PatrolTimer); //清除巡逻计时器
			ShowHealthBar();
			StartChasing();
		}
		else if (CanAttack()) 
		{
			GetWorldTimerManager().ClearTimer(PatrolTimer); //清除巡逻计时器
			GetCharacterMovement()->MaxWalkSpeed = 0.f;
			ShowHealthBar();
			StartAttackTimer(); //开始攻击计时器
		}
	}
	else
	{
		StartPatrolling(); //如果没有战斗目标, 则开始巡逻
		HideHealthBar(); //隐藏血条
	}
}

void AEnemy::StartChasing()
{
	EnemyState = EEnemyState::EES_Chasing; //设置敌人状态为追击
	GetCharacterMovement()->MaxWalkSpeed = 300.f; // 设置追击速度
	MoveToTarget(CombatTarget); //移动到战斗目标
}

void AEnemy::StartPatrolling()
{
	CombatTarget = nullptr; 
	EnemyState = EEnemyState::EES_Patrolling; 
	GetCharacterMovement()->MaxWalkSpeed = 150.f; 
	if (PatrolTargets.Num() == 0) return;
	// 检查是否需要选择新的巡逻目标,不在移动中
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
		MoveToTarget(PatrolTarget); //如果巡逻目标存在, 则移动到巡逻目标
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)HealthBarWidget->SetVisibility(true); //显示血条
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (EnemyState == EEnemyState::EES_Dead) return; //如果敌人已经死亡, 则不处理受击事件
	AttackEnd(); //结束攻击状态
	GetWorldTimerManager().ClearTimer(AttackTimer); //清除攻击计时器
	EnemyState = EEnemyState::EES_Idle; //设置敌人状态为空闲
	ShowHealthBar();
	PlayHitSound(ImpactPoint); //播放受击音效
	PlayHitParticles(ImpactPoint); //播放受击粒子效果
	if (IsAlive()&&Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount); //调用父类处理伤害方法
	if(HealthBarWidget)HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent()); //更新血条百分比
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount); //处理伤害
	CombatTarget = EventInstigator->GetPawn(); //设置战斗目标为攻击者
	return DamageAmount;
}


void AEnemy::Destroyed()
{
	if (EquippedWeapon)EquippedWeapon->Destroy();
}

void AEnemy::Die()
{
	//播放死亡动画
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		HideHealthBar(); //隐藏血条
		EnemyState = EEnemyState::EES_Dead; //设置敌人状态为死亡
		PlayDeathMontage(); //播放死亡动画
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //禁用碰撞
		SetLifeSpan(5.f); //设置生命期, 5秒后销毁
		FTimerHandle SoulSpawnTimer;
		UWorld* World = GetWorld();
		if(World)
			World->GetTimerManager().SetTimer(SoulSpawnTimer, this, &AEnemy::SpawnSoul, 2.f, false); //2秒后生成灵魂
		
	}
}
void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if (World && SoulClass)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 50.f); //生成位置在敌人上方
		ASoul* SpawnSoul = World->SpawnActor<ASoul>(SoulClass, SpawnLocation, FRotator::ZeroRotator); //生成灵魂
		if (!SpawnSoul) return; //如果生成失败, 则返回
		SpawnSoul->SetSouls(Attributes->GetSouls()); //设置灵魂数量
	}
}
void AEnemy::Attack()
{
	if (EnemyState == EEnemyState::EES_Dead) return;
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr; //如果战斗目标已经死亡, 则清空战斗目标
		return;
	}
	EnemyState = EEnemyState::EES_Engaged; //设置敌人状态为交战
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
	if (EnemyController)
	{
		EnemyController->StopMovement(); // 立即停止当前移动
	}
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EquippedWeapon->IgnoreActors.Empty();//清空忽略的角色列表
}

FVector AEnemy::GetRotationWarpTarget()
{
	if (CombatTarget == nullptr)return FVector();
	const FVector TargetLocation = CombatTarget->GetActorLocation();
	const FVector EnemyLocation = GetActorLocation();

	FVector Direction = (TargetLocation - EnemyLocation).GetSafeNormal(); //获取目标方向
	return TargetLocation - Direction * WarpTargetDistance; //被攻击者位置-方向*距离
}

FVector AEnemy::GetTranslationWarpTarget()
{
	if (CombatTarget)return CombatTarget->GetActorLocation();
	return FVector();
}

void AEnemy::AttackEndState()
{
	EnemyState = EEnemyState::EES_Idle; //设置敌人状态为空闲
}
