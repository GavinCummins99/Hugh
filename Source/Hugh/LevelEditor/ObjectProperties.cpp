
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
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...
}

void UObjectProperties::BeginPlay()
{
	Super::BeginPlay();

	//Target = GetOwner()->GetActorLocation();
	
	if (Pushable){
		// Get the owner actor and bind to its hit event
		if (AActor* Owner = GetOwner())
		{
			Owner->OnActorHit.AddDynamic(this, &UObjectProperties::OnParentHit);
		}

	}
	
}

//Move object if pushing
void UObjectProperties::Push_Move(FVector TargetLocation) {
	float ConstantSpeed = 100; // Units per second
	FVector NewLocation = FMath::VInterpConstantTo(GetOwner()->GetActorLocation(), TargetLocation, GetWorld()->DeltaTimeSeconds, ConstantSpeed);
    
	GetOwner()->SetActorLocation(NewLocation);
}

//Called when touching the object
void UObjectProperties::OnParentHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
    //Checks if touching player
    if(OtherActor->IsA(ACharacter::StaticClass())) {

    	TimeStartPush = GetWorld()->GetTimeSeconds();

       // Calculate the push direction (from player to box) and snap to 90 degrees
       FVector PushDirection = GetOwner()->GetActorLocation() - OtherActor->GetActorLocation();
       PushDirection.Z = 0; // Keep it flat horizontally
       PushDirection = FRotator(0, FMath::RoundToInt(FRotationMatrix::MakeFromX(PushDirection).Rotator().Yaw / 90) * 90, 0).Vector();
       PushDirection.Normalize();
       
       // Calculate trace direction (box to potential wall)
       FVector TraceDirection = PushDirection; // Same as push direction
       
       // Start position at the center of the cube
       FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 40);
       
       // End position far away in push direction
       FVector End = Start + (TraceDirection * 15);
       
       // Create a box size that matches your cube
       FVector HalfSize(35, 35, 35); // Using your 40cm value
       
       // No rotation needed for the box
       FRotator Orientation = FRotator::ZeroRotator;
       
       // Ignore the cube itself
       TArray<AActor*> ActorsToIgnore;
       ActorsToIgnore.Add(GetOwner());
       
       // Perform box trace
       FHitResult HitResult;
       bool bHit = UKismetSystemLibrary::BoxTraceSingle(
           GetWorld(), 
           Start, 
           End, 
           HalfSize, 
           Orientation,
           UEngineTypes::ConvertToTraceType(ECC_Visibility),
           false, 
           ActorsToIgnore, 
           EDrawDebugTrace::None,
           HitResult,
           true,
           FColor::Red,
           FColor::Green,
           1.0f
       );
       
    	if (bHit && HitResult.GetActor()->GetComponentByClass<UObjectProperties>()->ObjectColor != ObjectColor) {
    		// The most reliable approach: 
    		// Calculate maximum distance the box can move based on hit distance
    		float MaxMoveDistance = HitResult.Distance - 5; // Keep 10 units away from wall
            
    		// Ensure we don't move backward
    		MaxMoveDistance = FMath::Max(0.0f, MaxMoveDistance);
            
    		// Calculate target position
    		Target = GetOwner()->GetActorLocation() + (PushDirection * MaxMoveDistance);
    		Target.Z = GetOwner()->GetActorLocation().Z; // Maintain Z height
            
    		GEngine->AddOnScreenDebugMessage(1236, 2.0f, FColor::Red, 
				FString::Printf(TEXT("Max move distance: %f"), MaxMoveDistance));
    	} else if (!bHit || HitResult.GetActor()->GetComponentByClass<UObjectProperties>()->MetaTages.Contains("CanPhase")){
    		// No wall detected, move in push direction
    		Target = Start + (PushDirection * 10); // Move a predefined distance
    		Target.Z = GetOwner()->GetActorLocation().Z;
    	}
    }
}


// Called every frame
void UObjectProperties::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(20, 5, FColor::Red, "I am ticking");
	CheckGround();
	Push_Move(Target);

	IsPushing = (GetWorld()->GetTimeSeconds() - TimeStartPush) < 0.2f;
}

void UObjectProperties::CheckGround(){
	FHitResult HitResult;
	FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 40);
	FVector End = Start - GetOwner()->GetActorUpVector() * 10;
	FVector HalfSize(35.0f, 35.0f, 35.0f); 
	FRotator Orientation = FRotator(0, 0, 0);
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	
	if (!UKismetSystemLibrary::UKismetSystemLibrary::BoxTraceSingle(GetWorld(), Start, End, HalfSize, Orientation,UEngineTypes::ConvertToTraceType(ECC_Visibility),false, ActorsToIgnore, EDrawDebugTrace::None,HitResult,true,FLinearColor::Red,FLinearColor::Green,.1f)){
		Target.Z -= 10;
	}
	else{
		if (HitResult.GetActor()->GetComponentByClass<UObjectProperties>()->ObjectColor != ObjectColor){
			Target.Z = HitResult.ImpactPoint.Z;
		}
	}
}

void UObjectProperties::PowerObject(FColor PowerColor) {
	IsPowered = true;
	OnPowered.Broadcast(PowerColor);
}

void UObjectProperties::UnpowerObject() {
	IsPowered = false;
	OnPowerStop.Broadcast();
}

void UObjectProperties::OnPlaced() {
	IsPlaced = true;
	OnObjectPlaced.Broadcast();
	Target = GetOwner()->GetActorLocation();
	SetComponentTickEnabled(Pushable);
}

//Helper function for snapping vector to grid
FVector UObjectProperties::Snap(FVector InVector) {
	float GridSize = 20;
	float extra = 0;
	return FVector(
	FMath::RoundToFloat(InVector.X / GridSize) * GridSize,
	FMath::RoundToFloat(InVector.Y / GridSize) * GridSize,
	InVector.Z
	);

	//FMath::RoundToFloat(InVector.Z / GridSize) * GridSize
	// + (Cast<AHughLevelEditor>(GetOwner())->ObjectProperties->GridSnap) - FVector::OneVector * 100
}
