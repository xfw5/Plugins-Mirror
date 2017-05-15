// Fill out your copyright notice in the Description page of Project Settings.

#include "MirrorRuntime.h"
#include "MirrorComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"

#if WITH_EDITOR
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#endif
#include "MirrorUtils.h"

// Sets default values for this component's properties
UMirrorComponent::UMirrorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	UpdateTransformTolerance = 0.0001f;
	bLockedUpdate = false;
	bTeleportPhysics = false;
}

void UMirrorComponent::OnRegister()
{
	Super::OnRegister();

	if (GetOwner() && GetOwner()->GetRootComponent())
	{
		GetOwner()->GetRootComponent()->TransformUpdated.AddUObject(this, &UMirrorComponent::TransformUpdate);
	}
}

void UMirrorComponent::OnUnregister()
{
	Super::OnUnregister();

	if (GetOwner() && GetOwner()->GetRootComponent())
	{
		GetOwner()->GetRootComponent()->TransformUpdated.RemoveAll(this);
	}
}

bool UMirrorComponent::GetMirrorLocationAndRotation(FVector& OutLocation, FRotator& OutRotation)
{
	if (MirrorCenter == nullptr) return false;

	const FVector& CurLocation = GetOwner()->GetActorLocation();
	const FRotator& CurRotation = GetOwner()->GetActorRotation();

	const FVector& MirrorAxis = MirrorCenter->GetActorUpVector();
	const FVector& CenterLocation = MirrorCenter->GetActorLocation();

	FVector CenterToActor = CurLocation - CenterLocation;
	FVector CenterToMirror = CenterToActor.RotateAngleAxis(180, MirrorAxis);
	OutLocation = CenterToMirror + CenterLocation;

	FVector Delta = MirrorAxis * 180.f;
	FRotator DeltaRotation = FRotator(Delta.Y, Delta.Z, Delta.X);
	OutRotation = FRotator(FQuat(DeltaRotation) * FQuat(CurRotation));
	return true;
}

void UMirrorComponent::TransformUpdate(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	if (!MirrorActor || bLockedUpdate) return;

	bLockedUpdate = true;
	if (MirrorSpace == EMirrorSpace::LocalSpace)
	{
		const FTransform RelativeTransform = GetOwner()->GetRootComponent()->GetRelativeTransform();
		const FTransform MirrorRelativeTransform = MirrorActor->GetRootComponent()->GetRelativeTransform();
		const FVector CurrentLocation = RelativeTransform.GetLocation();
		const FQuat CurrentRotation = RelativeTransform.GetRotation();
		const FVector& MirrorLocation = MirrorRelativeTransform.GetLocation();
		const FQuat& MirrrorRotation = MirrorRelativeTransform.GetRotation();

		if (!MirrorLocation.Equals(CurrentLocation, UpdateTransformTolerance))
		{
			MirrorActor->SetActorRelativeLocation(CurrentLocation);
			MirrorActor->Modify();
		}

		if (!MirrrorRotation.Equals(CurrentRotation, UpdateTransformTolerance))
		{
			MirrorActor->SetActorRelativeRotation(CurrentRotation);
		}
	}
	else if (MirrorCenter != nullptr)
	{
		const FVector& MirrorLocation = MirrorActor->GetActorLocation();
		const FRotator& MirrrorRotation = MirrorActor->GetActorRotation();

		FVector DesireMirrorLocation;
		FRotator DesireMirrorRotation;
		if (GetMirrorLocationAndRotation(DesireMirrorLocation, DesireMirrorRotation))
		{
			if (!DesireMirrorLocation.Equals(MirrorLocation, UpdateTransformTolerance) || 
				!DesireMirrorRotation.Equals(MirrrorRotation, UpdateTransformTolerance))
			{
				MirrorActor->SetActorLocationAndRotation(DesireMirrorLocation, DesireMirrorRotation, false, nullptr, \
												bTeleportPhysics ? ETeleportType::TeleportPhysics : ETeleportType::None);
			}
		}
	}

	bLockedUpdate = false;
}

#if WITH_EDITOR

bool UMirrorComponent::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentValue = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UMirrorComponent, MirrorCenter))
	{
		return ParentValue && MirrorSpace == EMirrorSpace::CenterSpace;
	}
	else
	{
		return ParentValue;
	}
}

void UMirrorComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bool bNeedRefreshCustomizationModule = false;

	if (PropertyChangedEvent.MemberProperty != nullptr)
	{
		const FName Member = PropertyChangedEvent.MemberProperty->GetFName();
		if (Member == GET_MEMBER_NAME_CHECKED(UMirrorComponent, MirrorActor) || 
			Member == GET_MEMBER_NAME_CHECKED(UMirrorComponent, MirrorCenter) ||
			Member == GET_MEMBER_NAME_CHECKED(UMirrorComponent, MirrorSpace))
		{
			bNeedRefreshCustomizationModule = true;

			if (MirrorActor && CheckPhysicsSetting(MirrorActor))
			{
				UMirrorComponent* MirrirComp = MirrorActor->FindComponentByClass<UMirrorComponent>();
				if (MirrirComp)
				{
					CopyMirrorSetting(MirrirComp);
				}
			}
		}
	}

	if (bNeedRefreshCustomizationModule)
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UMirrorComponent::CopyMirrorSetting(UMirrorComponent* MirrorComp)
{
	MirrorComp->MirrorCenter = MirrorCenter;
	MirrorComp->MirrorActor = GetOwner();
	MirrorComp->UpdateTransformTolerance = UpdateTransformTolerance;
	MirrorComp->bTeleportPhysics = bTeleportPhysics;
}

bool UMirrorComponent::CheckPhysicsSetting(AActor* InMirrorActor)
{
	if (InMirrorActor == nullptr) return true;

	if (MirrorUtils::IsSimulatePhysics(InMirrorActor) != MirrorUtils::IsSimulatePhysics(GetOwner()) ||
		MirrorUtils::IsEnableGravity(InMirrorActor) != MirrorUtils::IsEnableGravity(GetOwner()))
	{
		UE_LOG(LogTemp, Error, TEXT("Physics setting is different between mirror actors. Please fix: %s->%s"), \
			*GetOwner()->GetActorLabel(), *InMirrorActor->GetActorLabel());
		return false;
	}

	return true;
}
#endif
