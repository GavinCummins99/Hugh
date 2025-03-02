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

	bool DebugEnabled = false;
	void Push_Move(FVector TargetLocation);
	UFUNCTION() void OnParentHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);



public:	
	// Called every frame
	UPROPERTY()FVector Target;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	//Objects settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString ObjectName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UTexture2D* ObjectImage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FColor ObjectColor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool Pushable = false;
	
	//Public properties
	UPROPERTY(BlueprintReadWrite) bool EmittingLight = false;

		
};
