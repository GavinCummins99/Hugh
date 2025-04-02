#include "HLE_SaveLoad.h"
#include "EngineUtils.h"
#include "HttpModule.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "ObjectProperties.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IHttpResponse.h"

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

            // Check if the actor has the "ObjectProperties" component
            UActorComponent* ObjectPropertiesComponent = Actor->GetComponentByClass(UObjectProperties::StaticClass());
            if (ObjectPropertiesComponent) {
                UObjectProperties* PropertiesComp = Cast<UObjectProperties>(ObjectPropertiesComponent);
                if (PropertiesComp) {
                    // Get the ObjectColor property
                    FColor ObjectColor = PropertiesComp->ObjectColor;
                    
                    // Convert to hex format #RRGGBBAA
                    FString HexColor = FString::Printf(TEXT("#%02X%02X%02X%02X"), 
                        ObjectColor.R, ObjectColor.G, ObjectColor.B, ObjectColor.A);
                    
                    // Create a new JSON object to store component properties
                    TSharedPtr<FJsonObject> PropertiesJson = MakeShared<FJsonObject>();
                    PropertiesJson->SetStringField("ObjectColor", HexColor);
                    
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
        
        // Add ObjectColor if it exists
        if (ActorObj->HasField("Properties")) {
            const TSharedPtr<FJsonObject>& PropertiesObj = ActorObj->GetObjectField("Properties");
            if (PropertiesObj->HasField("ObjectColor")) {
                OutputString += ",\n\t\t\t\"Properties\": {\n";
                OutputString += "\t\t\t\t\"ObjectColor\": \"" + PropertiesObj->GetStringField("ObjectColor") + "\"\n";
                OutputString += "\t\t\t}";
            }
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
*/

//Saves level to Firebase Storage
void UHLE_SaveLoad::SaveLevel(FString LevelName) const {
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
                    // Get the ObjectColor property
                    FColor ObjectColor = PropertiesComp->ObjectColor;
                    
                    // Convert to hex format #RRGGBBAA
                    FString HexColor = FString::Printf(TEXT("#%02X%02X%02X%02X"), 
                        ObjectColor.R, ObjectColor.G, ObjectColor.B, ObjectColor.A);
                    
                    // Create a new JSON object to store component properties
                    TSharedPtr<FJsonObject> PropertiesJson = MakeShared<FJsonObject>();
                    PropertiesJson->SetStringField("ObjectColor", HexColor);
                     
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
            const TSharedPtr<FJsonObject>& PropertiesObj = ActorObj->GetObjectField("Properties");
    
            OutputString += ",\n\t\t\t\"Properties\": {\n";
    
            // Add ObjectColor if it exists
            if (PropertiesObj->HasField("ObjectColor")) {
                OutputString += "\t\t\t\t\"ObjectColor\": \"" + PropertiesObj->GetStringField("ObjectColor") + "\"";
        
                // Add comma if Pushable also exists
                if (PropertiesObj->HasField("Pushable")) {
                    OutputString += ",\n";
                } else {
                    OutputString += "\n";
                }
            }
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

    // Firebase Storage configuration
    FString StorageBucket = "hugh-c1e9e.firebasestorage.app"; // From your Firebase config
    FString FileName = LevelName + ".json";
    
    // URL encode the filename (replace spaces with %20, etc.)
    FString EncodedFileName = FGenericPlatformHttp::UrlEncode(FileName);
    
    // Firebase Storage upload URL
    FString UploadURL = FString::Printf(TEXT("https://firebasestorage.googleapis.com/v0/b/%s/o?name=%s&uploadType=media"), 
                                       *StorageBucket, *EncodedFileName);
    
    // Create HTTP request using Unreal's HTTP module
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(UploadURL);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(OutputString);
    
    // Set up callback for when request completes
    Request->OnProcessRequestComplete().BindLambda(
        [FileName](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
            if (bWasSuccessful && Response.IsValid()) {
                // Check response code (200 or 201 typically means success)
                if (Response->GetResponseCode() == 200 || Response->GetResponseCode() == 201) {
                    UE_LOG(LogTemp, Log, TEXT("Successfully uploaded level %s to Firebase"), *FileName);
                    
                    // Parse the response to get download URL if needed
                    TSharedPtr<FJsonObject> JsonObject;
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
                    if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
                        // Firebase returns a "name" field with the path to the file
                        if (JsonObject->HasField("downloadTokens")) {
                            FString DownloadToken = JsonObject->GetStringField("downloadTokens");
                            FString Name = JsonObject->GetStringField("name");
                            FString StorageBucket = "hugh-c1e9e.firebasestorage.app";
                            
                            // Construct download URL
                            FString DownloadURL = FString::Printf(TEXT("https://firebasestorage.googleapis.com/v0/b/%s/o/%s?alt=media&token=%s"), 
                                                                *StorageBucket, *FGenericPlatformHttp::UrlEncode(Name), *DownloadToken);
                            
                            UE_LOG(LogTemp, Log, TEXT("Level download URL: %s"), *DownloadURL);
                        }
                    }
                }
                else {
                    UE_LOG(LogTemp, Error, TEXT("Failed to upload level %s. Response Code: %d"), 
                          *FileName, Response->GetResponseCode());
                }
            }
            else {
                UE_LOG(LogTemp, Error, TEXT("Failed to upload level %s. No response received."), *FileName);
            }
        }
    );
    
    // Send the request
    Request->ProcessRequest();
    
    UE_LOG(LogTemp, Log, TEXT("Uploading level to Firebase: %s"), *FileName);
}

//Loads a level file from json string
/*void UHLE_SaveLoad::LoadLevel(FString LevelName, FString LoadPath) {
    
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
            // Add the LevelEditorObject tag
            NewActor->Tags.Add(FName("LevelEditorObject"));
            
            // Check if we have Properties to restore
            const TSharedPtr<FJsonObject>* PropertiesObj;
            if (ActorObject->TryGetObjectField("Properties", PropertiesObj))
            {
                // Try to find the ObjectProperties component on the actor
                UObjectProperties* PropertiesComp = Cast<UObjectProperties>(NewActor->GetComponentByClass(UObjectProperties::StaticClass()));
                if (PropertiesComp)
                {
                    // Check if we have an ObjectColor property
                    FString HexColorString;
                    if ((*PropertiesObj)->TryGetStringField("ObjectColor", HexColorString))
                    {
                        // Parse the hex color string
                        if (HexColorString.StartsWith("#") && HexColorString.Len() == 9)
                        {
                            // Remove the # character
                            FString ColorHex = HexColorString.Mid(1);
                            
                            // Convert the hex string directly to a color
                            PropertiesComp->ObjectColor = FColor::FromHex(ColorHex);
                        }
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Successfully loaded level from: %s"), *FullPath);
}
*/

//Loads a level file from Firebase Storage
void UHLE_SaveLoad::LoadLevel(FString LevelName) {
    
    //Unload previous level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("LevelEditorObject"), FoundActors);

    for (auto Element : FoundActors){
       Element->Destroy();    
    }
    
    // Firebase Storage configuration
    FString StorageBucket = TEXT("hugh-c1e9e.firebasestorage.app");
    FString FileName = LevelName + TEXT(".json");
    
    // URL encode the filename
    FString EncodedFileName = FGenericPlatformHttp::UrlEncode(FileName);
    
    // Firebase Storage download URL
    FString DownloadURL = FString::Printf(TEXT("https://firebasestorage.googleapis.com/v0/b/%s/o/%s?alt=media"), 
                                        *StorageBucket, *EncodedFileName);
    
    // Create HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(DownloadURL);
    Request->SetVerb(TEXT("GET"));
    
    // Store reference to World and level name for use in lambda
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Set up callback for when request completes
    Request->OnProcessRequestComplete().BindLambda(
        [this, World, LevelName](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to download level %s from Firebase. Response code: %d"), 
                      *LevelName, Response.IsValid() ? Response->GetResponseCode() : 0);
                return;
            }
            
            // Get the JSON string from the response
            FString JsonString = Response->GetContentAsString();
            
            TSharedPtr<FJsonObject> MainJsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
            
            if (!FJsonSerializer::Deserialize(Reader, MainJsonObject) || !MainJsonObject.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON for level %s"), *LevelName);
                return;
            }

            const TArray<TSharedPtr<FJsonValue>>* ActorsArrayPtr = nullptr;
            if (!MainJsonObject->TryGetArrayField(FString(TEXT("Actors")), ActorsArrayPtr) || !ActorsArrayPtr)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to find Actors array in JSON for level %s"), *LevelName);
                return;
            }

            for (const TSharedPtr<FJsonValue>& ActorValue : *ActorsArrayPtr)
            {
                TSharedPtr<FJsonObject> ActorObject = ActorValue->AsObject();
                if (!ActorObject.IsValid()) continue;

                FString ActorClassPath;
                if (!ActorObject->TryGetStringField(TEXT("ActorClass"), ActorClassPath)) continue;

                UClass* ActorClass = FindObject<UClass>(ANY_PACKAGE, *ActorClassPath);
                if (!ActorClass) continue;

                // Get transform data
                FVector Location = FVector::ZeroVector;
                FRotator Rotation = FRotator::ZeroRotator;
                
                const TSharedPtr<FJsonObject>* LocationObj = nullptr;
                if (ActorObject->TryGetObjectField(TEXT("Location"), LocationObj) && LocationObj)
                {
                    Location = FVector(
                        (*LocationObj)->GetNumberField(TEXT("X")),
                        (*LocationObj)->GetNumberField(TEXT("Y")),
                        (*LocationObj)->GetNumberField(TEXT("Z"))
                    );
                }

                const TSharedPtr<FJsonObject>* RotationObj = nullptr;
                if (ActorObject->TryGetObjectField(TEXT("Rotation"), RotationObj) && RotationObj)
                {
                    Rotation = FRotator(
                        (*RotationObj)->GetNumberField(TEXT("Pitch")),
                        (*RotationObj)->GetNumberField(TEXT("Yaw")),
                        (*RotationObj)->GetNumberField(TEXT("Roll"))
                    );
                }

                // Spawn actor
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                
                AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
                if (NewActor)
                {

                    
                    // Add the LevelEditorObject tag
                    NewActor->Tags.Add(FName("LevelEditorObject"));
                    
                    // Check if we have Properties to restore
                    const TSharedPtr<FJsonObject>* PropertiesObj = nullptr;
                    if (ActorObject->TryGetObjectField(TEXT("Properties"), PropertiesObj) && PropertiesObj)
                    {
                        // Try to find the ObjectProperties component on the actor
                        UObjectProperties* PropertiesComp = Cast<UObjectProperties>(NewActor->GetComponentByClass(UObjectProperties::StaticClass()));
                        if (PropertiesComp)
                        {
                            PropertiesComp->OnPlaced();
                            
                            // Check if we have an ObjectColor property
                            FString HexColorString;
                            if ((*PropertiesObj)->TryGetStringField(TEXT("ObjectColor"), HexColorString))
                            {
                                // Parse the hex color string
                                if (HexColorString.StartsWith(TEXT("#")) && HexColorString.Len() == 9)
                                {
                                    // Remove the # character
                                    FString ColorHex = HexColorString.Mid(1);
                                    
                                    // Convert the hex string directly to a color
                                    PropertiesComp->ObjectColor = FColor::FromHex(ColorHex);
                                }
                            }
                        }
                    }
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Successfully loaded level %s from Firebase"), *LevelName);
        }
    );
    
    // Send the request
    Request->ProcessRequest();
    
    UE_LOG(LogTemp, Log, TEXT("Downloading level %s from Firebase..."), *LevelName);
}

void UHLE_SaveLoad::GetLevelNames(const FLevelNamesCallback& Callback)
{
    // Firebase Storage configuration
    FString StorageBucket = TEXT("hugh-c1e9e.firebasestorage.app");
    FString ListURL = FString::Printf(TEXT("https://firebasestorage.googleapis.com/v0/b/%s/o"), *StorageBucket);
    
    // Create HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ListURL);
    Request->SetVerb(TEXT("GET"));
    
    // Set up callback
    Request->OnProcessRequestComplete().BindLambda(
        [Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            TArray<FString> LevelNames;
            
            if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
            {
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
                
                if (FJsonSerializer::Deserialize(Reader, JsonObject))
                {
                    // Fix for the TryGetArrayField issue
                    const TArray<TSharedPtr<FJsonValue>>* ItemsPtr = nullptr;
                    if (JsonObject->TryGetArrayField(FString(TEXT("items")), ItemsPtr) && ItemsPtr != nullptr)
                    {
                        for (const TSharedPtr<FJsonValue>& Item : *ItemsPtr)
                        {
                            TSharedPtr<FJsonObject> ItemObj = Item->AsObject();
                            if (ItemObj.IsValid())
                            {
                                FString Name;
                                if (ItemObj->TryGetStringField(TEXT("name"), Name) && Name.EndsWith(TEXT(".json")))
                                {
                                    LevelNames.Add(Name.Left(Name.Len() - 5));
                                }
                            }
                        }
                    }
                }
            }
            
            // Execute the callback with the level names
            Callback.ExecuteIfBound(LevelNames);
        }
    );
    
    // Send the request
    Request->ProcessRequest();
}