#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class FBox2DCollisionEditor;
class UBox2DCollisionProfile;

class SBox2DBodyList : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SBox2DBodyList) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<class FBox2DCollisionEditor> InEditor);

private:
    TSharedRef<ITableRow> OnGenerateBodyRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<ITableRow> OnGenerateJointRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

    void OnBodySelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);
    void OnJointSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);

    FReply OnAddBody();
    FReply OnDeleteBody();
    bool CanDeleteBody() const;

    FReply OnAddJoint();
    FReply OnDeleteJoint();
    bool CanDeleteJoint() const;

    UBox2DCollisionProfile* GetProfile() const;
    void RefreshListData();

    TWeakPtr<FBox2DCollisionEditor> EditorPtr;

    int32 SelectedBodyIndex;
    int32 SelectedJointIndex;

    TArray<TSharedPtr<FString>> BodyItems;
    TArray<TSharedPtr<FString>> JointItems;

    TSharedPtr<SListView<TSharedPtr<FString>>> BodyListView;
    TSharedPtr<SListView<TSharedPtr<FString>>> JointListView;
};
