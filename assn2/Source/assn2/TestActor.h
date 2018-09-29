// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Array.h"
#include "AMazeCell.h"
#include "TestActor.generated.h"

UCLASS()
class ASSN2_API ATestActor : public AActor
{
	GENERATED_BODY()
		TArray<AMazeCell*> cells;
public:	
	// Sets default values for this actor's properties
	ATestActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
		
};
