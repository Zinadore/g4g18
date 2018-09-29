// Fill out your copyright notice in the Description page of Project Settings.

#include "MyActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstance.h" 
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Engine/BasicShapes/Plane"));
	UStaticMesh* StaticMesh = MeshRef.Object;
	check(StaticMesh != nullptr);

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialRef(TEXT("Material'/Game/WallMaterial.WallMaterial'"));
	UMaterial* Material = MaterialRef.Object;
	//check(Material != nullptr);

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"), true);
	check(SceneComponent != nullptr);
	this->RootComponent = SceneComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	//StaticMeshComponent->SetMobility(EComponentMobility::Movable);
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetStaticMesh(StaticMesh);
	StaticMeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Material);

	//StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	//StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	//StaticMeshComponent->SetGenerateOverlapEvents(false);
	//StaticMeshComponent->SetCanEverAffectNavigation(false);
	//StaticMeshComponent->bCastDynamicShadow = false;
	//StaticMeshComponent->bCastStaticShadow = false;
	//StaticMeshComponent->bAffectDistanceFieldLighting = false;
	//StaticMeshComponent->bAffectDynamicIndirectLighting = false;

	//auto tileSize = 100;
	//StaticMeshComponent->SetRelativeLocation(FVector(0.0f, tileSize / 2.0f, tileSize / 2.0f));
	//FQuat rot = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 0.0f));
	//StaticMeshComponent->AddRelativeRotation(rot);
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	//StaticMeshComponent->SetStaticMesh(nullptr);	
	
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

