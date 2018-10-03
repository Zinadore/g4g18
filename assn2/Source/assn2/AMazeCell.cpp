// Fill out your copyright notice in the Description page of Project Settings.
#pragma optimize("", off)
#include "AMazeCell.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstance.h" 
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AMazeCell::AMazeCell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Engine/BasicShapes/Plane"));
	StaticMesh = MeshRef.Object;
	check(StaticMesh != nullptr);
	

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialRef(TEXT("Material'/Game/WallMaterial.WallMaterial'"));
	Material = MaterialRef.Object;
	check(Material != nullptr);

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"), true);
	check(SceneComponent != nullptr);
	this->RootComponent = SceneComponent;
	this->WallComponents.SetNum(5);

	static ConstructorHelpers::FObjectFinder<UMaterial> FloorMaterialRef(TEXT("Material'/Game/FloorMaterial.FloorMaterial'"));
	auto FloorMaterial = FloorMaterialRef.Object;
	check(FloorMaterial != nullptr);

	auto f = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	f->SetStaticMesh(StaticMesh);
	f->SetupAttachment(RootComponent);
	f->CreateAndSetMaterialInstanceDynamicFromMaterial(0, FloorMaterial);
	//f->RegisterComponent();
	WallComponents[Wall::Floor] = f;
}

// Called when the game starts or when spawned
void AMazeCell::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMazeCell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMazeCell::EnableWall(Wall wall, int tileSize)
{
	auto existing_m = this->WallComponents[wall];

	if (existing_m) {
		existing_m->SetStaticMesh(StaticMesh);
		return;
	}

	auto parentName = GetFName().ToString();
	auto name = FString::Printf(TEXT("%s Wall %d"), *parentName, wall);
	auto m = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), FName(*name), RF_Transactional);
	this->WallComponents[wall] = m;

	m->SetStaticMesh(StaticMesh);
	m->SetupAttachment(RootComponent);
	m->CreateAndSetMaterialInstanceDynamicFromMaterial(0, Material);
	m->RegisterComponent();

	switch (wall) {
		case Wall::North: {
			m->SetRelativeLocation(FVector(tileSize / 2.0f, 0.0f, tileSize / 2.0f));
			FQuat rot = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 90.0f));
			m->AddRelativeRotation(rot);
			break;
		}
		case Wall::East: {
			m->SetRelativeLocation(FVector(0.0f, tileSize / 2.0f, tileSize / 2.0f));
			FQuat rot = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 0.0f));
			m->AddRelativeRotation(rot);
			break;
		}
		case Wall::South: {
			m->SetRelativeLocation(FVector(-tileSize / 2.0f, 0.0f, tileSize / 2.0f));
			FQuat rot = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 270.0f));
			m->AddRelativeRotation(rot);
			break;
		}
		case Wall::West: {
			m->SetRelativeLocation(FVector(0.0f, -tileSize / 2.0f, tileSize / 2.0f));
			FQuat rot = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 180.0f));
			m->AddRelativeRotation(rot);
			break;
		}
	}
}
/**
* Will disable the wall specified. If the wall was not enabled to begin with, it does nothing
*
* @param Wall type to destroy.
*/
void AMazeCell::DisableWall(Wall wall) {
	auto w = WallComponents[wall];

	if (w!= nullptr) {
		w->SetStaticMesh(0);
	}
}

void AMazeCell::BuildNeighbors(int x, int y, int width, int height, TArray<AMazeCell*> cells)
{
	this->x = x;
	this->y = y;

	//Add the cell above us to the neighbor list, if applicable
	auto north_x = x + 1;
	if (north_x < height) {
		this->Neighbors.Add(cells[y * height + north_x]);
	}

	//Add the cell to the right of us to the neighbor list, if applicable
	auto east_y = y + 1;
	if (east_y < width) {
		this->Neighbors.Add(cells[east_y * height + x]);
	}

	//Add the cell below us to the neighbor list, if applicable
	auto south_x = x - 1;
	if (south_x >= 0) {
		this->Neighbors.Add(cells[y * height + south_x]);
	}

	//Add the cell to the left of us to the neighbor list, if applicable
	auto west_y = y - 1;
	if (west_y >= 0) {
		this->Neighbors.Add(cells[west_y * height + x]);
	}
}

void AMazeCell::ConnectToFirstInNeighbor()
{
	for (int i = 0; i < this->Neighbors.Num(); i++)
	{
		auto n = this->Neighbors[i];

		if (n->InMaze) {
			if (n->x > this->x) {
				this->DisableWall(Wall::North);
				n->DisableWall(Wall::South);
			}

			if (n->x < this->x) {
				this->DisableWall(Wall::South);
				n->DisableWall(Wall::North);
			}

			if (n->y > this->y) {
				this->DisableWall(Wall::East);
				n->DisableWall(Wall::West);
			}

			if (n->y < this->y) {
				this->DisableWall(Wall::West);
				n->DisableWall(Wall::East);
			}
			break;
		}
	}
}
