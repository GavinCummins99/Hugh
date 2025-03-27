#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "HLE_Movement.h"
#include "HLE_Placement.h"
#include "InputActionValue.h"
#include "../LevelEditorGameMode.h"
#include "ObjectProperties.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "HLE_SaveLoad.h"
#include "HughLevelEditor.generated.h"


UENUM(BlueprintType)
enum class Modes : uint8  {Building, Removing, Editing, None};

USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FString ActorClass;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector Scale;
};

UCLASS()
class HUGH_API AHughLevelEditor : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHughLevelEditor();

	UPROPERTY(BlueprintReadWrite, EditAnywhere) UStaticMeshComponent* DisplayMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UCameraComponent* EditorCamera;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float GridSize = 100;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> CurrentObject;
	UFUNCTION(BlueprintCallable) void SetObjectIndex(int NewIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//UEnhancedInputComponent* Input;
	FVector2D SavedMouseLoc;
	FVector2D NewMouseLoc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* DefaultMappingContext;

	bool b_CanLook;
	TArray<AActor*> GhostObjects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input) class UInputAction* IA_Place;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input) class UInputAction* IA_Look;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input) class UInputAction* IA_Zoom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input) class UInputAction* IA_LookButton;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input) class UInputAction* IA_RotateObject;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input) class UInputAction* IA_PanButton;


	FVector StartPoint;
	FVector EndPoint;

	AActor* HoveredObject;
	bool EditingSelection;


	ALevelEditorGameMode* GM;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);

	UPROPERTY(BlueprintReadWrite) UHLE_SaveLoad* SaveLoad; 
	UPROPERTY(BlueprintReadWrite) UHLE_Movement* HLE_CameraComponent; 
	UPROPERTY(BlueprintReadWrite) UHLE_Placement* HLE_Placement; 

	UFUNCTION(BlueprintCallable) void SetInputMap();

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<AActor*> ObjectsInView;

	
	// This will create the variable type dropdown you're looking for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable Selection", meta = (PinType))
	FEdGraphPinType VariableType;

	FVector Snap(FVector InVector);
	void RotateObject();
	void Trace();
	void CanLook(const FInputActionValue& Value);
	void Pan(const FInputActionValue& Value);
	void RotateCamera(const FInputActionValue& Value);
	void Zoom(const FInputActionValue& Value);
	void CamCollision();
	UFUNCTION(BlueprintCallable) void ShowWalls();
	void StartPlacing(const FInputActionValue& Value);
	void PlaceObject(const FInputActionValue& Value);
	UFUNCTION(BlueprintCallable) void SetObject(int Index);
	UFUNCTION(BlueprintCallable)void SetMode(Modes NewMode);
	void GetActorsFromFolder(const FString& InFolderPath);
	UFUNCTION(BlueprintCallable) UObjectProperties* GetObjectProperties();
	UFUNCTION(BlueprintCallable) void RemoveSelectedObjects();
	UFUNCTION(BlueprintCallable) void ClearSelectedObjects();
	UFUNCTION(BlueprintCallable) void ReplaceSelectedObjects(AActor* NewObject);
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UMaterial* GhostMaterial;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UMaterial* RemovingMaterial;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UMaterial* EditingMaterial;

	void AddActorsToFolder(const TArray<AActor*>& ActorsToAdd, FName FolderName);

	
	float LookSensitiviy = 3;
	float ZoomSensitivity = 75;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) USpringArmComponent* CameraArm;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<AActor*> AllObjects;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int ObjectIndex;
	bool Placing;

	bool PanButtonPressed;

	float PanStrenght = 40;
	float PanDir = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) Modes EditorMode;
	UPROPERTY(BlueprintReadOnly) TArray<AActor*> TaggedObjects;
	UPROPERTY(BlueprintReadOnly) TArray<AActor*> SelectedObjects;

	UPROPERTY(BlueprintReadWrite,EditAnywhere) UObjectProperties* ObjectProperties;


};
