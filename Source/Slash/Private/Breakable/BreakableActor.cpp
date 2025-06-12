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
	//允许重叠事件
	GeometryCollection->SetGenerateOverlapEvents(true);
	//忽视相机
	GeometryCollection->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//忽视pawn
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
	if (IsBroken)return;//如果已经破坏了就不再处理
	IsBroken = true; //设置为已破坏状态
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