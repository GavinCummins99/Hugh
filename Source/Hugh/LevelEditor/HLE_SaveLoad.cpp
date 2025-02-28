// Fill out your copyright notice in the Description page of Project Settings.


#include "HLE_SaveLoad.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"

// Sets default values for this component's properties
UHLE_SaveLoad::UHLE_SaveLoad()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHLE_SaveLoad::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

//Saves level to JSON string
void UHLE_SaveLoad::SaveLevel(FString LevelName, FString SavePath) const {
	const UWorld* World = GetWorld();
	if (!World) return;

	// Create a JSON array to store all actors
	TArray<TSharedPtr<FJsonValue>> ActorArray;

	// Find all actors with tag
	for (TActorIterator<AActor> It(World); It; ++It) {
		const AActor* Actor = *It;
		if (Actor && Actor->ActorHasTag("LevelEditorObject")) {
			// Create a JSON object for this actor
			TSharedPtr<FJsonObject> ActorJson = MakeShared<FJsonObject>();

			// Store the class name
			ActorJson->SetStringField("ActorClass", Actor->GetClass()->GetPathName());

			// Store location
			const FVector Location = Actor->GetActorLocation();
			TSharedPtr<FJsonObject> LocationJson = MakeShared<FJsonObject>();
			LocationJson->SetNumberField("X", Location.X);
			LocationJson->SetNumberField("Y", Location.Y);
			LocationJson->SetNumberField("Z", Location.Z);
			ActorJson->SetObjectField("Location", LocationJson);

			// Store rotation
			const FRotator Rotation = Actor->GetActorRotation();
			TSharedPtr<FJsonObject> RotationJson = MakeShared<FJsonObject>();
			RotationJson->SetNumberField("Pitch", Rotation.Pitch);
			RotationJson->SetNumberField("Yaw", Rotation.Yaw);
			RotationJson->SetNumberField("Roll", Rotation.Roll);
			ActorJson->SetObjectField("Rotation", RotationJson);

			// Store scale
			const FVector Scale = Actor->GetActorScale3D();
			TSharedPtr<FJsonObject> ScaleJson = MakeShared<FJsonObject>();
			ScaleJson->SetNumberField("X", Scale.X);
			ScaleJson->SetNumberField("Y", Scale.Y);
			ScaleJson->SetNumberField("Z", Scale.Z);
			ActorJson->SetObjectField("Scale", ScaleJson);

			// Add this actor's JSON object to our array
			ActorArray.Add(MakeShared<FJsonValueObject>(ActorJson));
		}
	}
	
	// Create the main JSON object
	const TSharedPtr<FJsonObject> MainJsonObject = MakeShared<FJsonObject>();
	MainJsonObject->SetArrayField("Actors", ActorArray);

	// Convert to string
	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(MainJsonObject.ToSharedRef(), Writer);

	// Save to file
	//FString SavePath = FPaths::ProjectSavedDir() + "LevelSaves/" + LevelName + ".json";
	if (FFileHelper::SaveStringToFile(OutputString, *(SavePath + ".json"))) {
		UE_LOG(LogTemp, Log, TEXT("Successfully saved to: %s"), *SavePath);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to save file"));
	}
}

