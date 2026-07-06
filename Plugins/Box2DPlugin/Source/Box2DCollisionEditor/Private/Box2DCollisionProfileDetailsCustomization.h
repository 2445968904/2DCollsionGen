#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UBox2DCollisionProfile;

class FBox2DCollisionProfileDetailsCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    FReply OnAutoGenerateSingleBox();
    FReply OnAutoGenerateSingleCircle();
    FReply OnAutoGenerateConvexPolygon();

    UBox2DCollisionProfile* GetProfile() const;

    TWeakObjectPtr<UBox2DCollisionProfile> CachedProfile;
};
