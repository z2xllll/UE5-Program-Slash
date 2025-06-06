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
	//�����ڵ����
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//�����ص��¼�
	GetMesh()->SetGenerateOverlapEvents(true);
	//�����ڵ����
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

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
	if(HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false); //��ʼ����Ѫ��
	}
	EnemyController = Cast<AAIController>(GetController()); //��ȡAI������
	MoveToTarget(PatrolTarget); //��ʼѲ��

	if (PawnSensing)
	{
		//���ø�֪�¼�
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen); //�󶨸�֪�¼�
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
		MoveRequest.SetAcceptanceRadius(20.f); //���ý��ܰ뾶, 20��λ
		EnemyController->MoveTo(MoveRequest); //��ʼ�ƶ���Ѳ��Ŀ��
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (EnemyState == EEnemyState::EES_Chasing) return; //�������״̬Ϊ׷��, �򲻴����֪�¼�
	if (SeenPawn->ActorHasTag(FName("SlashCharacter")))
	{
		EnemyState = EEnemyState::EES_Chasing; //���õ���״̬Ϊ����
		GetWorldTimerManager().ClearTimer(PatrolTimer); //���Ѳ�߼�ʱ��
		CombatTarget = SeenPawn; //����ս��Ŀ��Ϊ����֪�Ľ�ɫ
		MoveToTarget(CombatTarget); //�ƶ���ս��Ŀ��
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget); //Ѳ�߼�ʱ��������, �ƶ���Ѳ��Ŀ��
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsAlive) return; //��������Ѿ�����, ��ִ�������߼�

	if (CombatTarget)
	{
		if (!InTargetRange(CombatTarget,CombatRadius))
		{
			//ֹ֮ͣǰ���ƶ�
			if (EnemyController)
			{
				EnemyController->StopMovement();
			}
			CombatTarget = nullptr; //���Ŀ������Զ, �����ս��Ŀ��
			EnemyState = EEnemyState::EES_Patrolling; //���õ���״̬ΪѲ��
            GetCharacterMovement()->MaxWalkSpeed = 150.f; // ����Ѳ���ٶ�
			if (HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(false); //����Ѫ��
			}
			MoveToTarget(PatrolTarget);
		}
		else if(!InTargetRange(CombatTarget,AttackRadius)&&InTargetRange(CombatTarget,CombatRadius))
		{
			EnemyState = EEnemyState::EES_Chasing; //���õ���״̬Ϊ׷��
			GetCharacterMovement()->MaxWalkSpeed = 300.f; // ����׷���ٶ�
			if (HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(true); //��ʾѪ��
			}
			MoveToTarget(CombatTarget); //�ƶ���ս��Ŀ��
		}
		else if (InTargetRange(CombatTarget, AttackRadius))
		{
			EnemyState = EEnemyState::EES_Attacking; //���õ���״̬Ϊ����
			if(HealthBarWidget)
			{
				HealthBarWidget->SetVisibility(true); //��ʾѪ��
			}
		}
	}
	else if(EnemyState!= EEnemyState::EES_Patrolling)
	{
		//���û��ս��Ŀ��, �����õ���״̬ΪѲ��
		EnemyState = EEnemyState::EES_Patrolling; //���õ���״̬ΪѲ��
		GetCharacterMovement()->MaxWalkSpeed = 150.f; // ����Ѳ���ٶ�
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false); //����Ѫ��
		}
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
		//��������Ѿ�����, ������������
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
	CombatTarget = EventInstigator->GetPawn(); //����ս��Ŀ��Ϊ������
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
	//���������Ƿ�����ֵ, ����ǿ����ɸ���
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
	//������������
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		HealthBarWidget->SetVisibility(false); //����Ѫ��
		AnimInstance->Montage_Play(DeathMontage);
		AnimInstance->Montage_JumpToSection(FName("Death"),DeathMontage);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //������ײ
		SetLifeSpan(5.f); //����������, 5�������
	}
}

