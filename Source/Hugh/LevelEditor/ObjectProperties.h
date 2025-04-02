// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectProperties.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObjectPlaced);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartedPushing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStoppedPushing);


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
	UPROPERTY()FVector Target = FVector(0,0,220);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	//Objects settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Display settings") FString ObjectName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Display settings") UTexture2D* ObjectImage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FColor ObjectColor;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool Pushable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Object settings") bool AllowRotation = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Object settings") bool AlignNormalToface = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Object settings") FVector GridSnap = FVector(1,1,1);

	//Public properties
	UPROPERTY(BlueprintReadWrite) bool EmittingPower = false;


	UPROPERTY(BlueprintAssignable) FObjectPlaced OnObjectPlaced;
	UPROPERTY(BlueprintReadOnly) bool IsPlaced;
	UPROPERTY(BlueprintAssignable) FObjectPlaced OnStartedPushing;
	UPROPERTY(BlueprintAssignable) FObjectPlaced OnOStoppedPushing;

	UFUNCTION(BlueprintCallable) void OnPlaced();
	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool IsPowered = false;

		
};
