// Fill out your copyright notice in the Description page of Project Settings.

#include "TestActor.h"


// Sets default values
ATestActor::ATestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	cells.SetNum(2);
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ROOT"), true);
	check(SceneComponent != nullptr);
	this->RootComponent = SceneComponent;
}

// Called when the game starts or when spawned
void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	FRotator rotation(0.0f, 0.0f, 0.0f);
	FActorSpawnParameters spawnInfo;

	for (int i = 0; i < cells.Num(); ++i) {
		FVector location(i * 100, 0.0f, 0.0f);
		auto c = GetWorld()->SpawnActor<AMazeCell>(location, rotation, spawnInfo);
		this->cells[i] = c;
		c->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
		c->SetActorLabel(FString(TEXT("Cell_")) + FString::FromInt(i) + FString(" , ") + FString::FromInt(0));
		c->EnableWall(Wall::North, 100);
		c->EnableWall(Wall::West, 100);
		c->EnableWall(Wall::East, 100);
		c->EnableWall(Wall::South, 100);
	}

	cells[0]->DisableWall(Wall::East);
	cells[1]->DisableWall(Wall::West);
	
}

// Called every frame
void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

