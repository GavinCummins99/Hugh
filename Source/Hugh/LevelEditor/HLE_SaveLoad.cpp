#include "HLE_SaveLoad.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"

// Sets default values for this component's properties
UHLE_SaveLoad::UHLE_SaveLoad() {
	PrimaryComponentTick.bCanEverTick = false;
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
	if (FFileHelper::SaveStringToFile(OutputString, *(SavePath + LevelName + ".json"))) {
		UE_LOG(LogTemp, Log, TEXT("Successfully saved to: %s"), *SavePath);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to save file"));
	}
}

//Loads a level file from json string
void UHLE_SaveLoad::LoadLevel(FString LevelName, FString LoadPath) {
	
	//Unload previous level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("LevelEditorObject"), FoundActors);

	for (auto Element : FoundActors){
		Element->Destroy();    
	}
	
    FString JsonString;
    //FString LoadPath = FPaths::ProjectSavedDir() + "LevelSaves/" + LevelName + ".json";
    
    if (!FFileHelper::LoadFileToString(JsonString, *LoadPath.Append(LevelName + ".json")))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load JSON file"));
        return;
    }

    TSharedPtr<FJsonObject> MainJsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(Reader, MainJsonObject) || !MainJsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* ActorsArrayPtr;
    if (!MainJsonObject->TryGetArrayField("Actors", ActorsArrayPtr))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find Actors array in JSON"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    for (const TSharedPtr<FJsonValue>& ActorValue : *ActorsArrayPtr)
    {
        TSharedPtr<FJsonObject> ActorObject = ActorValue->AsObject();
        if (!ActorObject.IsValid()) continue;

        FString ActorClassPath;
        if (!ActorObject->TryGetStringField("ActorClass", ActorClassPath)) continue;

        UClass* ActorClass = FindObject<UClass>(ANY_PACKAGE, *ActorClassPath);
        if (!ActorClass) continue;

        // Get transform data
        FVector Location, Scale;
        FRotator Rotation;
        
        const TSharedPtr<FJsonObject>* LocationObj;
        if (ActorObject->TryGetObjectField("Location", LocationObj))
        {
            Location = FVector(
                (*LocationObj)->GetNumberField("X"),
                (*LocationObj)->GetNumberField("Y"),
                (*LocationObj)->GetNumberField("Z")
            );
        }

        const TSharedPtr<FJsonObject>* RotationObj;
        if (ActorObject->TryGetObjectField("Rotation", RotationObj))
        {
            Rotation = FRotator(
                (*RotationObj)->GetNumberField("Pitch"),
                (*RotationObj)->GetNumberField("Yaw"),
                (*RotationObj)->GetNumberField("Roll")
            );
        }

        const TSharedPtr<FJsonObject>* ScaleObj;
        if (ActorObject->TryGetObjectField("Scale", ScaleObj))
        {
            Scale = FVector(
                (*ScaleObj)->GetNumberField("X"),
                (*ScaleObj)->GetNumberField("Y"),
                (*ScaleObj)->GetNumberField("Z")
            );
        }

        // Spawn actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
        if (NewActor)
        {
            NewActor->SetActorScale3D(Scale);
            NewActor->Tags.Add("LevelEditorObject");
        }
    }
}


