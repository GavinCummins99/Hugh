// Fill out your copyright notice in the Description page of Project Settings.

#include "HLE_Movement.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values for this component's properties
UHLE_Movement::UHLE_Movement(){
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}



// Called when the game starts
void UHLE_Movement::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHLE_Movement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHLE_Movement::Pan(const FInputActionValue& Value) {
	PanButtonPressed = Value.Get<bool>();
}

void UHLE_Movement::CanLook(const FInputActionValue& Value) {
	b_CanLook = Value.Get<bool>();
}

//Orbits the camera
void UHLE_Movement::RotateCamera(const FInputActionValue& Value) {
	if (PanButtonPressed) {
		FVector2D PanVector = Value.Get<FVector2D>();

		FVector ForwardVector = CameraArm->GetForwardVector();
		// Remove the pitch component to keep movement in the horizontal plane
		ForwardVector.Z = 0;
		ForwardVector.Normalize();
		FVector RightVector = FVector::CrossProduct(ForwardVector, FVector::UpVector);
		FVector PanDirection = (-ForwardVector * PanVector.Y + RightVector * PanVector.X);
		
		CameraArm->AddWorldOffset(PanDirection * PanStrenght * PanDir * (PanDir == -1? 5 : 1));
	}
	else if (b_CanLook) {
		FVector2D LookAxisVector = Value.Get<FVector2D>() * LookSensitiviy;
		//CameraArm->SetWorldRotation(CameraArm->GetComponentRotation() + FRotator(LookAxisVector.Y,LookAxisVector.X,0));

		// Get current rotation
		FRotator CurrentRotation = CameraArm->GetComponentRotation();

		// Calculate new rotation
		FRotator DeltaRotation = FRotator(LookAxisVector.Y, LookAxisVector.X, 0);
		FRotator NewRotation = CurrentRotation + DeltaRotation;

		// Clamp pitch between 0 and 85 degrees
		NewRotation.Pitch = FMath::ClampAngle(NewRotation.Pitch, -85.0f, -5.0f);

		// Apply the new rotation
		CameraArm->SetWorldRotation(NewRotation);
	} 
}

void UHLE_Movement::Zoom(const FInputActionValue& Value) {
	float ZoomAmount = Value.Get<float>() * ZoomSensitivity;
	GEngine->AddOnScreenDebugMessage(10, 10, FColor::Red, "Zoom" + FString::SanitizeFloat(ZoomAmount));

	CameraArm->TargetArmLength -= ZoomAmount;
}
