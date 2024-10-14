#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FColor LaserColor = FColor::Red;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int MaxBounces = 8;

protected:
	void UseLaser() const;
};
