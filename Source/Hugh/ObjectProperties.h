// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectProperties.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUGH_API UObjectProperties : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UObjectProperties();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString ObjectName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UTexture2D* ObjectImage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere) FColor ObjectColor;
	UPROPERTY(BlueprintReadWrite) bool EmittingLight = false;

		
};
