// Fill out your copyright notice in the Description page of Project Settings.

#include "MirrorRuntime.h"
#include "MirrorUtils.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

bool MirrorUtils::IsSimulatePhysics(const AActor* InActor)
{
	if (InActor == nullptr) return false;

	TInlineComponentArray<UPrimitiveComponent*> Components;
	if (InActor->GetOwner() == nullptr) return false;

	InActor->GetOwner()->GetComponents(Components);

	for (UPrimitiveComponent* Component : Components)
	{
		if (Component->BodyInstance.bSimulatePhysics) return true;
	}

	return false;
}

bool MirrorUtils::IsEnableGravity(const class AActor* InActor)
{
	if (InActor == nullptr) return false;

	TInlineComponentArray<UPrimitiveComponent*> Components;
	if (InActor->GetOwner() == nullptr) return false;

	InActor->GetOwner()->GetComponents(Components);

	for (UPrimitiveComponent* Component : Components)
	{
		if (Component->BodyInstance.bEnableGravity) return true;
	}

	return false;
}
