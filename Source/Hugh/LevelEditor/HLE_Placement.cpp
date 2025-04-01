// Fill out your copyright notice in the Description page of Project Settings.


#include "HLE_Placement.h"
#include "HughLevelEditor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Media/Public/IMediaControls.h"

// Sets default values for this component's properties
UHLE_Placement::UHLE_Placement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHLE_Placement::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UHLE_Placement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	Trace();
	if (CurrentObject && Cast<AHughLevelEditor>(GetOwner())->EditorMode == Modes::Building){
		CurrentObject->SetHidden(false);
		if (IsPlacing){
			//CurrentObject->SetActorLocation(Snap(CursorStartLoc));
			//CurrentObject->SetActorLocation(Snap(CursorLoc));
			//CurrentObject->SetActorRotation(CurrentObject->GetComponentByClass<UObjectProperties>()->AllowRotation? FRotator(0,TargetYawRotation,0) : FRotator::ZeroRotator);

		} else{
			CurrentObject->SetActorLocation(Snap(CursorLoc));
			//FRotator TargetRotation = CurrentObject->GetComponentByClass<UObjectProperties>()->AllowRotation? FRotator(0,TargetYawRotation,0) : FRotator::ZeroRotator;
			//CurrentObject->SetActorRotation(FMath::RInterpTo(CurrentObject->GetActorRotation(), TargetRotation, GetWorld()->GetTimeSeconds(), 5));
			//CurrentObject->SetActorRotation(CurrentObject->GetComponentByClass<UObjectProperties>()->AllowRotation? FRotator(0,TargetYawRotation,0) : FRotator::ZeroRotator);
		}
	}
	else {
		CurrentObject->SetHidden(true);
	}
	//TargetRotation = CurrentObject->GetComponentByClass<UObjectProperties>()->AllowRotation? FRotator(0,TargetYawRotation,0) : FRotator::ZeroRotator;
	//FRotator TargetRotation = CurrentObject->GetComponentByClass<UObjectProperties>()->AllowRotation? FRotator(0,TargetYawRotation,0) : FRotator::ZeroRotator;
	CurrentObject->SetActorRotation(FMath::RInterpTo(CurrentObject->GetActorRotation(), TargetRotation, DeltaTime, 10));
	//CurrentObject->SetActorRotation(TargetRotation);
}

//Handel's main line trace from the camera to world space
void UHLE_Placement::Trace() {
	// Get the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
		return;

	// Get the viewport size
	FVector2D ViewportSize;
	GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);

	// Get the mouse position
	float MouseX, MouseY;
	PlayerController->GetMousePosition(MouseX, MouseY);

	// Deproject the screen position of the mouse to world space
	FVector WorldLocation, WorldDirection;
	PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);
	
	// Perform a line trace
	FHitResult Hit;
	const FVector Start = WorldLocation;
	const FVector End = WorldLocation + WorldDirection * 10000.0f; 
	FCollisionQueryParams QueryParams; QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(CurrentObject);

	// Ignore all spawned objects during placement
	if (IsPlacing) {
		for (auto& Pair : SpawnedObjects) {
			if (IsValid(Pair.Value)) {
				QueryParams.AddIgnoredActor(Pair.Value);
			}
		}
	}

	//Setup plane
	FVector PlaneOrigin = FVector(0,0,0);
	FVector PlaneNormal = FVector(0,0,1);

	//Out values for intersetion
	FVector IntersectionPoint;
	float T;

	//For for immediate line trace hit
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams)) {
		PlaneOrigin = Hit.Location;
		HoveredObject = Hit.GetActor();

		///+ (Hit.ImpactNormal * 25)
		if (!IsPlacing)CursorLoc = Hit.ImpactPoint;
		else if (UKismetMathLibrary::LinePlaneIntersection(Start, End, FPlane(PlaneOrigin, PlaneNormal), T, IntersectionPoint)) {
			CursorLoc = IntersectionPoint;
		}

		
		/*DisplayMesh->SetWorldLocation(Snap(Hit.ImpactPoint + (Hit.ImpactNormal * 25)));

		if(EditorMode != Modes::Building) {
			if(HoveredObject && !TaggedObjects.Contains(HoveredObject) && !SelectedObjects.Contains(HoveredObject) && EditorMode == Modes::Removing) HoveredObject->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);////////////////////////////////
			HoveredObject = Hit.GetActor();
			if(EditorMode == Modes::Removing) Hit.GetActor()->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(EditorMode == Modes::Removing? RemovingMaterial : EditingMaterial);//////////////////
		}
		*/

		//Sets the new scale of the object
		if (false){
			FVector NewScale = FVector(.1) + (CursorStartLoc - CursorLoc).Length() * .01f;
			CurrentObject->SetActorScale3D(IsPlacing ? NewScale : FVector::One());
		}
		else{
			CurrentObject->SetActorScale3D(FVector(1));
		}

		//Sets the new Rotation of the object
		if (CurrentObject->GetComponentByClass<UObjectProperties>()->AlignNormalToface){
			FRotator NewRotation = Hit.Normal.Rotation(); 
			NewRotation.Pitch -= 90.0f;
			TargetRotation = NewRotation;
			//CurrentObject->SetActorRotation(NewRotation);
		}
		else {
			//TargetRotation = FRotator(0,TargetYawRotation,0);
			TargetRotation = CurrentObject->GetComponentByClass<UObjectProperties>()->AllowRotation? FRotator(0,TargetYawRotation,0) : FRotator::ZeroRotator;

			//CurrentObject->SetActorRotation(FRotator(0,TargetYawRotation,0));
		}


		if (IsPlacing && Cast<AHughLevelEditor>(GetOwner())->EditorMode == Modes::Editing){
			SetMaterial(Hit.GetActor(), Cast<AHughLevelEditor>(GetOwner())->EditingMaterial);
			Cast<AHughLevelEditor>(GetOwner())->SelectedObjects.Add(Hit.GetActor());
		}

		

		
		GEngine->AddOnScreenDebugMessage(111, 5.f, FColor::Blue, "Dist : " + FString::SanitizeFloat((CursorStartLoc - CursorLoc).Length()));

	}
	//If no immediate hit check for point along plane 
	else if (UKismetMathLibrary::LinePlaneIntersection(Start, End, FPlane(PlaneOrigin, PlaneNormal), T, IntersectionPoint)) {
		CursorLoc = IntersectionPoint;
		//DisplayMesh->SetWorldLocation(Snap(IntersectionPoint + (Hit.ImpactNormal * 25)));
		//if(HoveredObject && !SelectedObjects.Contains(HoveredObject)) HoveredObject->GetComponentByClass<UStaticMeshComponent>()->SetOverlayMaterial(nullptr);
	}
	/*
	if (IsPlacing){
	XLen = (CurrentObject->GetActorLocation().X - CursorStartLoc.X) / 100;
	YLen = (CurrentObject->GetActorLocation().Y - CursorStartLoc.Y)/ 100;
		for (int X = CachedXLen; X < XLen; X++){
			for (int Y = CachedYLen; Y < YLen; Y++){
				//Create Objects
				FVector Location = FVector(CursorStartLoc.X + (X * 100 * (XLen>0?1:-1)),CursorStartLoc.Y + (Y * 100 * (YLen>0?1:-1)), CursorStartLoc.Z);
			
				const FActorSpawnParameters SpawnInfo;
				AActor* SpawnedObject = GetWorld()->SpawnActor(CurrentObject->GetClass(), &Location, &FRotator::ZeroRotator, SpawnInfo);
				//SpawnedObject->SetActorScale3D(Scale);
			}
		}
	CachedXLen = XLen;
	CachedYLen = YLen;
	}
	*/
	if (Cast<AHughLevelEditor>(GetOwner())->EditorMode == Modes::Building) PlaceObjects();
	GEngine->AddOnScreenDebugMessage(111, 5.f, FColor::Green, "LEnghtssssssss : " + FString::FromInt(XLen) + " / " + FString::FromInt(YLen));


	
}

void UHLE_Placement::StartPlacement(){
	if (Cast<AHughLevelEditor>(GetOwner())->EditorMode == Modes::Building || Cast<AHughLevelEditor>(GetOwner())->EditorMode ==Modes::Editing) {
		CursorStartLoc = Snap(CursorLoc);
		IsPlacing = true;
		GEngine->AddOnScreenDebugMessage(101, 5.f, FColor::Blue, "Start : " + CursorStartLoc.ToString());
	}
	else if (Cast<AHughLevelEditor>(GetOwner())->EditorMode == Modes::Removing) {
		HoveredObject->Destroy();
	}
}

void UHLE_Placement::EndPlacement(){
	CursorEndLoc = CursorLoc;
	IsPlacing = false;
	GEngine->AddOnScreenDebugMessage(100, 5.f, FColor::Blue, "End : " + CursorEndLoc.ToString());
	GEngine->AddOnScreenDebugMessage(100, 5.f, FColor::Blue, "Distance : " + (CursorStartLoc - CursorEndLoc).ToString());

	for (auto Element : SpawnedObjects) {
		AActor* Actor = Element.Value;
		UObjectProperties* Component = Actor->FindComponentByClass<UObjectProperties>();
    
		if (Component) {
			Component->OnPlaced();
		}
	}

	//CreateObjects();
	CachedXLen = 0;
	CachedYLen = 0;
	SpawnedObjects.Empty();
}

void UHLE_Placement::PlaceObjects()
{
    if (IsPlacing)
    {
        // Get properly snapped positions - use direct cursor positions
        FVector SnappedStartLoc = Snap(CursorStartLoc);
        
        // Important: Use CursorLoc directly, which should be the current mouse position
        // from your Trace() function
        FVector SnappedCurrentLoc = Snap(CursorLoc);
        
        // Calculate grid dimensions
        XLen = FMath::FloorToInt((SnappedCurrentLoc.X - SnappedStartLoc.X) / 100.0f);
        YLen = FMath::FloorToInt((SnappedCurrentLoc.Y - SnappedStartLoc.Y) / 100.0f);
        
        // Add debug messages to verify positions
        GEngine->AddOnScreenDebugMessage(201, 5.f, FColor::Cyan, 
            FString::Printf(TEXT("Start Pos: %s"), *SnappedStartLoc.ToString()));
        GEngine->AddOnScreenDebugMessage(202, 5.f, FColor::Cyan, 
            FString::Printf(TEXT("Current Pos: %s"), *SnappedCurrentLoc.ToString()));
        GEngine->AddOnScreenDebugMessage(203, 5.f, FColor::Cyan, 
            FString::Printf(TEXT("Grid Dimensions: %d x %d"), XLen, YLen));
        
        // Create a set of desired coordinates
        TSet<FIntPoint> DesiredCoords;
        
        // Add origin point
        DesiredCoords.Add(FIntPoint(0, 0));
        
        // Fill grid based on dimensions
        for (int32 X = (XLen < 0 ? XLen : 0); X <= (XLen > 0 ? XLen : 0); X++)
        {                                      
            for (int32 Y = (YLen < 0 ? YLen : 0); Y <= (YLen > 0 ? YLen : 0); Y++)
            {
                DesiredCoords.Add(FIntPoint(X, Y));
            }
        }
        
        // Create objects for new coordinates
        for (const FIntPoint& Coord : DesiredCoords)
        {
            if (!SpawnedObjects.Contains(Coord))
            {
                // Calculate exact world position
                FVector Location = FVector(
                    SnappedStartLoc.X + (Coord.X * 100.0f),
                    SnappedStartLoc.Y + (Coord.Y * 100.0f),
                    SnappedStartLoc.Z
                );
                
                // Spawn the object
                FActorSpawnParameters SpawnInfo;
            	//FRotator Rotator = FRotator(0,TargetYawRotation,0);

                //AActor* SpawnedObject = GetWorld()->SpawnActor(CurrentObject->GetClass(), &Location, CurrentObject->GetComponentByClass<UObjectProperties>()->AllowRotation? &Rotator : &FRotator::ZeroRotator, SpawnInfo);
            	AActor* SpawnedObject = GetWorld()->SpawnActor(CurrentObject->GetClass(), &Location, &TargetRotation, SpawnInfo);

                // Store in our map
                SpawnedObjects.Add(Coord, SpawnedObject);
            	
            	///
            	if (SpawnedObject)
            	{
            		//UObjectProperties* SourceOP = AllObjects[ObjectIndex]->FindComponentByClass<UObjectProperties>();
            		UObjectProperties* TargetOP = SpawnedObject->FindComponentByClass<UObjectProperties>();

 
            		if (Cast<AHughLevelEditor>(GetOwner())->ObjectProperties && TargetOP)
            			//{
            			// Copy all properties from source to target
            			UEngine::CopyPropertiesForUnrelatedObjects(Cast<AHughLevelEditor>(GetOwner())->ObjectProperties, TargetOP);
            		//}
            	}
            }
        }
        
        // Remove objects that are no longer needed
        TArray<FIntPoint> CoordsToRemove;
        for (auto& Pair : SpawnedObjects)
        {
            if (!DesiredCoords.Contains(Pair.Key))
            {
                if (IsValid(Pair.Value))
                {
                    Pair.Value->Destroy();
                }
                CoordsToRemove.Add(Pair.Key);
            }
        }
        
        // Remove destroyed objects from our map
        for (const FIntPoint& Coord : CoordsToRemove)
        {
            SpawnedObjects.Remove(Coord);
        }
        
        // Update cached dimensions
        CachedXLen = XLen;
        CachedYLen = YLen;



    }
}

//Creates either a single or a group of objects
void UHLE_Placement::CreateObjects(){

	//Transforms / parms
	const FVector Loc = CurrentObject->GetActorLocation();
	const FRotator Rot = CurrentObject->GetActorRotation();
	const FVector Scale = CurrentObject->GetActorScale3D();
	const FActorSpawnParameters SpawnInfo;

	//Create Objects
	AActor* SpawnedObject = GetWorld()->SpawnActor(CurrentObject->GetClass(), &Loc, &Rot, SpawnInfo);
	SpawnedObject->SetActorScale3D(Scale);
}

//Helper function for snapping vector to grid
FVector UHLE_Placement::Snap(FVector InVector) {
	float GridSize = 100;
	float extra = 0;
	return FVector(
	FMath::RoundToFloat(InVector.X / GridSize) * GridSize,
	FMath::RoundToFloat(InVector.Y / GridSize) * GridSize,
	InVector.Z
	);

	//FMath::RoundToFloat(InVector.Z / GridSize) * GridSize
	// + (Cast<AHughLevelEditor>(GetOwner())->ObjectProperties->GridSnap) - FVector::OneVector * 100
}


void UHLE_Placement::SetMaterial(AActor* Actor, UMaterialInterface* OverlayMaterial)
{
	if (!Actor)
		return;
    
	// Find all Static Mesh Components
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
	// Apply overlay material to all Static Mesh Components
	for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->SetOverlayMaterial(OverlayMaterial);
		}
	}
    
	// Find all Skeletal Mesh Components
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
	// Apply overlay material to all Skeletal Mesh Components
	for (USkeletalMeshComponent* SkeletalComp : SkeletalMeshComponents)
	{
		if (SkeletalComp)
		{
			SkeletalComp->SetOverlayMaterial(OverlayMaterial);
		}
	}
}

//Rotates object
void UHLE_Placement::RotateObject() {
	TargetYawRotation += 45;
}
