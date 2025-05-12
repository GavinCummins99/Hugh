#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "HLE_SaveLoad.generated.h"

// Delegate for level names callback
DECLARE_DYNAMIC_DELEGATE_OneParam(FLevelNamesCallback, const TArray<FString>&, LevelNames);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HUGH_API UHLE_SaveLoad : public UActorComponent
{
    GENERATED_BODY()

public:    
    // Sets default values for this component's properties
    UHLE_SaveLoad();

    // Called when the game starts
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Level Editor")
    void SaveLevel(FString LevelName) const;

    UFUNCTION(BlueprintCallable, Category = "Level Editor")
    void LoadLevel(FString LevelName, bool FromLocal = false);

    UFUNCTION(BlueprintCallable, Category = "Level Editor")
    void GetLevelNames(const FLevelNamesCallback& Callback);

    // Initialize HTTP Module
    void InitializeHTTPModule();

private:
    // Firebase Authentication methods
    void AuthenticateWithFirebase();
    void OnAuthenticationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    
    // Check if auth token is valid and refresh if needed
    bool EnsureValidAuthToken() const;
    
    // Apply auth token to HTTP request
    void ApplyAuthToRequest(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request) const;
    
    // Firebase authentication state
    mutable FString FirebaseAuthToken;
    mutable double TokenExpiration;
};