#include "LaserComponent.h"

#include "DrawDebugHelpers.h"
#include "JsonObjectConverter.h"
#include "ObjectProperties.h"
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
    
    // Create a temporary list of actors to ignore for this laser trace
    TArray<AActor*> ActorsToIgnore;
    
    // Bounce laser a max of 'MaxBounces' times
    for (int i = 0; i < MaxBounces; i++) {
        // Setup trace parameters with actors to ignore
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActors(ActorsToIgnore);
        
        // Shoot laser
        GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_Camera, QueryParams);
        LaserHitResult = Hit;

        if (Hit.bBlockingHit) {
            // Create segment to the hit point
            CreateSegment(StartLocation, Hit.Location, i);
            
            // Get the hit actor
            AActor* HitActor = Hit.GetActor();
            if (!HitActor) {
                break; // No actor hit
            }
            
            // PRIORITY 1: Check for mirrors first (before any color checking)
            // This ensures mirrors always work regardless of color
            if (Hit.Component.IsValid() && Hit.Component->ComponentHasTag("Mirror")) {
                // Bounce laser off mirror
                StartLocation = Hit.Location;
                FVector ReflectedDirection = UKismetMathLibrary::MirrorVectorByNormal(
                    (EndLocation - StartLocation).GetSafeNormal(), 
                    Hit.Normal
                );
                EndLocation = StartLocation + ReflectedDirection * 100000;
                
                // Continue to next iteration with new direction
                continue;
            }
            
            // PRIORITY 2: If not a mirror, check for color-based interaction
            UObjectProperties* ObjProperties = HitActor->FindComponentByClass<UObjectProperties>();
            UObjectProperties* LaserProperties = GetOwner()->FindComponentByClass<UObjectProperties>();
            
            // If both objects have property components, check colors
            if (ObjProperties && LaserProperties) {
                // If colors match, pass through
                if (ObjProperties->ObjectColor == LaserProperties->ObjectColor) {
                    // Add this actor to the ignore list
                    ActorsToIgnore.AddUnique(HitActor);
                    
                    // Pass through this object of the same color
                    StartLocation = Hit.Location;
                    
                    // Continue in the same direction
                    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
                    EndLocation = StartLocation + Direction * 100000;
                    continue; // Continue to next iteration
                }
                // If colors don't match, block the laser
                else {
                    // Different color = blocked
                    break;
                }
            }
            
            // If we reach here, it's a hit with no special properties
            // Default behavior is to block the laser
            break;
        }
        else {
            // No hit, create segment to the end point
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