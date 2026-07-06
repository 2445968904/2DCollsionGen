#pragma once

#include "Components/ActorComponent.h"
#include "Box2DCollisionProfile.h"
#include "Box2DWorldComponent.h"
#include "Box2DCollisionProfileComponent.generated.h"

// Runtime component that instantiates a UBox2DCollisionProfile on a UBox2DWorldComponent
UCLASS(ClassGroup=(Box2D), meta=(BlueprintSpawnableComponent))
class BOX2DPLUGIN_API UBox2DCollisionProfileComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBox2DCollisionProfileComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    UBox2DCollisionProfile* CollisionProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    bool bAutoCreateOnBeginPlay = true;

    // Create all bodies/shapes/joints from the profile
    UFUNCTION(BlueprintCallable, Category = Box2D)
    void InstantiateProfile();

    // Destroy all created bodies and joints
    UFUNCTION(BlueprintCallable, Category = Box2D)
    void DestroyInstantiatedBodies();

    // Get the body handle by name
    UFUNCTION(BlueprintPure, Category = Box2D)
    FBox2DBodyHandle GetBodyHandle(FName BodyName) const;

    // Check if the profile has been instantiated
    UFUNCTION(BlueprintPure, Category = Box2D)
    bool IsInstantiated() const { return bIsInstantiated; }

private:
    UPROPERTY()
    UBox2DWorldComponent* CachedWorldComponent = nullptr;

    TMap<FName, FBox2DBodyHandle> BodyHandleMap;
    TArray<FBox2DJointHandle> CreatedJointHandles;
    bool bIsInstantiated = false;

    UBox2DWorldComponent* FindWorldComponent();
};
