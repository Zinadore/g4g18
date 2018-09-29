// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Array.h"
#include "AMazeCell.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterial;

UENUM()
enum Wall
{
	North	= 0 UMETA(DisplayName = "North Wall"),
	East	= 1 UMETA(DisplayName = "East Wall"),
	South	= 2 UMETA(DisplayName = "South Wall"),
	West	= 3 UMETA(DisplayName = "West Wall"),
	Floor	= 4 UMETA(DisplayName = "Floor")
};

UCLASS()
class ASSN2_API AMazeCell : public AActor
{
	GENERATED_BODY()

private:
	USceneComponent * SceneComponent;
	UStaticMesh* StaticMesh;
	UMaterial* Material;
	
public:
	bool InMaze;
	TArray<AMazeCell*> Neighbors;
	TArray<UStaticMeshComponent*> WallComponents;

	int x;
	int y;

public:	
	// Sets default values for this actor's properties
	AMazeCell();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void EnableWall(Wall all, int tileSize);
	void DisableWall(Wall all);
	void BuildNeighbors(int x, int y, int width, int height, TArray<AMazeCell*> cells);
	void ConnectToFirstInNeighbor();
};
