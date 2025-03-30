
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
	PrimaryComponentTick.bStartWithTickEnabled = true;

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
	FVector NewLocation = FMath::VInterpTo(GetOwner()->GetActorLocation(), TargetLocation, GetWorld()->DeltaTimeSeconds, 3);
	GetOwner()->SetActorLocation(NewLocation);
}

//Called when touching the object
void UObjectProperties::OnParentHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	//Checks if touching player
	if(OtherActor->IsA(ACharacter::StaticClass())) {

		FHitResult HitResult;
		FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 40);
		FVector End;
		End = GetOwner()->GetActorLocation() - OtherActor->GetActorLocation();
		End.Z = GetOwner()->GetActorLocation().Z;
		End = FRotator(0,FMath::RoundToInt(FRotationMatrix::MakeFromX(End).Rotator().Yaw / 90) * 90,0).Vector();
		FVector TargetLoc = (End) + GetOwner()->GetActorLocation();
		End *= 40;
		End += Start;
		FVector HalfSize(35.0f, 35.0f, 35.0f); 
		FRotator Orientation = FRotator(0, 0, 0);
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(GetOwner()); 

		//Do box check
		if (!UKismetSystemLibrary::UKismetSystemLibrary::BoxTraceSingle(GetWorld(), Start, End, HalfSize, Orientation,UEngineTypes::ConvertToTraceType(ECC_Visibility),false, ActorsToIgnore, EDrawDebugTrace::ForDuration,HitResult,true,FLinearColor::Red,FLinearColor::Green,.1f)) {
			End.Z = GetOwner()->GetActorLocation().Z;
			Target = End;
			//Push_Move(End);

		}

	}
}


// Called every frame
void UObjectProperties::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(20, 5, FColor::Red, "I am ticking");
	Push_Move(Target);
}

void UObjectProperties::OnPlaced() {
	OnObjectPlaced.Broadcast();
	Target = GetOwner()->GetActorLocation();
	SetComponentTickEnabled(Pushable);
}