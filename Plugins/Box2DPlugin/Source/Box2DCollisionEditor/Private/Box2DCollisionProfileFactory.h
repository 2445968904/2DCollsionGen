#pragma once

#include "Factories/Factory.h"
#include "Box2DCollisionProfileFactory.generated.h"

UCLASS()
class UBox2DCollisionProfileFactory : public UFactory
{
    GENERATED_UCLASS_BODY()

    // UFactory interface
    virtual bool ConfigureProperties() override;
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
    // End of UFactory interface
};
