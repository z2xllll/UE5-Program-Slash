

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
	//���ö�ȫ��ͨ������ײ��ӦΪ�ص�
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	//���ö���ҵ���ײ��ӦΪ����
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
	SetOwner(NewActor); //����������ӵ����
	SetInstigator(NewInstigater); //����������ʩ����
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
		//ͣ�û���Ч
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
	//��������ö�����ͣ��������
	ItemMesh->AttachToComponent(InParent, FATR, InSocketName);
}


void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // ��� OtherActor �Ƿ�Ϊ��
    if (!OtherActor)return;
    if (OtherActor->IsA<ATreasure>())return;
	if (ActorIsSameType(OtherActor,FName("Enemy")))return;

    FHitResult BoxHit;
    BoxTrace(BoxHit);
    AActor* HitActor = BoxHit.GetActor();

    // ��� HitActor �Ƿ�Ϊ��
    if (!HitActor) return;
    if (ActorIsSameType(HitActor, FName("Enemy")))return;
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

    IHitInterface* HitInterface = Cast<IHitInterface>(HitActor);
    if (HitInterface)
    {
        HitInterface->Execute_GetHit(HitActor, BoxHit.ImpactPoint,GetOwner());
        CreateFields(BoxHit.ImpactPoint);
    }
}

bool AWeapon::ActorIsSameType(AActor* OtherActor, FName Tag)
{
	if (GetOwner() == nullptr || OtherActor == nullptr) return false; //���ӵ���ߺ�����Actor�Ƿ�Ϊ��
    return GetOwner()->ActorHasTag(Tag) && OtherActor->ActorHasTag(Tag); //ͬ��֮�䲻����
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
    // �������Ƿ�Ϊ��
    if (!BoxTraceStart || !BoxTraceEnd)return;

    const FVector Start = BoxTraceStart->GetComponentLocation();
    const FVector End = BoxTraceEnd->GetComponentLocation();
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.AddUnique(this);
	if (GetOwner() != nullptr)
	ActorsToIgnore.AddUnique(GetOwner()); // ���ӵ���ߵ������б�

    for (auto& Actor : IgnoreActors)
    {
        if (Actor) // ��� Actor �Ƿ�Ϊ��
        {
            ActorsToIgnore.AddUnique(Actor);
        }
    }

    UKismetSystemLibrary::BoxTraceSingle(
        this,
        Start,
        End,
        FVector(10.f, 10.f, 10.f),
        BoxTraceStart->GetComponentRotation(),
        ETraceTypeQuery::TraceTypeQuery1,
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        BoxHit,
        true
    );

    // ֻ�е����е�Actor��Ϊ��ʱ����ӵ������б�
	if (BoxHit.GetActor() == nullptr) return;
    AActor* HitActor = BoxHit.GetActor();
    if (HitActor)IgnoreActors.AddUnique(HitActor);
}


