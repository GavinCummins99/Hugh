#include "LaserComponent.h"

#include "DrawDebugHelpers.h"
#include "JsonObjectConverter.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
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

void ULaserComponent::UseLaser()
{
    // Clear existing laser segments
    ClearLaserSegments();
    
    // Variables for laser
    FVector StartLocation = GetComponentLocation();
    FVector EndLocation = GetComponentLocation() + (GetForwardVector() * 100000);
    FHitResult Hit;

    // Bounce laser a max of 'i' times
    for (int i = 0; i < MaxBounces; i++) {
        // Shoot laser
        GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_Camera);
        LaserHitResult = Hit;

        if (Hit.bBlockingHit) {
            // Create segment to the hit point
            CreateSegment(StartLocation, Hit.Location, i);

            // Bounce laser
            StartLocation = Hit.Location;
            FVector ReflectedDirection = UKismetMathLibrary::MirrorVectorByNormal(
                (EndLocation - StartLocation).GetSafeNormal(), 
                Hit.Normal
            );
            EndLocation = StartLocation + ReflectedDirection * 100000;

            // Break out of loop if hit component is not mirror
            if (!Hit.Component->ComponentHasTag("Mirror")) 
                break;
        }
        else {
            // Create segment to the end point (no hit)
            CreateSegment(StartLocation, EndLocation, i);
            break;
        }
    }
}

void ULaserComponent::CreateSegment(FVector Start, FVector End, int32 SegmentIndex)
{
    // Create unique name for each segment
    FString SegmentName = FString::Printf(TEXT("LaserMesh_%d"), SegmentIndex);
    
    UStaticMeshComponent* LaserMeshComponent = NewObject<UStaticMeshComponent>(
        this, 
        UStaticMeshComponent::StaticClass(),
        *SegmentName
    );

    //Setup laser mesh
    LaserMeshComponent->SetupAttachment(this);
    LaserMeshComponent->RegisterComponent();
    LaserMeshComponent->SetStaticMesh(LaserMesh);
    LaserMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Calculate the direction and length
    FVector Direction = End - Start;
    float Length = Direction.Size();
    FRotator Rotation = Direction.Rotation();
    
    // Position at start and orient toward end
    LaserMeshComponent->SetWorldLocation(Start);
    LaserMeshComponent->SetWorldRotation(Rotation);
    
    // Scale based on length and thickness
    LaserMeshComponent->SetRelativeScale3D(FVector(Length / 100, LaserThickness, LaserThickness));

    // Create dynamic material instance and set the color
    if (LaserMesh && LaserMesh->GetMaterial(0))
    {
        UMaterialInterface* OriginalMaterial = LaserMesh->GetMaterial(0);
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(OriginalMaterial, this);
        
        if (DynamicMaterial)
        {
            // Set the Color parameter (Vector4)
            // Using FLinearColor which can be easily converted to/from FVector4
            DynamicMaterial->SetVectorParameterValue("Color", LaserColor);
            
            // Apply the dynamic material to the mesh
            LaserMeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
    
    // Optionally store reference to delete later
    LaserSegments.Add(LaserMeshComponent);
}

// Add this function to clean up old segments
void ULaserComponent::ClearLaserSegments()
{
    for (UStaticMeshComponent* Segment : LaserSegments)
    {
        if (Segment)
        {
            Segment->DestroyComponent();
        }
    }
    LaserSegments.Empty();
}