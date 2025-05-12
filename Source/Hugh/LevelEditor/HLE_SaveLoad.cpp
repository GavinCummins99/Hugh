#include "HLE_SaveLoad.h"
#include "EngineUtils.h"
#include "HttpModule.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "ObjectProperties.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Json.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "PlatformHttp.h"

// Sets default values for this component's properties
UHLE_SaveLoad::UHLE_SaveLoad() {
    PrimaryComponentTick.bCanEverTick = false;
    FirebaseAuthToken = TEXT("");
    TokenExpiration = 0.0f;
}

// Called when the game starts
void UHLE_SaveLoad::BeginPlay()
{
    Super::BeginPlay();
    InitializeHTTPModule();
    
    // Attempt to authenticate with Firebase
    AuthenticateWithFirebase();
}

// Initialize HTTP Module and configure SSL
void UHLE_SaveLoad::InitializeHTTPModule()
{
    // No direct API to disable SSL verification in Unreal Engine 5.4
    // We'll rely on the n.VerifyPeer=0 setting in DefaultEngine.ini
    #if !UE_EDITOR
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Running in packaged build - using engine config for SSL settings"));
    #endif
}

// Firebase Authentication method - using API Key auth
void UHLE_SaveLoad::AuthenticateWithFirebase()
{
    // Firebase API Key from your Firebase config
    FString ApiKey = TEXT("AIzaSyA_GK_OT1hnj2AI-kzvy730PCNdBgMhNWE");
    
    // We'll use anonymous authentication for simplicity
    FString AuthURL = FString::Printf(TEXT("https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=%s"), *ApiKey);
    
    // Create the request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(AuthURL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    
    // Create the JSON payload - for anonymous auth
    TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
    JsonObj->SetBoolField(TEXT("returnSecureToken"), true);
    
    // Serialize to string
    FString JsonPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
    FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
    
    Request->SetContentAsString(JsonPayload);
    
    // Set callback for when request completes
    Request->OnProcessRequestComplete().BindUObject(this, &UHLE_SaveLoad::OnAuthenticationResponse);
    
    // Send the request
    Request->ProcessRequest();
    
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Firebase Authentication request sent..."));
}

// Handle Firebase Authentication response
void UHLE_SaveLoad::OnAuthenticationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        // Parse response JSON
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            // Extract the ID token and expiration
            if (JsonObject->HasField(TEXT("idToken")))
            {
                FirebaseAuthToken = JsonObject->GetStringField(TEXT("idToken"));
                
                // Calculate token expiration (Firebase tokens typically expire in 1 hour)
                if (JsonObject->HasField(TEXT("expiresIn")))
                {
                    float ExpiresInSeconds = FCString::Atof(*JsonObject->GetStringField(TEXT("expiresIn")));
                    TokenExpiration = FPlatformTime::Seconds() + ExpiresInSeconds;
                }
                else
                {
                    // Default expiration: 1 hour
                    TokenExpiration = FPlatformTime::Seconds() + 3600.0f;
                }
                
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Firebase Authentication successful!"));
            }
        }
    }
    else
    {
        FString ErrorMsg = FString::Printf(TEXT("Firebase Authentication failed: %d"), 
            Response.IsValid() ? Response->GetResponseCode() : 0);
        
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, ErrorMsg);
        
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
    }
}

// Check if auth token is valid and refresh if needed
bool UHLE_SaveLoad::EnsureValidAuthToken() const
{
    // If no token or token is about to expire (within 5 minutes), refresh
    if (FirebaseAuthToken.IsEmpty() || (FPlatformTime::Seconds() > (TokenExpiration - 300.0f)))
    {
        // Re-authenticate to get a new token - this is a const method, so we need a const_cast
        const_cast<UHLE_SaveLoad*>(this)->AuthenticateWithFirebase();
        
        // If still no token, return false
        if (FirebaseAuthToken.IsEmpty())
        {
            return false;
        }
    }
    
    return true;
}

// Apply auth token to HTTP request
void UHLE_SaveLoad::ApplyAuthToRequest(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request) const
{
    if (EnsureValidAuthToken())
    {
        Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *FirebaseAuthToken));
    }
}

//Saves level to Firebase Storage
void UHLE_SaveLoad::SaveLevel(FString LevelName) const {
    const UWorld* World = GetWorld();
    if (!World) return;

    GEngine->AddOnScreenDebugMessage(4234, 5, FColor::Green, "Saving level : " + LevelName);

    // Create a JSON array to store all actors
    TArray<TSharedPtr<FJsonValue>> ActorArray;

    // Find all actors with tag
    for (TActorIterator<AActor> It(World); It; ++It) {
        const AActor* Actor = *It;
        if (Actor && Actor->ActorHasTag("LevelEditorObject")) {
            // Create a JSON object for this actor
            TSharedPtr<FJsonObject> ActorJson = MakeShared<FJsonObject>();

            // Store the class name
            ActorJson->SetStringField(TEXT("ActorClass"), Actor->GetClass()->GetPathName());

            // Store location
            const FVector Location = Actor->GetActorLocation();
            TSharedPtr<FJsonObject> LocationJson = MakeShared<FJsonObject>();
            LocationJson->SetNumberField(TEXT("X"), Location.X);
            LocationJson->SetNumberField(TEXT("Y"), Location.Y);
            LocationJson->SetNumberField(TEXT("Z"), Location.Z);
            ActorJson->SetObjectField(TEXT("Location"), LocationJson);

            // Store rotation
            const FRotator Rotation = Actor->GetActorRotation();
            TSharedPtr<FJsonObject> RotationJson = MakeShared<FJsonObject>();
            RotationJson->SetNumberField(TEXT("Pitch"), Rotation.Pitch);
            RotationJson->SetNumberField(TEXT("Yaw"), Rotation.Yaw);
            RotationJson->SetNumberField(TEXT("Roll"), Rotation.Roll);
            ActorJson->SetObjectField(TEXT("Rotation"), RotationJson);

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
                    PropertiesJson->SetStringField(TEXT("ObjectColor"), HexColor);

                    // Add the properties object to the actor JSON
                    ActorJson->SetObjectField(TEXT("Properties"), PropertiesJson);
                }
            }

            // Check if this actor is a duplicate before adding
            bool bIsDuplicate = false;
            for (const TSharedPtr<FJsonValue>& ExistingActorValue : ActorArray) {
                const TSharedPtr<FJsonObject>& ExistingActor = ExistingActorValue->AsObject();
                
                // Compare class
                if (ExistingActor->GetStringField(TEXT("ActorClass")) != ActorJson->GetStringField(TEXT("ActorClass"))) {
                    continue;
                }
                
                // Compare location
                const TSharedPtr<FJsonObject>& ExistingLocation = ExistingActor->GetObjectField(TEXT("Location"));
                const TSharedPtr<FJsonObject>& NewLocation = ActorJson->GetObjectField(TEXT("Location"));
                if (ExistingLocation->GetNumberField(TEXT("X")) != NewLocation->GetNumberField(TEXT("X")) ||
                    ExistingLocation->GetNumberField(TEXT("Y")) != NewLocation->GetNumberField(TEXT("Y")) ||
                    ExistingLocation->GetNumberField(TEXT("Z")) != NewLocation->GetNumberField(TEXT("Z"))) {
                    continue;
                }
                
                // Compare rotation
                const TSharedPtr<FJsonObject>& ExistingRotation = ExistingActor->GetObjectField(TEXT("Rotation"));
                const TSharedPtr<FJsonObject>& NewRotation = ActorJson->GetObjectField(TEXT("Rotation"));
                if (ExistingRotation->GetNumberField(TEXT("Pitch")) != NewRotation->GetNumberField(TEXT("Pitch")) ||
                    ExistingRotation->GetNumberField(TEXT("Yaw")) != NewRotation->GetNumberField(TEXT("Yaw")) ||
                    ExistingRotation->GetNumberField(TEXT("Roll")) != NewRotation->GetNumberField(TEXT("Roll"))) {
                    continue;
                }
                
                // Compare properties (if they exist)
                if (ExistingActor->HasField(TEXT("Properties")) && ActorJson->HasField(TEXT("Properties"))) {
                    const TSharedPtr<FJsonObject>& ExistingProps = ExistingActor->GetObjectField(TEXT("Properties"));
                    const TSharedPtr<FJsonObject>& NewProps = ActorJson->GetObjectField(TEXT("Properties"));
                    
                    // Compare colors if they exist
                    if (ExistingProps->HasField(TEXT("ObjectColor")) && NewProps->HasField(TEXT("ObjectColor"))) {
                        if (ExistingProps->GetStringField(TEXT("ObjectColor")) != NewProps->GetStringField(TEXT("ObjectColor"))) {
                            continue;
                        }
                    }
                    // If one has properties and the other doesn't, they're not duplicates
                    else if (ExistingProps->HasField(TEXT("ObjectColor")) != NewProps->HasField(TEXT("ObjectColor"))) {
                        continue;
                    }
                }
                else if (ExistingActor->HasField(TEXT("Properties")) != ActorJson->HasField(TEXT("Properties"))) {
                    // If one has properties and the other doesn't, they're not duplicates
                    continue;
                }
                
                // If we got here, all properties match
                bIsDuplicate = true;
                break;
            }
            
            // Only add this actor if it's not a duplicate
            if (!bIsDuplicate) {
                ActorArray.Add(MakeShared<FJsonValueObject>(ActorJson));
                UE_LOG(LogTemp, Log, TEXT("Added actor to level: %s at X=%f, Y=%f, Z=%f"), 
                    *Actor->GetClass()->GetPathName(), Location.X, Location.Y, Location.Z);
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("Skipped duplicate actor: %s at X=%f, Y=%f, Z=%f"), 
                    *Actor->GetClass()->GetPathName(), Location.X, Location.Y, Location.Z);
            }
        }
    }

    // Create the main JSON object
    const TSharedPtr<FJsonObject> MainJsonObject = MakeShared<FJsonObject>();
    MainJsonObject->SetArrayField(TEXT("Actors"), ActorArray);

    // Since we need custom formatting, let's manually create the JSON string
    FString OutputString = TEXT("{\n\t\"Actors\": [\n");

    // For each actor
    for (int32 i = 0; i < ActorArray.Num(); i++) {
        const TSharedPtr<FJsonObject>& ActorObj = ActorArray[i]->AsObject();

        OutputString += TEXT("\t\t{\n");
        OutputString += TEXT("\t\t\t\"ActorClass\": \"") + ActorObj->GetStringField(TEXT("ActorClass")) + TEXT("\",\n");

        // Format Location in-line
        const TSharedPtr<FJsonObject>& LocationObj = ActorObj->GetObjectField(TEXT("Location"));
        OutputString += TEXT("\t\t\t\"Location\": {\"X\": ") +
            FString::FromInt((int32)LocationObj->GetNumberField(TEXT("X"))) + TEXT(", \"Y\": ") +
            FString::FromInt((int32)LocationObj->GetNumberField(TEXT("Y"))) + TEXT(", \"Z\": ") +
            FString::FromInt((int32)LocationObj->GetNumberField(TEXT("Z"))) + TEXT("},\n");

        // Format Rotation in-line
        const TSharedPtr<FJsonObject>& RotationObj = ActorObj->GetObjectField(TEXT("Rotation"));
        OutputString += TEXT("\t\t\t\"Rotation\": {\"Pitch\": ") +
            FString::FromInt((int32)RotationObj->GetNumberField(TEXT("Pitch"))) + TEXT(", \"Yaw\": ") +
            FString::FromInt((int32)RotationObj->GetNumberField(TEXT("Yaw"))) + TEXT(", \"Roll\": ") +
            FString::FromInt((int32)RotationObj->GetNumberField(TEXT("Roll"))) + TEXT("}");

        // Add ObjectColor if it exists
        if (ActorObj->HasField(TEXT("Properties"))) {
            const TSharedPtr<FJsonObject>& PropertiesObj = ActorObj->GetObjectField(TEXT("Properties"));
            if (PropertiesObj->HasField(TEXT("ObjectColor"))) {
                OutputString += TEXT(",\n\t\t\t\"Properties\": {\n");
                OutputString += TEXT("\t\t\t\t\"ObjectColor\": \"") + PropertiesObj->GetStringField(TEXT("ObjectColor")) + TEXT("\"\n");
                OutputString += TEXT("\t\t\t}");
            }
        }

        // Add closing brace (with comma if not the last item)
        if (i < ActorArray.Num() - 1) {
            OutputString += TEXT("\n\t\t},\n");
        }
        else {
            OutputString += TEXT("\n\t\t}\n");
        }
    }

    // Close the arrays and objects
    OutputString += TEXT("\t]\n}");

    // Firebase Storage configuration
    FString StorageBucket = TEXT("hugh-c1e9e.firebasestorage.app"); // From your Firebase config
    FString FileName = LevelName + TEXT(".json");

    // URL encode the filename (replace spaces with %20, etc.)
    FString EncodedFileName = FPlatformHttp::UrlEncode(FileName);

    // Firebase Storage upload URL
    FString UploadURL = FString::Printf(TEXT("https://firebasestorage.googleapis.com/v0/b/%s/o?name=%s&uploadType=media"),
        *StorageBucket, *EncodedFileName);

    // Create HTTP request using Unreal's HTTP module
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(UploadURL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    
    // Apply auth token to the request
    if (EnsureValidAuthToken())
    {
        Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *FirebaseAuthToken));
    }
    
    Request->SetContentAsString(OutputString);

    // Set up callback for when request completes
    Request->OnProcessRequestComplete().BindLambda(
        [FileName, StorageBucket](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
            if (bWasSuccessful && Response.IsValid()) {
                // Check response code (200 or 201 typically means success)
                if (Response->GetResponseCode() == 200 || Response->GetResponseCode() == 201) {
                    UE_LOG(LogTemp, Log, TEXT("Successfully uploaded level %s to Firebase"), *FileName);
                    
                    // Save the response to local debug
                    FString ResponseContent = Response->GetContentAsString();
                    UE_LOG(LogTemp, Log, TEXT("Firebase Response: %s"), *ResponseContent);

                    // Parse the response to get download URL if needed
                    TSharedPtr<FJsonObject> JsonObject;
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
                    if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
                        // Check for name field (Firebase Storage response structure)
                        if (JsonObject->HasField(TEXT("name"))) {
                            FString Name = JsonObject->GetStringField(TEXT("name"));
                            
                            // Construct download URL with token from metadata
                            FString DownloadURL = FString::Printf(TEXT("https://firebasestorage.googleapis.com/v0/b/%s/o/%s?alt=media"),
                                *StorageBucket, *FPlatformHttp::UrlEncode(Name));
                            
                            UE_LOG(LogTemp, Log, TEXT("Level file stored as: %s"), *Name);
                            UE_LOG(LogTemp, Log, TEXT("Download access URL: %s"), *DownloadURL);
                            
                            if (GEngine)
                                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Level saved successfully"));
                        }
                    }
                }
                else {
                    UE_LOG(LogTemp, Error, TEXT("Failed to upload level %s. Response Code: %d, Content: %s"),
                        *FileName, Response->GetResponseCode(), *Response->GetContentAsString());
                        
                    if (GEngine)
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                            FString::Printf(TEXT("Upload failed with code %d"), Response->GetResponseCode()));
                }
            }
            else {
                UE_LOG(LogTemp, Error, TEXT("Failed to upload level %s. No response received."), *FileName);
                
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Upload failed - no response"));
            }
        }
    );

    // Send the request
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("Uploading level to Firebase: %s (with %d unique actors)"), *FileName, ActorArray.Num());

    GEngine->AddOnScreenDebugMessage(4234, 5, FColor::Green, "Saving complete! : " + LevelName);
}

//Loads a level file from Firebase Storage or Local Storage
void UHLE_SaveLoad::LoadLevel(FString LevelName, bool FromLocal) {

    // Screen message - starting load
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Starting to load level: %s from %s"), 
            *LevelName, FromLocal ? TEXT("local storage") : TEXT("Firebase")));

    //Unload previous level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("LevelEditorObject"), FoundActors);

    // Screen message - removing previous actors
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Removing %d previous actors"), FoundActors.Num()));

    for (auto Element : FoundActors) {
        Element->Destroy();
    }

    // Get World reference
    UWorld* World = GetWorld();
    if (!World) {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: Invalid World reference"));
        return;
    }

    if (FromLocal) {
        // Load from local storage
        FString SaveDirectory = FPaths::ProjectDir() + TEXT("LevelSaves/");
        FString FullPath = SaveDirectory + LevelName + TEXT(".json");

        // Screen message - loading path
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Loading from local path: %s"), *FullPath));

        // Check if directory exists, if not, create it
        if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*SaveDirectory)) {
            FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SaveDirectory);
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Created LevelSaves directory"));
        }

        // Check if file exists
        if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullPath)) {
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Level file not found: %s"), *FullPath));
            return;
        }

        // Read file content
        FString JsonString;
        if (!FFileHelper::LoadFileToString(JsonString, *FullPath)) {
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Failed to read level file: %s"), *FullPath));
            return;
        }

        // Screen message - JSON size
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("JSON size: %d bytes"), JsonString.Len()));

        // Process the loaded JSON
        TSharedPtr<FJsonObject> MainJsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

        if (!FJsonSerializer::Deserialize(Reader, MainJsonObject) || !MainJsonObject.IsValid())
        {
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Failed to parse JSON for level %s"), *LevelName));
            return;
        }

        // Screen message - JSON parsed
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("JSON parsed successfully"));

        const TArray<TSharedPtr<FJsonValue>>* ActorsArrayPtr = nullptr;
        if (!MainJsonObject->TryGetArrayField(FString(TEXT("Actors")), ActorsArrayPtr) || !ActorsArrayPtr)
        {
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Failed to find Actors array in JSON for level %s"), *LevelName));
            return;
        }

        // Screen message - found actors in JSON
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Found %d actors in JSON"), ActorsArrayPtr->Num()));

        int ActorsSpawned = 0;
        int ActorsFailed = 0;

        for (const TSharedPtr<FJsonValue>& ActorValue : *ActorsArrayPtr)
        {
            TSharedPtr<FJsonObject> ActorObject = ActorValue->AsObject();
            if (!ActorObject.IsValid()) {
                ActorsFailed++;
                continue;
            }

            FString ActorClassPath;
            if (!ActorObject->TryGetStringField(TEXT("ActorClass"), ActorClassPath)) {
                ActorsFailed++;
                continue;
            }

            // Use FindClass instead of FindObject<UClass>(ANY_PACKAGE, ...)
            UClass* ActorClass = FindObject<UClass>(nullptr, *ActorClassPath);
            if (!ActorClass) {
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Could not find actor class: %s"), *ActorClassPath));
                ActorsFailed++;
                continue;
            }

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
                ActorsSpawned++;

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
                    else {
                        if (GEngine)
                            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No ObjectProperties component found on actor"));
                    }
                }
            }
            else {
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Failed to spawn actor of class: %s"), *ActorClassPath));
                ActorsFailed++;
            }
        }

        // Final screen message - loading complete
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
                FString::Printf(TEXT("Level %s loaded: %d actors spawned, %d actors failed"),
                    *LevelName, ActorsSpawned, ActorsFailed));
    }
    else {
        // Firebase Storage configuration
        FString StorageBucket = TEXT("hugh-c1e9e.firebasestorage.app");
        FString FileName = LevelName + TEXT(".json");

        // URL encode the filename
        FString EncodedFileName = FPlatformHttp::UrlEncode(FileName);

        // Firebase Storage download URL
        FString DownloadURL = FString::Printf(TEXT("https://firebasestorage.googleapis.com/v0/b/%s/o/%s?alt=media"),
            *StorageBucket, *EncodedFileName);

        // Screen message - download URL
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Download URL: %s"), *DownloadURL));

        // Create HTTP request
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
        Request->SetURL(DownloadURL);
        Request->SetVerb(TEXT("GET"));
        
        // Apply auth token to the request
        if (EnsureValidAuthToken())
        {
            Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *FirebaseAuthToken));
        }

        // Set up callback for when request completes
        Request->OnProcessRequestComplete().BindLambda(
            [this, World, LevelName](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
            {
                if (!bWasSuccessful || !Response.IsValid())
                {
                    if (GEngine)
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: HTTP request failed for level %s"), *LevelName));
                    return;
                }

                int ResponseCode = Response->GetResponseCode();
                if (ResponseCode != 200)
                {
                    if (GEngine)
                    {
                        FString ErrorMessage = FString::Printf(TEXT("ERROR: HTTP response code %d for level %s"), ResponseCode, *LevelName);
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, ErrorMessage);
                        
                        // Print response content for debugging
                        FString ResponseContent = Response->GetContentAsString();
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                            FString::Printf(TEXT("Response: %s"), *ResponseContent));
                    }
                    
                    // If unauthorized (401), try to refresh token
                    if (ResponseCode == 401 || ResponseCode == 403)
                    {
                        if (GEngine)
                            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Auth error - attempting to refresh token..."));
                        
                        // Reset token and force refresh
                        const_cast<UHLE_SaveLoad*>(this)->FirebaseAuthToken = TEXT("");
                        const_cast<UHLE_SaveLoad*>(this)->TokenExpiration = 0.0f;
                        const_cast<UHLE_SaveLoad*>(this)->AuthenticateWithFirebase();
                    }
                    
                    return;
                }

                // Screen message - response received
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("HTTP response received successfully"));

                // Get the JSON string from the response
                FString JsonString = Response->GetContentAsString();

                // Screen message - JSON size
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("JSON size: %d bytes"), JsonString.Len()));

                TSharedPtr<FJsonObject> MainJsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

                if (!FJsonSerializer::Deserialize(Reader, MainJsonObject) || !MainJsonObject.IsValid())
                {
                    if (GEngine)
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Failed to parse JSON for level %s"), *LevelName));
                    return;
                }

                // Screen message - JSON parsed
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("JSON parsed successfully"));

                const TArray<TSharedPtr<FJsonValue>>* ActorsArrayPtr = nullptr;
                if (!MainJsonObject->TryGetArrayField(FString(TEXT("Actors")), ActorsArrayPtr) || !ActorsArrayPtr)
                {
                    if (GEngine)
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Failed to find Actors array in JSON for level %s"), *LevelName));
                    return;
                }

                // Screen message - found actors in JSON
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Found %d actors in JSON"), ActorsArrayPtr->Num()));

                int ActorsSpawned = 0;
                int ActorsFailed = 0;

                for (const TSharedPtr<FJsonValue>& ActorValue : *ActorsArrayPtr)
                {
                    TSharedPtr<FJsonObject> ActorObject = ActorValue->AsObject();
                    if (!ActorObject.IsValid()) {
                        ActorsFailed++;
                        continue;
                    }

                    FString ActorClassPath;
                    if (!ActorObject->TryGetStringField(TEXT("ActorClass"), ActorClassPath)) {
                        ActorsFailed++;
                        continue;
                    }

                    // Use FindClass instead of FindObject<UClass>(ANY_PACKAGE, ...)
                    UClass* ActorClass = FindObject<UClass>(nullptr, *ActorClassPath);
                    if (!ActorClass) {
                        if (GEngine)
                            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Could not find actor class: %s"), *ActorClassPath));
                        ActorsFailed++;
                        continue;
                    }

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
                        ActorsSpawned++;

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
                            else {
                                if (GEngine)
                                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No ObjectProperties component found on actor"));
                            }
                        }
                    }
                    else {
                        if (GEngine)
                            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("ERROR: Failed to spawn actor of class: %s"), *ActorClassPath));
                        ActorsFailed++;
                    }
                }

                // Final screen message - loading complete
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
                        FString::Printf(TEXT("Level %s loaded: %d actors spawned, %d actors failed"),
                            *LevelName, ActorsSpawned, ActorsFailed));
            }
        );

        // Send the request
        Request->ProcessRequest();

        // Screen message - request sent
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("HTTP request sent for level %s"), *LevelName));
    }
}

void UHLE_SaveLoad::GetLevelNames(const FLevelNamesCallback& Callback)
{
    // Log start of function
    UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Starting to retrieve level names"));
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Starting level names request..."));
    
    // Firebase Storage configuration
    FString StorageBucket = TEXT("hugh-c1e9e.firebasestorage.app");
    FString ListURL = FString::Printf(TEXT("https://firebasestorage.googleapis.com/v0/b/%s/o"), *StorageBucket);
    
    UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Using URL: %s"), *ListURL);
    
    // Log auth token status
    if (FirebaseAuthToken.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("GetLevelNames: No auth token available, will attempt to authenticate"));
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No auth token - authenticating..."));
    }
    else 
    {
        float TimeUntilExpiration = TokenExpiration - FPlatformTime::Seconds();
        UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Auth token available, expires in %.2f seconds"), TimeUntilExpiration);
    }

    // Create HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ListURL);
    Request->SetVerb(TEXT("GET"));
    
    // Apply auth token to the request
    bool bTokenApplied = false;
    if (EnsureValidAuthToken())
    {
        Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *FirebaseAuthToken));
        bTokenApplied = true;
        
        UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Auth token applied successfully"));
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Auth token applied to request"));
    }
    else 
    {
        UE_LOG(LogTemp, Error, TEXT("GetLevelNames: Failed to apply auth token"));
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to get valid auth token"));
    }

    // Set up callback
    Request->OnProcessRequestComplete().BindLambda(
        [Callback, this, ListURL, bTokenApplied](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            TArray<FString> LevelNames;
            
            // Log request completion status
            UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Request completed - Success: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));
            
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
                    FString::Printf(TEXT("Level names request completed - Success: %s"), 
                        bWasSuccessful ? TEXT("True") : TEXT("False")));

            if (!bWasSuccessful)
            {
                UE_LOG(LogTemp, Error, TEXT("GetLevelNames: HTTP request failed"));
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HTTP Request Failed"));
                
                // Execute callback with empty array
                Callback.ExecuteIfBound(LevelNames);
                return;
            }
            
            if (!Response.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("GetLevelNames: HTTP response is invalid"));
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HTTP Response Invalid"));
                
                // Execute callback with empty array
                Callback.ExecuteIfBound(LevelNames);
                return;
            }

            int ResponseCode = Response->GetResponseCode();
            UE_LOG(LogTemp, Display, TEXT("GetLevelNames: HTTP Response Code: %d"), ResponseCode);
            
            FString ResponseContent = Response->GetContentAsString();
            
            // Log truncated response for debugging
            FString TruncatedResponse = ResponseContent;
            if (TruncatedResponse.Len() > 500)
            {
                TruncatedResponse = TruncatedResponse.Left(500) + TEXT("...[truncated]");
            }
            
            UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Response Content: %s"), *TruncatedResponse);
            
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
                    FString::Printf(TEXT("Response Code: %d"), ResponseCode));

            if (ResponseCode == 200)
            {
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

                bool bDeserializeSuccess = FJsonSerializer::Deserialize(Reader, JsonObject);
                UE_LOG(LogTemp, Display, TEXT("GetLevelNames: JSON Deserialization: %s"), 
                    bDeserializeSuccess ? TEXT("Success") : TEXT("Failed"));
                
                if (bDeserializeSuccess && JsonObject.IsValid())
                {
                    // Check if "items" field exists
                    if (JsonObject->HasField(TEXT("items")))
                    {
                        UE_LOG(LogTemp, Display, TEXT("GetLevelNames: 'items' field found in JSON"));
                        
                        // Fix for the TryGetArrayField issue
                        const TArray<TSharedPtr<FJsonValue>>* ItemsPtr = nullptr;
                        bool bHasItemsArray = JsonObject->TryGetArrayField(TEXT("items"), ItemsPtr);
                        
                        UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Items array retrieval: %s, ItemsPtr valid: %s"), 
                            bHasItemsArray ? TEXT("Success") : TEXT("Failed"),
                            (ItemsPtr != nullptr) ? TEXT("Yes") : TEXT("No"));
                        
                        if (bHasItemsArray && ItemsPtr != nullptr)
                        {
                            UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Found %d items in storage"), ItemsPtr->Num());
                            if (GEngine)
                                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                                    FString::Printf(TEXT("Found %d items in Firebase storage"), ItemsPtr->Num()));
                            
                            for (const TSharedPtr<FJsonValue>& Item : *ItemsPtr)
                            {
                                TSharedPtr<FJsonObject> ItemObj = Item->AsObject();
                                if (ItemObj.IsValid())
                                {
                                    FString Name;
                                    if (ItemObj->TryGetStringField(TEXT("name"), Name))
                                    {
                                        UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Found item named: %s"), *Name);
                                        
                                        if (Name.EndsWith(TEXT(".json")))
                                        {
                                            FString LevelName = Name.Left(Name.Len() - 5);
                                            LevelNames.Add(LevelName);
                                            UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Added level name: %s"), *LevelName);
                                        }
                                        else
                                        {
                                            UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Skipping non-JSON file: %s"), *Name);
                                        }
                                    }
                                    else
                                    {
                                        UE_LOG(LogTemp, Warning, TEXT("GetLevelNames: Item without 'name' field"));
                                    }
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("GetLevelNames: Invalid item object in array"));
                                }
                            }
                        }
                        else 
                        {
                            UE_LOG(LogTemp, Warning, TEXT("GetLevelNames: 'items' field is not a valid array"));
                            
                            // Try to log what the actual type is
                            TSharedPtr<FJsonValue> ItemsValue = JsonObject->TryGetField(TEXT("items"));
                            if(ItemsValue.IsValid())
                            {
                                UE_LOG(LogTemp, Warning, TEXT("GetLevelNames: 'items' field has type: %d"), (int)ItemsValue->Type);
                            }
                        }
                    }
                    else if (JsonObject->HasField(TEXT("prefixes")))
                    {
                        // Firebase might return 'prefixes' instead of 'items' for folder-style storage
                        UE_LOG(LogTemp, Display, TEXT("GetLevelNames: 'prefixes' field found (folder structure)"));
                        
                        const TArray<TSharedPtr<FJsonValue>>* PrefixesPtr = nullptr;
                        if (JsonObject->TryGetArrayField(TEXT("prefixes"), PrefixesPtr) && PrefixesPtr != nullptr)
                        {
                            UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Found %d prefixes"), PrefixesPtr->Num());
                            
                            for (const TSharedPtr<FJsonValue>& Prefix : *PrefixesPtr)
                            {
                                if (Prefix->Type == EJson::String)
                                {
                                    FString PrefixStr = Prefix->AsString();
                                    if (PrefixStr.EndsWith(TEXT(".json/")))
                                    {
                                        // Extract level name from prefix path
                                        FString LevelName = PrefixStr;
                                        LevelName.RemoveFromEnd(TEXT(".json/"));
                                        
                                        int32 LastSlashIndex;
                                        if (LevelName.FindLastChar('/', LastSlashIndex))
                                        {
                                            LevelName = LevelName.RightChop(LastSlashIndex + 1);
                                        }
                                        
                                        LevelNames.Add(LevelName);
                                        UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Added level name from prefix: %s"), *LevelName);
                                    }
                                }
                            }
                        }
                    }
                    else 
                    {
                        UE_LOG(LogTemp, Warning, TEXT("GetLevelNames: Neither 'items' nor 'prefixes' field found in JSON"));
                        
                        // Log all top-level keys to understand the structure
                        TArray<FString> AllKeys;
                        JsonObject->Values.GetKeys(AllKeys);
                        
                        FString KeysString = TEXT("Keys: ");
                        for (const FString& Key : AllKeys)
                        {
                            KeysString += Key + TEXT(", ");
                        }
                        
                        UE_LOG(LogTemp, Display, TEXT("GetLevelNames: JSON structure - %s"), *KeysString);
                    }
                }
                else 
                {
                    UE_LOG(LogTemp, Error, TEXT("GetLevelNames: Failed to parse JSON response"));
                    if (GEngine)
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to parse JSON response"));
                }
            }
            else if (ResponseCode == 401 || ResponseCode == 403)
            {
                // Auth error, force token refresh and try again
                UE_LOG(LogTemp, Warning, TEXT("GetLevelNames: Auth error (code %d) - refreshing token"), ResponseCode);
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                        FString::Printf(TEXT("Auth error %d - refreshing token"), ResponseCode));
                
                // Reset token and force refresh
                const_cast<UHLE_SaveLoad*>(this)->FirebaseAuthToken = TEXT("");
                const_cast<UHLE_SaveLoad*>(this)->TokenExpiration = 0.0f;
                const_cast<UHLE_SaveLoad*>(this)->AuthenticateWithFirebase();
                
                // Log auth status
                if (FirebaseAuthToken.IsEmpty())
                {
                    UE_LOG(LogTemp, Error, TEXT("GetLevelNames: Still no valid auth token after refresh"));
                }
                
                // Schedule a retry after a delay
                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Callback]()
                {
                    UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Retrying after auth refresh"));
                    if (GEngine)
                        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Retrying GetLevelNames..."));
                    
                    GetLevelNames(Callback);
                }, 2.0f, false);
                
                return; // Exit without calling the callback yet
            }
            else 
            {
                UE_LOG(LogTemp, Error, TEXT("GetLevelNames: Unexpected response code: %d"), ResponseCode);
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                        FString::Printf(TEXT("Unexpected response code: %d"), ResponseCode));
            }

            // Final logging
            UE_LOG(LogTemp, Display, TEXT("GetLevelNames: Found %d level names"), LevelNames.Num());
            
            FString NamesString = TEXT("Level names: ");
            for (const FString& Name : LevelNames)
            {
                NamesString += Name + TEXT(", ");
            }
            
            if (LevelNames.Num() > 0)
            {
                UE_LOG(LogTemp, Display, TEXT("GetLevelNames: %s"), *NamesString);
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        FString::Printf(TEXT("Found %d levels: %s"), LevelNames.Num(), *NamesString));
            }
            else 
            {
                UE_LOG(LogTemp, Warning, TEXT("GetLevelNames: No level names found"));
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No level names found"));
            }

            // Execute the callback with the level names
            Callback.ExecuteIfBound(LevelNames);
        }
    );

    // Send the request
    Request->ProcessRequest();
    UE_LOG(LogTemp, Display, TEXT("GetLevelNames: HTTP request sent"));
    
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Level names HTTP request sent"));
}