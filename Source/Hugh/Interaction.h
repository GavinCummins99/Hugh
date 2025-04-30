#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"  
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Interaction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE (FInteractionDelagate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUGH_API UInteraction : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteraction();
	void OnComponentCreated();
	virtual void OnRegister() override;
	void BeginPlay();

	//Variables for interaction
	UPROPERTY(BlueprintAssignable) FInteractionDelagate OnInteract;
    UFUNCTION(BlueprintCallable) void Interact();
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float ColliderRadius = 250;
	UPROPERTY(BlueprintReadWrite) UWidgetComponent* WidgetComp;

private:
	UPROPERTY() USphereComponent* SphereCollision;

	UFUNCTION() void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,  int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION() void SetWidgetVisibility(bool Visibility);
	bool InRange;
};


