// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Slash/Public/DebugMacros.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
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

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

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
	if(HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false); //初始隐藏血条
	}
	EnemyController = Cast<AAIController>(GetController()); //获取AI控制器
	MoveToTarget(PatrolTarget); //开始巡逻

	if (PawnSensing)
	{
		//设置感知事件
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen); //绑定感知事件
	}
	GetCharacterMovement()->MaxWalkSpeed = 150.f;
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

bool AEnemy::InTargetRange(AActor* Target,double Radius)
{
	if (!Target) return false; //如果目标为空, 则返回false
	const double DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	return DistanceToTarget <= Radius; //如果目标距离小于等于半径, 则返回true
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!Target) return; //如果目标为空, 则返回
	if (EnemyController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target); //设置巡逻目标
		MoveRequest.SetAcceptanceRadius(20.f); //设置接受半径, 20单位
		EnemyController->MoveTo(MoveRequest); //开始移动到巡逻目标
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (EnemyState == EEnemyState::EES_Chasing) return; //如果敌人状态为追击, 则不处理感知事件
	if (SeenPawn->ActorHasTag(FName("SlashCharacter")))
	{
		EnemyState = EEnemyState::EES_Chasing; //设置敌人状态为攻击
		GetWorldTimerManager().ClearTimer(PatrolTimer); //清除巡逻计时器
		CombatTarget = SeenPawn; //设置战斗目标为被感知的角色
		MoveToTarget(CombatTarget); //移动到战斗目标
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget); //巡逻计时器结束后, 移动到巡逻目标
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsAlive) return; //如果敌人已经死亡, 则不执行其他逻辑

	if (CombatTarget)
	{
		if (!InTargetRange(CombatTarget,CombatRadius))
		{
			//停止之前的移动
			if (EnemyController)
			{
				EnemyController->StopMovement();
			}
			CombatTarget = nullptr; //如果目标距离过远, 则清除战斗目标
			EnemyState = EEnemyState::EES_Patrolling; //设置敌人状态为巡逻
            GetCharacterMovement()->MaxWalkSpeed = 150.f; // 设置巡逻速度
			if (HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(false); //隐藏血条
			}
			MoveToTarget(PatrolTarget);
		}
		else if(!InTargetRange(CombatTarget,AttackRadius)&&InTargetRange(CombatTarget,CombatRadius))
		{
			EnemyState = EEnemyState::EES_Chasing; //设置敌人状态为追击
			GetCharacterMovement()->MaxWalkSpeed = 300.f; // 设置追击速度
			if (HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(true); //显示血条
			}
			MoveToTarget(CombatTarget); //移动到战斗目标
		}
		else if (InTargetRange(CombatTarget, AttackRadius))
		{
			EnemyState = EEnemyState::EES_Attacking; //设置敌人状态为攻击
			if(HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(true); //显示血条
			}
		}
	}
	else if(EnemyState!= EEnemyState::EES_Patrolling)
	{
		//如果没有战斗目标, 则设置敌人状态为巡逻
		EnemyState = EEnemyState::EES_Patrolling; //设置敌人状态为巡逻
		GetCharacterMovement()->MaxWalkSpeed = 150.f; // 设置巡逻速度
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false); //隐藏血条
		}
	}
	if (EnemyState == EEnemyState::EES_Patrolling)
	{
		if (PatrolTargets.Num() == 0) return;

		// 检查是否需要选择新的巡逻目标
		if ((!PatrolTarget || !GetWorldTimerManager().IsTimerActive(PatrolTimer))&& !(EnemyController->GetMoveStatus() == EPathFollowingStatus::Moving))
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
	}
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	//DRAW_IMPACTPOINT_SPHERE(ImpactPoint);
	if (Attributes&&Attributes->IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	}
	else
	{
		//如果敌人已经死亡, 播放死亡动画
		bIsAlive = false;
		Die();
	}


	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
	}
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			HitParticles,
			ImpactPoint
		);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Attributes)
	{
		Attributes->ReveiveDamage(DamageAmount);
		if (HealthBarWidget)
		{
			HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
		}
	}
	CombatTarget = EventInstigator->GetPawn(); //设置战斗目标为攻击者
	return DamageAmount;
}

void AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	//Lower ImpactPoint to the Enemy's location to avoid the height difference
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward| * |ToHit| * cos(Theta)
	// Forward = 1, ToHit = 1, cos(Angle) = Forward * ToHit
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	//convert to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	// If the cross product is negative, the angle is negative
	//反余弦总是返回正值, 点积是可正可负的
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0.0f)
	{
		Theta = -Theta;
	}

	FName Section("FromBack");
	if (Theta >= -45.0f && Theta <= 45.0f)
	{
		Section = FName("FromFront");
	}
	else if (Theta > 45.0f && Theta <= 135.0f)
	{
		Section = FName("FromRight");
	}
	else if (Theta < -45.0f && Theta >= -135.0f)
	{
		Section = FName("FromLeft");
	}

	PlayHitReactMontage(Section);
}

void AEnemy::Die()
{
	//播放死亡动画
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		HealthBarWidget->SetVisibility(false); //隐藏血条
		AnimInstance->Montage_Play(DeathMontage);
		AnimInstance->Montage_JumpToSection(FName("Death"),DeathMontage);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //禁用碰撞
		SetLifeSpan(5.f); //设置生命期, 5秒后销毁
	}
}

