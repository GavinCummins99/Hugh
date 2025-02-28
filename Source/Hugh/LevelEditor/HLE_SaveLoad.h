// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HLE_SaveLoad.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUGH_API UHLE_SaveLoad : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHLE_SaveLoad();

	//UFUNCTION(BlueprintCallable )void LoadLevel(FString LevelName);
	UFUNCTION(BlueprintCallable) void SaveLevel(FString LevelName, FString SavePath) const;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
