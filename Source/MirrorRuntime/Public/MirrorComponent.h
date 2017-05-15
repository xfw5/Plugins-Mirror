// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "MirrorComponent.generated.h"

UENUM()
namespace EMirrorSpace
{
	enum Type
	{
		/* Mirror around the center actor. */
		CenterSpace,

		/* Mirror at each other local space. */
		LocalSpace,
	};
}

/*
* Transform Mirror Actors. When one actor transform changed, the other perform the same changed.
* Physics simulate is supported.(Required both physics simulate turn on.)
*/
UCLASS( ClassGroup=(Mirror), meta=(BlueprintSpawnableComponent) )
class MIRRORRUNTIME_API UMirrorComponent : public UActorComponent
{
	GENERATED_BODY()

	/* Tolerance for update transform. */
	UPROPERTY(EditInstanceOnly, category = "Mirror", meta = (UIMin = 0.0f, UIMax = 0.0001f, ClampMin = 0.0f, ClampMax = 0.0001f))
	float UpdateTransformTolerance;

public:	
	// Sets default values for this component's properties
	UMirrorComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Mirror")
	TEnumAsByte<EMirrorSpace::Type> MirrorSpace;

	/* The center going mirror around. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Mirror")
	AActor* MirrorCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Mirror")
	AActor* MirrorActor;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, category = "Mirror")
	uint32 bTeleportPhysics : 1;

	/* Locked transform update notify to avoid infinite update cause of mirror actor referenced by each other. */
	UPROPERTY(Transient)
	uint32 bLockedUpdate : 1;

public:	

	virtual void OnRegister() override;

	virtual void OnUnregister() override;

	UFUNCTION(BlueprintCallable, category = "Component|Mirror")
	bool GetMirrorLocationAndRotation(FVector& OutLocation, FRotator& OutRotation);

	/* Hook transform dirty event. */
	void TransformUpdate(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);

	FORCEINLINE bool CanCreateMirror() { return MirrorSpace == EMirrorSpace::LocalSpace ? true: MirrorCenter != nullptr; }

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void CopyMirrorSetting(UMirrorComponent* MirrorComp);
	bool CheckPhysicsSetting(AActor* QueryMirrorActor);

#endif
};
