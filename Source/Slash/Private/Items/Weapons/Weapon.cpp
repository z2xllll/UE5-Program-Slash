

#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Interfaces/HitInterface.h"
#include "Items/Treasure.h"
#include "NiagaraComponent.h"
#include "Enemy/Enemy.h"

AWeapon::AWeapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//设置对全部通道的碰撞响应为重叠
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	//设置对玩家的碰撞响应为无视
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName,AActor* NewActor, APawn* NewInstigater)
{
	SetOwner(NewActor); //设置武器的拥有者
	SetInstigator(NewInstigater); //设置武器的施法者
	ItemState = EItemState::EIS_Equipped;
	AttachMeshToSocket(InParent, InSocketName);
	DisableSphereCollision();
	PlayEquipSound();
	DeactivateEmbers();
}

void AWeapon::DeactivateEmbers()
{
	if (EmbersEffect)
	{
		//停用火花特效
		EmbersEffect->Deactivate();
	}
}

void AWeapon::DisableSphereCollision()
{
	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::PlayEquipSound()
{
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules FATR(EAttachmentRule::SnapToTarget, true);
	//吸附对象，枚举类型，插槽名称
	ItemMesh->AttachToComponent(InParent, FATR, InSocketName);
}


void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 检查 OtherActor 是否为空
    if (!OtherActor)
    {
        return;
    }

    if (OtherActor->IsA<ATreasure>())
    {
        return;
    }

    FHitResult BoxHit;
    BoxTrace(BoxHit);

    AActor* HitActor = BoxHit.GetActor();

    // 检查 HitActor 是否为空
    if (!HitActor)
    {
        return;
    }

	UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
    if (HitActor->IsA<AEnemy>())
    {
        // 检查 Instigator 和 Controller 是否为空
        APawn* InstigatorPawn = GetInstigator();
        if (InstigatorPawn && InstigatorPawn->GetController())
        {
            UGameplayStatics::ApplyDamage(
                HitActor,
                Damage,
                InstigatorPawn->GetController(),
                this,
                UDamageType::StaticClass()
            );
        }
    }

    IHitInterface* HitInterface = Cast<IHitInterface>(HitActor);
    if (HitInterface)
    {
        HitInterface->Execute_GetHit(HitActor, BoxHit.ImpactPoint);
        CreateFields(BoxHit.ImpactPoint);
    }
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
    // 检查组件是否为空
    if (!BoxTraceStart || !BoxTraceEnd)
    {
        return;
    }

    const FVector Start = BoxTraceStart->GetComponentLocation();
    const FVector End = BoxTraceEnd->GetComponentLocation();
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.AddUnique(this);

    for (auto& Actor : IgnoreActors)
    {
        if (Actor) // 检查 Actor 是否为空
        {
            ActorsToIgnore.AddUnique(Actor);
        }
    }

    UKismetSystemLibrary::BoxTraceSingle(
        this,
        Start,
        End,
        FVector(8.f, 8.f, 8.f),
        BoxTraceStart->GetComponentRotation(),
        ETraceTypeQuery::TraceTypeQuery1,
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        BoxHit,
        true
    );

    // 只有当击中的Actor不为空时才添加到忽略列表
    AActor* HitActor = BoxHit.GetActor();
    if (HitActor)
    {
        IgnoreActors.AddUnique(HitActor);
    }
}


