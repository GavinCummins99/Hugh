#include "HLE_SaveLoad.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "ObjectProperties.h"

// Sets default values for this component's properties
UHLE_SaveLoad::UHLE_SaveLoad() {
	PrimaryComponentTick.bCanEverTick = false;
}

/*Saves level to JSON string
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

          // Add this actor's JSON object to our array
          ActorArray.Add(MakeShared<FJsonValueObject>(ActorJson));
       }
    }
    
    // Create the main JSON object
    const TSharedPtr<FJsonObject> MainJsonObject = MakeShared<FJsonObject>();
    MainJsonObject->SetArrayField("Actors", ActorArray);

    // Since we need custom formatting, let's manually create the JSON string
    FString OutputString = "{\n\t\"Actors\": [\n";
    
    // For each actor
    for (int32 i = 0; i < ActorArray.Num(); i++) {
        const TSharedPtr<FJsonObject>& ActorObj = ActorArray[i]->AsObject();
        
        OutputString += "\t\t{\n";
        OutputString += "\t\t\t\"ActorClass\": \"" + ActorObj->GetStringField("ActorClass") + "\",\n";
        
        // Format Location in-line
        const TSharedPtr<FJsonObject>& LocationObj = ActorObj->GetObjectField("Location");
        OutputString += "\t\t\t\"Location\": {\"X\": " + 
            FString::FromInt((int32)LocationObj->GetNumberField("X")) + ", \"Y\": " + 
            FString::FromInt((int32)LocationObj->GetNumberField("Y")) + ", \"Z\": " + 
            FString::FromInt((int32)LocationObj->GetNumberField("Z")) + "},\n";
        
        // Format Rotation in-line
        const TSharedPtr<FJsonObject>& RotationObj = ActorObj->GetObjectField("Rotation");
        OutputString += "\t\t\t\"Rotation\": {\"Pitch\": " + 
            FString::FromInt((int32)RotationObj->GetNumberField("Pitch")) + ", \"Yaw\": " + 
            FString::FromInt((int32)RotationObj->GetNumberField("Yaw")) + ", \"Roll\": " + 
            FString::FromInt((int32)RotationObj->GetNumberField("Roll")) + "}\n";
        
        // Add closing brace (with comma if not the last item)
        if (i < ActorArray.Num() - 1) {
            OutputString += "\t\t},\n";
        } else {
            OutputString += "\t\t}\n";
        }
    }
    
    // Close the arrays and objects
    OutputString += "\t]\n}";

    // Save to file
    if (FFileHelper::SaveStringToFile(OutputString, *(SavePath + LevelName + ".json"))) {
       UE_LOG(LogTemp, Log, TEXT("Successfully saved to: %s"), *(SavePath + LevelName + ".json"));
    }
    else {
       UE_LOG(LogTemp, Error, TEXT("Failed to save file"));
    }
}
*/

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

            // Check if the actor has the "ObjectProperties" component
            UActorComponent* ObjectPropertiesComponent = Actor->GetComponentByClass(UObjectProperties::StaticClass());
            if (ObjectPropertiesComponent) {
                UObjectProperties* PropertiesComp = Cast<UObjectProperties>(ObjectPropertiesComponent);
                if (PropertiesComp) {
                    // Create a new JSON object to store component properties
                    TSharedPtr<FJsonObject> PropertiesJson = MakeShared<FJsonObject>();
                    
                    // Use reflection to get all properties of the component
                    for (TFieldIterator<FProperty> PropIt(PropertiesComp->GetClass()); PropIt; ++PropIt) {
                        FProperty* Property = *PropIt;
                        
                        // Skip properties that are from parent classes we don't care about
                        if (Property->GetOwnerClass()->GetName().StartsWith("Object") || 
                            Property->GetOwnerClass()->GetName().StartsWith("ActorComponent") ||
                            Property->GetOwnerClass()->GetName().StartsWith("SceneComponent")) {
                            continue;
                        }
                        
                        // Get property value
                        void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(PropertiesComp);
                        
                        if (FStrProperty* StrProperty = CastField<FStrProperty>(Property)) {
                            // Handle string properties
                            PropertiesJson->SetStringField(Property->GetName(), StrProperty->GetPropertyValue(PropertyValuePtr));
                        }
                        else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property)) {
                            // Handle integer properties
                            PropertiesJson->SetNumberField(Property->GetName(), IntProperty->GetPropertyValue(PropertyValuePtr));
                        }
                        else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property)) {
                            // Handle float properties
                            PropertiesJson->SetNumberField(Property->GetName(), FloatProperty->GetPropertyValue(PropertyValuePtr));
                        }
                        else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property)) {
                            // Handle boolean properties
                            PropertiesJson->SetBoolField(Property->GetName(), BoolProperty->GetPropertyValue(PropertyValuePtr));
                        }
                        else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property)) {
                            // Handle FName properties
                            PropertiesJson->SetStringField(Property->GetName(), NameProperty->GetPropertyValue(PropertyValuePtr).ToString());
                        }
                        else if (FTextProperty* TextProperty = CastField<FTextProperty>(Property)) {
                            // Handle FText properties
                            PropertiesJson->SetStringField(Property->GetName(), TextProperty->GetPropertyValue(PropertyValuePtr).ToString());
                        }
                        // Add more property types as needed
                    }
                    
                    // Add the properties object to the actor JSON
                    ActorJson->SetObjectField("Properties", PropertiesJson);
                }
            }

            // Add this actor's JSON object to our array
            ActorArray.Add(MakeShared<FJsonValueObject>(ActorJson));
        }
    }
    
    // Create the main JSON object
    const TSharedPtr<FJsonObject> MainJsonObject = MakeShared<FJsonObject>();
    MainJsonObject->SetArrayField("Actors", ActorArray);

    // Since we need custom formatting, let's manually create the JSON string
    FString OutputString = "{\n\t\"Actors\": [\n";
    
    // For each actor
    for (int32 i = 0; i < ActorArray.Num(); i++) {
        const TSharedPtr<FJsonObject>& ActorObj = ActorArray[i]->AsObject();
        
        OutputString += "\t\t{\n";
        OutputString += "\t\t\t\"ActorClass\": \"" + ActorObj->GetStringField("ActorClass") + "\",\n";
        
        // Format Location in-line
        const TSharedPtr<FJsonObject>& LocationObj = ActorObj->GetObjectField("Location");
        OutputString += "\t\t\t\"Location\": {\"X\": " + 
            FString::FromInt((int32)LocationObj->GetNumberField("X")) + ", \"Y\": " + 
            FString::FromInt((int32)LocationObj->GetNumberField("Y")) + ", \"Z\": " + 
            FString::FromInt((int32)LocationObj->GetNumberField("Z")) + "},\n";
        
        // Format Rotation in-line
        const TSharedPtr<FJsonObject>& RotationObj = ActorObj->GetObjectField("Rotation");
        OutputString += "\t\t\t\"Rotation\": {\"Pitch\": " + 
            FString::FromInt((int32)RotationObj->GetNumberField("Pitch")) + ", \"Yaw\": " + 
            FString::FromInt((int32)RotationObj->GetNumberField("Yaw")) + ", \"Roll\": " + 
            FString::FromInt((int32)RotationObj->GetNumberField("Roll")) + "}";
        
        // Add Properties if they exist
        if (ActorObj->HasField("Properties")) {
            OutputString += ",\n\t\t\t\"Properties\": {\n";
            
            // Get all properties
            const TSharedPtr<FJsonObject>& PropertiesObj = ActorObj->GetObjectField("Properties");
            TArray<FString> PropertyNames;
            PropertiesObj->Values.GetKeys(PropertyNames);
            
            // Add each property
            for (int32 j = 0; j < PropertyNames.Num(); j++) {
                const FString& PropName = PropertyNames[j];
                const TSharedPtr<FJsonValue>& PropValue = PropertiesObj->Values[PropName];
                
                OutputString += "\t\t\t\t\"" + PropName + "\": ";
                
                // Format based on value type
                if (PropValue->Type == EJson::String) {
                    OutputString += "\"" + PropValue->AsString() + "\"";
                } 
                else if (PropValue->Type == EJson::Number) {
                    OutputString += FString::SanitizeFloat(PropValue->AsNumber());
                }
                else if (PropValue->Type == EJson::Boolean) {
                    OutputString += PropValue->AsBool() ? "true" : "false";
                }
                
                // Add comma if not the last property
                if (j < PropertyNames.Num() - 1) {
                    OutputString += ",\n";
                } else {
                    OutputString += "\n";
                }
            }
            
            OutputString += "\t\t\t}";
        }
        
        // Add closing brace (with comma if not the last item)
        if (i < ActorArray.Num() - 1) {
            OutputString += "\n\t\t},\n";
        } else {
            OutputString += "\n\t\t}\n";
        }
    }
    
    // Close the arrays and objects
    OutputString += "\t]\n}";

    // Save to file
    if (FFileHelper::SaveStringToFile(OutputString, *(SavePath + LevelName + ".json"))) {
       UE_LOG(LogTemp, Log, TEXT("Successfully saved to: %s"), *(SavePath + LevelName + ".json"));
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
    
    // Build the full path and check if the file exists
    FString FullPath = LoadPath + LevelName + ".json";
    if (!FFileHelper::LoadFileToString(JsonString, *FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load JSON file from: %s"), *FullPath);
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
        FVector Location = FVector::ZeroVector;
        FRotator Rotation = FRotator::ZeroRotator;
        
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

        // Spawn actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
        if (NewActor)
        {
            // We're no longer storing Scale in our JSON, so we use default scale (1,1,1)
            NewActor->Tags.Add(FName("LevelEditorObject"));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Successfully loaded level from: %s"), *FullPath);
}


