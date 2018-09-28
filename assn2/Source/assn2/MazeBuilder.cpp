// Fill out your copyright notice in the Description page of Project Settings.
#include "MazeBuilder.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

// Sets default values
AMazeBuilder::AMazeBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ROOT"), true);
	check(SceneComponent != nullptr);
	this->RootComponent = SceneComponent;
}

AMazeBuilder::~AMazeBuilder()
{
	UE_LOG(LogTemp, Warning, TEXT("~MazeBuilder() called"))
	//if (this->cells.Num) {
	//	for (int i = 0; i < this->numTiles; ++i) {
	//		auto c = this->cells[i];
	//		this->cells[i] = nullptr;
	//		c->Destroy();
	//	}
	//}
}

// Called when the game starts or when spawned
void AMazeBuilder::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMazeBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMazeBuilder::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);



	int32 array_size = this->MazeWidth * this->MazeHeight;
	bool shouldRebuild = this->numTiles != array_size || this->oldTileSize != this->TileSize;

	if (shouldRebuild) {
		if (this->cells.Num() != 0) {
			for (int i = 0; i < this->cells.Num(); i++) {
				auto temp = this->cells[i];
				temp->Destroy();
			}

			if (oldHeight != MazeHeight || oldWidth != MazeWidth) {
				this->cells.SetNum(this->MazeWidth * this->MazeHeight);
			}
		}
		else {
			this->cells.SetNum(this->MazeWidth * this->MazeHeight);
		}

		FRotator rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters spawnInfo;
		//spawnInfo.Owner = this;
		//spawnInfo.bDeferConstruction = false;

		for (int y = 0; y < this->MazeHeight; y++) {
			for (int x = 0; x < this->MazeWidth; x++) {
				FVector location(x * this->TileSize, y * this->TileSize, 0.0f);
				auto c = GetWorld()->SpawnActor<AMazeCell>(location, rotation, spawnInfo);
				this->cells[y * this->MazeWidth + x] = c;
				c->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
				c->EnableWall(Wall::North, this->TileSize);
				c->EnableWall(Wall::West, this->TileSize);

				if (x == this->MazeWidth - 1) {
					c->EnableWall(Wall::East, this->TileSize);
				}

				if (y == this->MazeHeight - 1) {
					c->EnableWall(Wall::South, this->TileSize);
				}
			}
		}
		this->numTiles = array_size;
		this->oldTileSize = this->TileSize;
		this->oldWidth = this->MazeWidth;
		this->oldHeight = this->MazeHeight;
	}
	
}

void AMazeBuilder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogTemp, Warning, TEXT("EndPlay() called"))

	if (this->cells.Num() != 0) {
		for (int i = 0; i < this->cells.Num(); ++i) {
			auto c = this->cells[i];
			c->Destroy();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AMazeBuilder::Destroyed() {
	Super::Destroyed();
	if (this->cells.Num() != 0) {
		for (int i = 0; i < this->cells.Num(); ++i) {
			auto c = this->cells[i];
			c->Destroy();
		}
	}
}

