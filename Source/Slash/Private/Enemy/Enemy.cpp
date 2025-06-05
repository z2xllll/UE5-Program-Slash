// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Slash/Public/DebugMacros.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"

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

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	HealthBarWidget->SetVisibility(false); //初始隐藏血条
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

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CombatTarget)
	{
		const double DistanceToTarget = FVector::Dist(GetActorLocation(), CombatTarget->GetActorLocation());
		if (DistanceToTarget>CombatRadius||!bIsAlive)
		{
			CombatTarget = nullptr; //如果目标距离过远, 则清除战斗目标
			if (HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(false); //隐藏血条
			}
		}
		else
		{
			if(HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(true); //显示血条
			}
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

