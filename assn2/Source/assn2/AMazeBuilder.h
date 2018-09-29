// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AMazeCell.h"
#include "AMazeBuilder.generated.h"

class UStaticMesh;
class UMaterial;
class UInstancedStaticMeshComponent;

UCLASS()
class ASSN2_API AMazeBuilder : public AActor
{
	GENERATED_BODY()

private:
	
	int32 oldTileSize = 0;
	int32 oldWidth = 0;
	int32 oldHeight = 0;
	TArray<AMazeCell*> cells;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
		int MazeWidth = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
		int MazeHeight = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
		int TileSize = 100;
	// Sets default values for this actor's properties
	AMazeBuilder();
	~AMazeBuilder();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void KnockDemWallsDown();
	void BuildMaze(bool force = false);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void Destroyed() override;
	virtual void PostInitializeComponents() override;
};
