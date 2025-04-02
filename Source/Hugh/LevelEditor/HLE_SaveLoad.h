// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HLE_SaveLoad.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FLevelNamesCallback, const TArray<FString>&, LevelNames);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUGH_API UHLE_SaveLoad : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHLE_SaveLoad();

	//UFUNCTION(BlueprintCallable )void LoadLevel(FString LevelName, FString LoadPath);
	//UFUNCTION(BlueprintCallable) void SaveLevel(FString LevelName, FString SavePath) const;
	UFUNCTION(BlueprintCallable) void SaveLevel(FString LevelName) const;

	UFUNCTION(BlueprintCallable, Category = "Level Editor|Save Load")

	
	void GetLevelNames(const FLevelNamesCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Level Editor|Save Load")
	void LoadLevel(FString LevelName);

};
