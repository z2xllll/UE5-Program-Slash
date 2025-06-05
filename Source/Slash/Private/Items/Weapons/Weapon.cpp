

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
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
    if (EmbersEffect)
    {
		//停用火花特效
        EmbersEffect->Deactivate();
    }
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules FATR(EAttachmentRule::SnapToTarget, true);
	//吸附对象，枚举类型，插槽名称
	ItemMesh->AttachToComponent(InParent, FATR, InSocketName);
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}
void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

    if (OtherActor->IsA<ATreasure>())
    {
        return;
    }

    const FVector Start = BoxTraceStart->GetComponentLocation();
    const FVector End = BoxTraceEnd->GetComponentLocation();
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.AddUnique(this);
    for(auto &Actor: IgnoreActors)
    {
        ActorsToIgnore.AddUnique(Actor);
	}

    FHitResult BoxHit;
    bool bHit = UKismetSystemLibrary::BoxTraceSingle(
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

        AActor* HitActor = BoxHit.GetActor();
        bool bAlreadyIgnored = IgnoreActors.Contains(HitActor);

        if (bAlreadyIgnored||HitActor==nullptr)
        {
            return; // 如果已经在列表中，直接返回
        }

		if (HitActor->IsA<AEnemy>())
		{
			//如果是敌人，应用伤害
			UGameplayStatics::ApplyDamage(
				HitActor,
				Damage,
				GetInstigator()->GetController(),
				this,
				UDamageType::StaticClass()
			);
		}

        IHitInterface* HitInterface = Cast<IHitInterface>(HitActor);
        if (HitInterface)
        {
            HitInterface->Execute_GetHit(HitActor, BoxHit.ImpactPoint);
        }

        CreateFields(BoxHit.ImpactPoint);

		IgnoreActors.AddUnique(HitActor); // 将击中的Actor添加到忽略列表中
}



