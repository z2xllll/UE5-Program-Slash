// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Items/Treasure.h"
#include "Components/CapsuleComponent.h" // Ensure this header is included to resolve the incomplete type error

ABreakableActor::ABreakableActor()
{
	//Dont tick by default
	PrimaryActorTick.bCanEverTick = false;

	//Create the geometry collision component
	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	//Set the geometry collision component as the root component
	SetRootComponent(GeometryCollection);
	//�����ص��¼�
	GeometryCollection->SetGenerateOverlapEvents(true);
	//�������
	GeometryCollection->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//����pawn
	GeometryCollection->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// Create and attach the capsule component
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{  
	if (IsBroken)return;//����Ѿ��ƻ��˾Ͳ��ٴ���
	IsBroken = true; //����Ϊ���ƻ�״̬
    UWorld* World = GetWorld();  
    if (World && TreasureClasses.Num()>0)
    {  
        FVector Location = GetActorLocation();  
        Location.Z += 30.f;  
		
        //FActorSpawnParameters SpawnParams;  
        //SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;  
		int32 Index = FMath::RandRange(0, TreasureClasses.Num() - 1);
        World->SpawnActor<ATreasure>(TreasureClasses[Index], Location, GetActorRotation());
    }  
}