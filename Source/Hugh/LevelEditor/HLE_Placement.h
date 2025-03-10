// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HLE_Placement.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUGH_API UHLE_Placement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHLE_Placement();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//Cursor locations 
	FVector CursorLoc;
	FVector CursorStartLoc;
	FVector CursorEndLoc;
	
	int XLen;
	int YLen;
	int CachedXLen;
	int CachedYLen;
	TMap<FIntPoint, AActor*> SpawnedObjects;

	bool IsPlacing;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere) AActor* CurrentObject;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) AActor* PlaceTemp;
	
	void Trace();
	void StartPlacement();
	void EndPlacement();
	void Test();
	void CreateObjects();
	FVector Snap(FVector InVector);
};

