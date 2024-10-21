#include "LaserComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ULaserComponent::ULaserComponent() {
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void ULaserComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UseLaser();
}

//Code for shooting the laser from component origin
void ULaserComponent::UseLaser()
{
	//Variables for laser
	FHitResult Hit;
	FVector StartLocation = GetComponentLocation();
	FVector EndLocation = GetComponentLocation() + (GetForwardVector() * 10000000);

	//Bounce laser a max of 'i' times
	for (int i = 0; i < MaxBounces; i++) {
		//Shoot laser
		if (GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_Camera)) {
			DrawDebugLine(GetWorld(), StartLocation, Hit.Location, LaserColor, false, GetWorld()->GetDeltaSeconds(), 0, 5);

			//Bounce laser
			StartLocation = Hit.Location;
			EndLocation = UKismetMathLibrary::MirrorVectorByNormal(EndLocation, Hit.Normal) + (GetForwardVector() * 100000);

			//Break out of loop of hit component is not mirror
			if (!Hit.Component->ComponentHasTag("Mirror")) break;
		}
		else {
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, LaserColor, false, GetWorld()->GetDeltaSeconds(), 0, 5);
		}

		LaserHitResult = Hit;
	}
}

