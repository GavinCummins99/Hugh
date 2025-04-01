#include "HLE_Movement.h"
#include "InputActionValue.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values for this component's properties
UHLE_Movement::UHLE_Movement(){
	PrimaryComponentTick.bCanEverTick = false;
}

//Set weather the pan button is pressed 
void UHLE_Movement::Pan(const FInputActionValue& Value) {
	PanButtonPressed = Value.Get<bool>();
}

//Set weather the orbit button is pressed
void UHLE_Movement::CanLook(const FInputActionValue& Value) {
	OrbitButtonPressed = Value.Get<bool>();
}

//Orbits the camera
void UHLE_Movement::OribitCamera(const FInputActionValue& Value) {
	GEngine->AddOnScreenDebugMessage(110, 5, FColor::Red, "We are here");
	if (PanButtonPressed) {
		
		FVector2D PanVector = Value.Get<FVector2D>();

		FVector ForwardVector = CameraArm->GetForwardVector();
		// Remove the pitch component to keep movement in the horizontal plane
		ForwardVector.Z = 0;
		ForwardVector.Normalize();
		FVector RightVector = FVector::CrossProduct(ForwardVector, FVector::UpVector);
		FVector PanDirection = (-ForwardVector * PanVector.Y + RightVector * PanVector.X);
		
		CameraArm->AddWorldOffset(PanDirection * PanStrength * PanDir * (PanDir == -1? 5 : 1));
	}
	else if (OrbitButtonPressed) {
		FVector2D LookAxisVector = Value.Get<FVector2D>() * LookSensitivity;
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
	const float ZoomDelta = Value.Get<float>() * ZoomSensitivity;
	if(CameraArm) CameraArm->TargetArmLength -= ZoomDelta;
}
