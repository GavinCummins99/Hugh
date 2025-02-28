// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectProperties.h"

// Sets default values for this component's properties
UObjectProperties::UObjectProperties()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UObjectProperties::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UObjectProperties::Push_Move(FVector TargetLocation) {
	FVector NewLocation = FMath::VInterpTo(GetOwner()->GetActorLocation(), NewLocation, GetWorld()->DeltaTimeSeconds, 10);
	GetOwner()->SetActorLocation(NewLocation);
}


// Called every frame
void UObjectProperties::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "I am ticking");
}

