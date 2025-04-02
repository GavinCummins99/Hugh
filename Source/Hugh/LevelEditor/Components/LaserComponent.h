#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/HitResult.h"
#include "LaserComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUGH_API ULaserComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ULaserComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Global properties
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Laser") FColor LaserColor = FColor::Red;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Laser") int MaxBounces = 8;

	UPROPERTY(BlueprintReadOnly, Category = "Laser") FHitResult LaserHitResult;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UStaticMesh* LaserMesh;

protected:
	void UseLaser();
	void CreateSegment(FVector Start, FVector End, int32 SegmentIndex);
	float LaserThickness = 0.075;
	
	// Array to keep track of created segments
	UPROPERTY()
	TArray<UStaticMeshComponent*> LaserSegments;
    
	void ClearLaserSegments();
};
