// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/RandomStream.h"
#include "assn3_bluenoiseBPLibrary.generated.h"

USTRUCT(BlueprintType)
struct FBluenoiseContainer {
	GENERATED_USTRUCT_BODY()

private:
	FRandomStream m_Stream;
	TArray<FVector> m_ExistingPoints;

public:
	void Initialize(int32 Seed) { m_Stream.Initialize(Seed); }
	friend class Uassn3_bluenoiseBPLibrary;
};


/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class Uassn3_bluenoiseBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, category = "Blue Noise|assn3", meta = (Seed = "1992", Keywords = "Blue Noise assn3 seed", DisplayName = "Set Blue Noise Seed"))
	static void InitializeContainer(UPARAM(ref) FBluenoiseContainer& Container, int32 Seed);

	UFUNCTION(BlueprintPure,
			  category = "Blue Noise|assn3",
			  meta = (Min = "0",
					  Max = "1000",
					  Keywords = "Blue Noise random point",
					  DisplayName = "Get New Blue Noise Point")
	)
	static FVector GetPoint(UPARAM(ref) FBluenoiseContainer& Container, float Min, float Max);
};
