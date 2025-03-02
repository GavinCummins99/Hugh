// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HLE_Movement.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUGH_API UHLE_Movement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHLE_Movement();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void Pan(const FInputActionValue& Value);

	bool PanButtonPressed;
	bool b_CanLook;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	//UPROPERTY(BlueprintReadWrite, EditAnywhere) UCameraComponent* EditorCamera;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USpringArmComponent* CameraArm;

	//Functions 
	void RotateCamera(const FInputActionValue& Value);
	UFUNCTION()void Zoom(const FInputActionValue& Value);
	void CanLook(const FInputActionValue& Value);
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UCameraComponent* EditorCamera;


	//Movement settings
	float PanStrenght = 40;
	float PanDir = 1;
	float LookSensitiviy = 3;
	float ZoomSensitivity = 75;
};
