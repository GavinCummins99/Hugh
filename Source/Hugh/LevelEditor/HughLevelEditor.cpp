#include "HughLevelEditor.h"
//#include "EnhancedInputComponent.h"

#include "ActorFolder.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Class.h"
#include "GameFramework/Actor.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"
#include "AsyncTreeDifferences.h"
#include "EditorActorFolders.h"
#include "EngineUtils.h"
#include "JsonObjectConverter.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "../LevelEditorGameMode.h"
#include "ObjectProperties.h"
#include "AssetRegistry/AssetData.h"
#include "Components/StaticMeshComponent.h"
#include "Editor/BehaviorTreeEditor/Public/BehaviorTreeColors.h"
#include "Editor/BehaviorTreeEditor/Public/BehaviorTreeColors.h"
#include "Engine/Blueprint.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/Material.h"
#include "HLE_Movement.h"
#include "Runtime/Media/Public/IMediaControls.h"

// Sets default values
AHughLevelEditor::AHughLevelEditor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Crate display mesh
	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>("Display mesh");
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Create camera boom arm
	CameraArm = CreateDefaultSubobject<USpringArmComponent>("Camera arm");
	CameraArm->SetRelativeRotation(FRotator(-45,0,0));
	SetRootComponent(CameraArm);

	//Create camera and attach to boom arm
	EditorCamera = CreateDefaultSubobject<UCameraComponent>("Camera");
	EditorCamera->SetupAttachment(CameraArm);
	
	//This can probably be replaced by an "NewObject" 
	ObjectProperties = CreateDefaultSubobject<UObjectProperties>("Object Properties");
}

// Called when the game starts or when spawned
void AHughLevelEditor::BeginPlay()
{
	Super::BeginPlay();

	// Create components
	SaveLoad = NewObject<UHLE_SaveLoad>(this);
	HLE_CameraComponent = NewObject<UHLE_Movement>(this);
    
	// Register the movement component and add it to actor
	HLE_CameraComponent->RegisterComponent();
	this->AddInstanceComponent(HLE_CameraComponent);
    
	// Set up any references the component needs
	if (CameraArm) {
		HLE_CameraComponent->CameraArm = CameraArm;
	}
    
	// Bind input for the component now that it exists
	if (InputComponent)
	{
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
		//Setup inputs for movement component
		if(HLE_CameraComponent) {
			if (IA_Zoom)
			{
				EnhancedInputComponent->BindAction(IA_Zoom, ETriggerEvent::Triggered, HLE_CameraComponent, &UHLE_Movement::Zoom);
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Zoom bound in BeginPlay"));
			}

			if (IA_Look) {
				EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, HLE_CameraComponent, &UHLE_Movement::OribitCamera);
			}

			if (IA_LookButton) {
				EnhancedInputComponent->BindAction(IA_LookButton, ETriggerEvent::Started, HLE_CameraComponent, &UHLE_Movement::CanLook);
				EnhancedInputComponent->BindAction(IA_LookButton, ETriggerEvent::Completed, HLE_CameraComponent, &UHLE_Movement::CanLook);
			}

			if (IA_PanButton) {
				EnhancedInputComponent->BindAction(IA_PanButton, ETriggerEvent::Started, HLE_CameraComponent, &UHLE_Movement::Pan);
				EnhancedInputComponent->BindAction(IA_PanButton, ETriggerEvent::Completed, HLE_CameraComponent, &UHLE_Movement::Pan);
			}
		}
	}

	GetActorsFromFolder("/Game/Objects");
	GM = Cast<ALevelEditorGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
		return;
	
	// In BeginPlay(), after your existing code:
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	
	PlayerController->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);

}

void AHughLevelEditor::SetInputMap() {
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("TEST FUNCTION CALLED"));

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


// Called every frame
void AHughLevelEditor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Trace();
	//GEngine->AddOnScreenDebugMessage(5, 5, FColor::Blue, "---- Pan : " + PanButtonPressed);

	//APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	//if (!PlayerController)
	//	return;
//
	//PlayerController->GetInputMouseDelta(NewMouseLoc.X, NewMouseLoc.Y);
	//RotateCamera(FVector2D(NewMouseLoc.X - SavedMouseLoc.X, NewMouseLoc.X - SavedMouseLoc.X));
//
	//PlayerController->GetInputMouseDelta(SavedMouseLoc.X, SavedMouseLoc.Y);
	//
	//
	int XLen = DisplayMesh->GetComponentLocation().X / 100;
	int YLen = DisplayMesh->GetComponentLocation().Y / 100;

	GEngine->AddOnScreenDebugMessage(0 ,0, FColor::Blue, FString::FromInt(XLen));
	GEngine->AddOnScreenDebugMessage(1 ,0, FColor::Blue, FString::FromInt(YLen));


	if (EditorMode == Modes::Building) {
		if (!Placing)StartPlacing(false);
		PlaceObject(true);
	}

	//if(GM->HideWalls) CamCollision();
}

// Called to bind functionality to input
void AHughLevelEditor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (Input)
	{
		if (IA_Place) {
			Input->BindAction(IA_Place, ETriggerEvent::Started, this, &AHughLevelEditor::StartPlacing);
			Input->BindAction(IA_Place, ETriggerEvent::Triggered, this, &AHughLevelEditor::PlaceObject);
			Input->BindAction(IA_Place, ETriggerEvent::Completed, this, &AHughLevelEditor::PlaceObject);
		}
		if (IA_RotateObject) Input->BindAction(IA_RotateObject, ETriggerEvent::Started, this, &AHughLevelEditor::RotateObject);
	}

}


//Helper function for snapping vector to grid
FVector AHughLevelEditor::Snap(FVector InVector) {
	return FVector(
	FMath::RoundToFloat(InVector.X / GridSize) * GridSize,
	FMath::RoundToFloat(InVector.Y / GridSize) * GridSize,
	FMath::RoundToFloat(InVector.Z / GridSize) * GridSize
	);
}

//Rotates object
void AHughLevelEditor::RotateObject() {
	DisplayMesh->AddWorldRotation(FRotator(0,45,0));
}

//Handel's main line trace from the camera to world space
void AHughLevelEditor::Trace() {
	// Get the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
		return;

	// Get the viewport size
	FVector2D ViewportSize;
	GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);

	// Get the mouse position
	float MouseX, MouseY;
	PlayerController->GetMousePosition(MouseX, MouseY);

	// Deproject the screen position of the mouse to world space
	FVector WorldLocation, WorldDirection;
	PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);
	
	// Perform a line trace
	FHitResult Hit;
	const FVector Start = WorldLocation;
	const FVector End = WorldLocation + WorldDirection * 10000.0f; 
	FCollisionQueryParams QueryParams; QueryParams.AddIgnoredActor(GetOwner());

	//Setup plane
	FVector PlaneOrigin = FVector(0,0,0);
	FVector PlaneNormal = FVector(0,0,1);

	//Out values for intersetion
	FVector IntersectionPoint;
	float T;

	//For for immediate line trace hit
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams)) {
		DisplayMesh->SetWorldLocation(Snap(Hit.ImpactPoint + (Hit.ImpactNormal * 25)));

		if(EditorMode != Modes::Building) {
			if(HoveredObject && !TaggedObjects.Contains(HoveredObject) && !SelectedObjects.Contains(HoveredObject) && EditorMode == Modes::Removing) HoveredObject->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);////////////////////////////////
			HoveredObject = Hit.GetActor();
			if(EditorMode == Modes::Removing) Hit.GetActor()->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(EditorMode == Modes::Removing? RemovingMaterial : EditingMaterial);//////////////////
		}
	}
	//If no immediate hit check for point along plane 
	else if (UKismetMathLibrary::LinePlaneIntersection(Start, End, FPlane(PlaneOrigin, PlaneNormal), T, IntersectionPoint)) {
		DisplayMesh->SetWorldLocation(Snap(IntersectionPoint + (Hit.ImpactNormal * 25)));
		if(HoveredObject && !SelectedObjects.Contains(HoveredObject)) HoveredObject->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);
	}
}

void AHughLevelEditor::CanLook(const FInputActionValue& Value) {
	b_CanLook = Value.Get<bool>();
}

void AHughLevelEditor::Pan(const FInputActionValue& Value) {
	PanButtonPressed = Value.Get<bool>();
}



//Hide actors in front of the camera
void AHughLevelEditor::CamCollision() {
	ShowWalls();
	
	//Vars for trace
	TArray<FHitResult> HitResult;
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + (GetActorForwardVector() * 500.0f);
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	//
	if (UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetActorLocation() + (-EditorCamera->GetForwardVector() * 500), EditorCamera->GetComponentLocation(), 500, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true)){
		for (auto Result : HitResult){
			if(Result.GetActor()->ActorHasTag("Wall")){
				Result.GetActor()->GetComponentByClass<UStaticMeshComponent>()->SetWorldScale3D(FVector(1,1,1));
				//Result.GetActor()->SetActorHiddenInGame(true);
				ObjectsInView.Add(Result.GetActor());
			}
	
		}
		
	}
}

//Show hidden actors
void AHughLevelEditor::ShowWalls() {
	//loop through all actors and hide them 
	for (auto Element : ObjectsInView){
		Element->GetComponentByClass<UStaticMeshComponent>()->SetWorldScale3D(FVector(1,1,4));
	}
	ObjectsInView.Empty();
}

void AHughLevelEditor::StartPlacing(const FInputActionValue& Value) {
	if(EditorMode == Modes::Building) {
		Placing = Value.Get<bool>();
		StartPoint = DisplayMesh->GetComponentLocation();

		if(Value.Get<bool>()) PanDir = -1;
	}
	
	else if(EditorMode == Modes::Removing) {
		if(HoveredObject) HoveredObject->Destroy();
	}
	else if (EditorMode == Modes::Editing) {
		EditingSelection = true;
	}

	
}

//Places the object with the correct size
void AHughLevelEditor::PlaceObject(const FInputActionValue& Value) {
	
	if (EditorMode == Modes::Editing) {
		if(Value.Get<bool>() == false) {
			EditingSelection = false;
			GEngine->AddOnScreenDebugMessage(10, 10, FColor::Red, "PlaceObject" );

			for (auto Element : TaggedObjects) {
				if(SelectedObjects.Contains(Element))
				{
					SelectedObjects.Remove(Element);
					Element->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Add" );

				}
				else
				{
					SelectedObjects.Add(Element);
					Element->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(EditingMaterial);
					GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, "Add" );

				}
			}
			TaggedObjects.Empty();
		}
		
		else if (HoveredObject) {
			//HoveredObject->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(EditingMaterial);
			if (!TaggedObjects.Contains(HoveredObject)) {
				if (!SelectedObjects.Contains(HoveredObject)) HoveredObject->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(EditingMaterial);
				else
				{
					HoveredObject->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "test" );

				}
				
				TaggedObjects.Add(HoveredObject);
			}
			//return;
		}

		//for (auto Element : TaggedObjects) {
		//	if(SelectedObjects.Contains(Element)) Element->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);
		//	else Element->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(EditingMaterial);
		//}
		return;
	}
	else if (EditorMode != Modes::Building || AllObjects.IsEmpty()) return;
	
	for (auto Element : GhostObjects) {
		Element->Destroy();
	}
	GhostObjects.Empty();

	//Calculate the size of the object
	int XLen = (DisplayMesh->GetComponentLocation().X - StartPoint.X) / 100;
	int YLen = (DisplayMesh->GetComponentLocation().Y - StartPoint.Y)/ 100;
	
	int count = 0;
	//Place objects
	for (int i = 0; i < FMath::Abs(XLen) + 1; i++) {
		for (int j = 0; j < FMath::Abs(YLen) + 1; j++) {
			
			FVector Location = FVector(StartPoint.X + (i * 100 * (XLen>0?1:-1)),StartPoint.Y + (j * 100 * (YLen>0?1:-1)), StartPoint.Z);
			FRotator Rotation = ObjectProperties->AllowRotation ? DisplayMesh->GetComponentRotation() : FRotator(0,0,0);
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Template = AllObjects[ObjectIndex];
			AActor* SpawnedActor = GetWorld()->SpawnActor(AllObjects[ObjectIndex]->GetClass(), &Location, &Rotation, SpawnInfo);

			///
			if (SpawnedActor)
			{
				//UObjectProperties* SourceOP = AllObjects[ObjectIndex]->FindComponentByClass<UObjectProperties>();
				UObjectProperties* TargetOP = SpawnedActor->FindComponentByClass<UObjectProperties>();

 
				if (ObjectProperties && TargetOP)
				//{
					// Copy all properties from source to target
					UEngine::CopyPropertiesForUnrelatedObjects(ObjectProperties, TargetOP);
				//}
			}
			
			SpawnedActor->SetFolderPath("Level Editor");
			count++;

			//GhostObjects.Add(SpawnedActor);
			if (Value.Get<bool>()) {
				GhostObjects.Add(SpawnedActor);
				//SpawnedActor->GetComponentByClass<UStaticMeshComponent>()->SetMaterial(0, GhostMaterial);
				SpawnedActor->SetActorEnableCollision(ECollisionEnabled::NoCollision);

			}
			else {
				Placing = false;
				UFunction* Function = SpawnedActor->FindFunction(FName("CheckSurroundings"));
				PanDir = 1;
				SpawnedActor->Tags.Add("LevelEditorObject");

				if (Function)
				{
					SpawnedActor->ProcessEvent(Function, nullptr);
					UE_LOG(LogTemp, Warning, TEXT("ProcessEvent called for function MyCustomEvent on actor: %s"), *SpawnedActor->GetName());
				}
			}
		}
	}
	
	GEngine->AddOnScreenDebugMessage(-5, 0, FColor::Red, "Placed : " + FString::FromInt(count));
}


//Adds an array of actors to the selected folder
void AddActorsToFolder(const TArray<AActor*>& ActorsToAdd, FName FolderName) {
	for (AActor* Actor : ActorsToAdd) {
		if (Actor)
		{
			Actor->SetFolderPath(FolderName);
		}
	}
}

//Changes object
void AHughLevelEditor::SetObject(int Index) {
	ObjectIndex = Index;
	DisplayMesh->SetStaticMesh(Cast<AActor>(AllObjects[Index])->GetComponentByClass<UStaticMeshComponent>()->GetStaticMesh());
}

void AHughLevelEditor::SetMode(Modes NewMode) {
	EditorMode = NewMode;

	DisplayMesh->SetVisibility(NewMode == Modes::Building);
	for (auto Element : GhostObjects) {
		Element->Destroy();
	}
	GhostObjects.Empty();
	
	if(HoveredObject) HoveredObject->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);

}


void AHughLevelEditor::GetActorsFromFolder(const FString& InFolderPath)
{
	TArray<TSubclassOf<AActor>> ActorClasses;
	// Get the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Create a filter for the assets
	FARFilter Filter;
	Filter.PackagePaths.Add(*InFolderPath);
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.bRecursivePaths = true;

	// Get all assets in the folder
	TArray<FAssetData> AssetData;
	AssetRegistry.GetAssets(Filter, AssetData);

	// Loop through all found assets
	for (const FAssetData& Asset : AssetData)
	{
		// Load the asset
		UObject* AssetObject = Asset.GetAsset();
        
		// Check if it's a Blueprint
		if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetObject))
		{
			// Check if the Blueprint generates an Actor class
			if (Blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
			{
				ActorClasses.Add(Blueprint->GeneratedClass.Get());
			}
		}
	}
	//AllObjects = ActorClasses;

	for (TSubclassOf<AActor> ActorClass : ActorClasses) {
		GEngine->AddOnScreenDebugMessage(20, 20, FColor::Red, "Actor : ");
		
		AllObjects.Add(ActorClass.GetDefaultObject());
	}

}




UObjectProperties* AHughLevelEditor::GetObjectProperties()
{
	if (AllObjects.Num() == 0 || !AllObjects[0]) return nullptr;
    
	UClass* ActorClass = AllObjects[0]->GetClass();
	UBlueprint* Blueprint = Cast<UBlueprint>(ActorClass->ClassGeneratedBy);
    
	if (Blueprint && Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
	{
		// Print the class we're looking at
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, 
			FString::Printf(TEXT("Checking class: %s"), *Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass)->GetName()));

		// Get components from the generated class
		TArray<TObjectPtr<UActorComponent>> ComponentTemplates = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass)->ComponentTemplates;

        
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, 
			FString::Printf(TEXT("Found %d component templates"), ComponentTemplates.Num()));

		// Look through all component templates
		for (UActorComponent* Component : ComponentTemplates)
		{
			if (Component)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, 
					FString::Printf(TEXT("Component: %s"), *Component->GetClass()->GetName()));
                
				if (UObjectProperties* ObjProps = Cast<UObjectProperties>(Component))
				{
					return ObjProps;
				}
			}
		}
	}
    
	return nullptr;
}

void AHughLevelEditor::RemoveSelectedObjects() {
	for (auto Element : SelectedObjects) {
		Element->Destroy();
	}
	SelectedObjects.Empty();
}

void AHughLevelEditor::ClearSelectedObjects() {
	for (auto Element : SelectedObjects) {
		Element->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);
	}
	SelectedObjects.Empty();
}

void AHughLevelEditor::ReplaceSelectedObjects(AActor* NewObject) {
	for (auto Element : SelectedObjects) {
		//Element = NewObject;
		FVector Location = Element->GetActorLocation();
		FRotator Rotation = Element->GetActorRotation();
		FActorSpawnParameters SpawnInfo;
		AActor* SpawnedActor = GetWorld()->SpawnActor(NewObject->GetClass(), &Location, &Rotation, SpawnInfo);
	}
	RemoveSelectedObjects();
}


