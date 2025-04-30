#include "Interaction.h"

#include "ObjectProperties.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/Material.h"
#include "Serialization/JsonTypes.h"
#include "UObject/ConstructorHelpers.h"

class UMaterial;
//Sets default values for this component's properties
UInteraction::UInteraction(){
	// First set any primary component properties
	PrimaryComponentTick.bCanEverTick = true;

	//Create sphere collision
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// Then make absolutely sure all channels overlap
	FCollisionResponseContainer ResponseContainer;
	ResponseContainer.SetAllChannels(ECR_Overlap);
	SphereCollision->SetCollisionResponseToChannels(ResponseContainer);
	
	//Create widget component
	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	WidgetComp->SetupAttachment(this);
	WidgetComp->SetDrawSize(FVector2D(75,75));
	WidgetComp->SetMaterial(0, LoadObject<UMaterial>(nullptr, TEXT("/Game/UI/WidgetMaterialCustom.WidgetMaterialCustom")));
    
	//Load the widget class
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(TEXT("/Game/UI/Interaction"));
	if (WidgetClassFinder.Succeeded()) {
		WidgetComp->SetWidgetClass(WidgetClassFinder.Class);
	}
}

//Called when the interaction component is fully created 
void UInteraction::OnComponentCreated() {
	Super::OnComponentCreated();

	//Gets the owning actor and sets the root of the sphere collision
	if (AActor* Owner = GetOwner()) {
		if (USceneComponent* Root = Owner->GetRootComponent()) {
			SphereCollision->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	//Sets up spheres overlap events
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &UInteraction::OnOverlapBegin);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &UInteraction::OnOverlapEnd);
}

//Called when object properties updated
void UInteraction::OnRegister() {
	Super::OnRegister();
	SphereCollision->SetSphereRadius(ColliderRadius);
}

//Executes at start of play
void UInteraction::BeginPlay(){
	Super::BeginPlay();

	//Initially hide widget
	SetWidgetVisibility(false);
}

//For triggering the interaction
void UInteraction::Interact(){
    if (InRange) 
	OnInteract.Broadcast();
}

//When in range
void UInteraction::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor->IsA(APawn::StaticClass()) && GetOwner()->GetComponentByClass<UObjectProperties>()) {
		if (GetOwner()->GetComponentByClass<UObjectProperties>()->IsOnSlidingTile) {
			SetWidgetVisibility(true);
			InRange = true;
		}
	}
}

//When out of range
void UInteraction::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor->IsA(APawn::StaticClass())) {
		InRange = true;
		SetWidgetVisibility(false);
	}
}

//Sets the visibility of the interaction widget
void UInteraction::SetWidgetVisibility(bool Visibility){
	InRange = Visibility;
	WidgetComp->SetVisibility(Visibility);

}

