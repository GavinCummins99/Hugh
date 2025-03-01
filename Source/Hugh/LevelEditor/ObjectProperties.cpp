// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectProperties.h"

#include "Components/BoxComponent.h"
#include "Elements/Framework/TypedElementOwnerStore.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UObjectProperties::UObjectProperties()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UObjectProperties::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(Pushable);
	if (Pushable){
		// Get the owner actor and bind to its hit event
		if (AActor* Owner = GetOwner())
		{
			Owner->OnActorHit.AddDynamic(this, &UObjectProperties::OnParentHit);
		}

	}
	
}

void UObjectProperties::Push_Move(FVector TargetLocation) {
	FVector NewLocation = FMath::VInterpTo(GetOwner()->GetActorLocation(), TargetLocation, GetWorld()->DeltaTimeSeconds, 5);
	GetOwner()->SetActorLocation(NewLocation);
}

void UObjectProperties::OnParentHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	//Checks if touching player
	if(OtherActor->IsA(ACharacter::StaticClass())) {

		FHitResult HitResult;
		FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 40);
		FVector End;
		End = GetOwner()->GetActorLocation() - OtherActor->GetActorLocation();
		End.Z = 0;
		End = FRotator(0,FMath::RoundToInt(FRotationMatrix::MakeFromX(End).Rotator().Yaw / 90) * 90,0).Vector() * 20;
		FVector TargetLoc = (End) + GetOwner()->GetActorLocation();
		//End *= 20;
		End += Start;
		FVector HalfSize(30.0f, 30.0f, 30.0f); 
		FRotator Orientation = FRotator(0, 0, 0);
		GEngine->AddOnScreenDebugMessage(21, 5, FColor::Green, End.ToString());

		GEngine->AddOnScreenDebugMessage(20, 5, FColor::Red, End.ToString());

		if (UKismetSystemLibrary::UKismetSystemLibrary::BoxTraceSingle(GetWorld(), Start, End, HalfSize, Orientation,UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_EngineTraceChannel2),false, TArray<AActor*>(), EDrawDebugTrace::ForDuration,HitResult,true,FLinearColor::Red,FLinearColor::Green,5.0f)) {
			Push_Move(TargetLoc);
		}
		else{
			Push_Move(TargetLoc);
		}
	}
}


// Called every frame
void UObjectProperties::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(20, 5, FColor::Red, "I am ticking");
	//GetOwner()->AddActorLocalOffset(FVector(0, 0, .1));
}

