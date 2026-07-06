#include "Box2DCollisionEditorPCH.h"
#include "Box2DCollisionProfileFactory.h"
#include "Box2DCollisionProfile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Box2DCollisionProfileFactory)

#define LOCTEXT_NAMESPACE "Box2DCollisionEditor"

UBox2DCollisionProfileFactory::UBox2DCollisionProfileFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bCreateNew = true;
    bEditAfterNew = true;
    SupportedClass = UBox2DCollisionProfile::StaticClass();
}

bool UBox2DCollisionProfileFactory::ConfigureProperties()
{
    return true;
}

UObject* UBox2DCollisionProfileFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    UBox2DCollisionProfile* NewProfile = NewObject<UBox2DCollisionProfile>(InParent, Class, Name, Flags | RF_Transactional);

    // Create a default static body with a 1m x 1m box
    NewProfile->AddDefaultBody(*FString::Printf(TEXT("Body_0")));
    if (NewProfile->Bodies.Num() > 0)
    {
        NewProfile->Bodies[0].BodyType = EBox2DBodyType::Static;
    }

    return NewProfile;
}

#undef LOCTEXT_NAMESPACE
