#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HLE_Movement.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUGH_API UHLE_Movement : public UActorComponent {
	GENERATED_BODY()

protected:

	bool PanButtonPressed;
	bool OrbitButtonPressed;

public:	
	// Sets default values for this component's properties
	UHLE_Movement();

	//Objects refs 
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USpringArmComponent* CameraArm;

	//Functions 
	void Pan(const FInputActionValue& Value);
	void OribitCamera(const FInputActionValue& Value);
	void Zoom(const FInputActionValue& Value);
	void CanLook(const FInputActionValue& Value);
	
	//Movement settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera settings")float PanStrength = 40;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera settings")float PanDir = 1;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera settings")float LookSensitivity = 3;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Camera settings")float ZoomSensitivity = 75;
};
