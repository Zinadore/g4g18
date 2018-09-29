// Fill out your copyright notice in the Description page of Project Settings.
#pragma optimize("", off)
#include "AMazeBuilder.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include <cstdlib>

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
}

// Called when the game starts or when spawned
void AMazeBuilder::BeginPlay()
{
	Super::BeginPlay();
	KnockDemWallsDown();
}

// Called every frame
void AMazeBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMazeBuilder::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);
}

void AMazeBuilder::KnockDemWallsDown()
{

	auto first_index = rand() % this->cells.Num();

	auto first_cell = this->cells[first_index];
	first_cell->InMaze = true;

	TArray<AMazeCell*> frontier;

	for (int i = 0; i < first_cell->Neighbors.Num(); i++) {
		auto neighbor = first_cell->Neighbors[i];
		if (!frontier.Contains(neighbor) && !neighbor->InMaze) {
			frontier.Add(neighbor);
		}
	}

	auto previous_cell = first_cell;
	while (frontier.Num())
	{
		int random_index = rand() % frontier.Num();

		auto current_cell = frontier[random_index];
		current_cell->InMaze = true;
		frontier.RemoveAt(random_index);
		// Find to which of the neighbors we want to be connected
		current_cell->ConnectToFirstInNeighbor();

		for (int i = 0; i < current_cell->Neighbors.Num(); i++) {
			auto neighbor = current_cell->Neighbors[i];
			if (!frontier.Contains(neighbor) && !neighbor->InMaze) {
				frontier.Add(neighbor);
			}
		}

		previous_cell = current_cell;
	}
}

void AMazeBuilder::BuildMaze(bool force)
{
	bool shouldRebuild = this->oldWidth != this->MazeWidth ||
		this->oldHeight != this->MazeHeight ||
		this->oldTileSize != this->TileSize ||
		force;

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

		for (int y = 0; y < this->MazeWidth; y++) {
			for (int x = 0; x < this->MazeHeight; x++) {
				FVector location(x * this->TileSize, y * this->TileSize, 0.0f);
				auto c = GetWorld()->SpawnActor<AMazeCell>(location, rotation, spawnInfo);
				this->cells[y * this->MazeHeight + x] = c;
				c->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
				c->SetActorLabel(FString(TEXT("Cell_")) + FString::FromInt(x) + FString(" , ") + FString::FromInt(y));
				c->EnableWall(Wall::North, this->TileSize);
				c->EnableWall(Wall::West, this->TileSize);

				if (y == this->MazeWidth - 1) {
					c->EnableWall(Wall::East, this->TileSize);
				}

				if (x == 0) {
					c->EnableWall(Wall::South, this->TileSize);
				}
			}
		}

		this->oldTileSize = this->TileSize;
		this->oldWidth = this->MazeWidth;
		this->oldHeight = this->MazeHeight;

		for (int y = 0; y < this->MazeWidth; y++) {
			for (int x = 0; x < this->MazeHeight; x++) {
				this->cells[y * this->MazeHeight + x]->BuildNeighbors(x, y, this->MazeWidth, this->MazeHeight, this->cells);
			}
		}
	}
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

void AMazeBuilder::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//for (int i = 0; i < cells.Num(); ++i) {
	//	auto w = cells[i];

	//	if (w != nullptr) {
	//		w->Destroy();
	//	}
	//}
	BuildMaze(true);
}

