// Fill out your copyright notice in the Description page of Project Settings.

#include "MirrorEditor.h"
#include "MirrorComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "SBoxPanel.h"
#include "Reply.h"
#include "DeclarativeSyntaxSupport.h"
#include "Internationalization.h"
#include "STextBlock.h"
#include "SButton.h"
#include "MirrorComponent.h"
#include "IDetailsView.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/Selection.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "MirrorComponentDetails"

FMirrorComponentDetails::FMirrorComponentDetails()
{
	LastDetailLayoutBuilder = nullptr;
	SelectedMirrorComponent = nullptr;
}

TSharedRef<IDetailCustomization> FMirrorComponentDetails::MakeInstance()
{
	return MakeShareable(new FMirrorComponentDetails());
}

UMirrorComponent* FMirrorComponentDetails::FindMirrorComponent(IDetailLayoutBuilder& DetailBuilder)
{
	TArray <TWeakObjectPtr<UObject>> SelectedObjects = DetailBuilder.GetDetailsView().GetSelectedObjects();

	for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
	{
		UMirrorComponent* MirrorComp = Cast<UMirrorComponent>(Object.Get());
		if (MirrorComp != nullptr && !MirrorComp->IsTemplate())
		{
			return MirrorComp;
		}
	}

	TArray<TWeakObjectPtr<AActor>> SelectedActors = DetailBuilder.GetDetailsView().GetSelectedActors();
	for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
	{
		AActor* TempActor = Cast<AActor>(Object.Get());
		if (TempActor != nullptr && !TempActor->IsTemplate())
		{
			UMirrorComponent* MirrorComp = TempActor->FindComponentByClass<UMirrorComponent>();
			if (MirrorComp != nullptr && !MirrorComp->IsTemplate())
			{
				return MirrorComp;
			}
		}
	}

	return nullptr;
}

void FMirrorComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	//LastDetailLayoutBuilder = &DetailBuilder;
	
	SelectedMirrorComponent = FindMirrorComponent(DetailBuilder);
	if (SelectedMirrorComponent == nullptr) return;

	IDetailCategoryBuilder& MirrorCategory = DetailBuilder.EditCategory("Mirror", FText::GetEmpty(), ECategoryPriority::Uncommon);

	const bool bHasMirrorActor = SelectedMirrorComponent->MirrorActor != nullptr;
	const bool bCanCreateMirrorActor = SelectedMirrorComponent->CanCreateMirror();
	TSharedPtr< SHorizontalBox > HorizontalBox;

	MirrorCategory.AddCustomRow(FText::GetEmpty(), true)
		[
			SAssignNew(HorizontalBox, SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("CreateMirror_Tooltips", "Create a mirror actor about mirror center."))
				.IsEnabled(bCanCreateMirrorActor)
				.OnClicked(this, &FMirrorComponentDetails::OnCreateMirrorClicked)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(bHasMirrorActor?LOCTEXT("RecreateMirror", "Recreate Mirror") : LOCTEXT("CreateMirror", "Create Mirror"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		];

	HorizontalBox->AddSlot()
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("DeleteMirror_Tooltips", "Delete the current exitsted mirror actor."))
			.IsEnabled(bHasMirrorActor)
			.OnClicked(this, &FMirrorComponentDetails::OnDeleteMirrorClicked)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DeleteMirror", "Delete Mirror"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];
}

FReply FMirrorComponentDetails::OnCreateMirrorClicked()
{
	/*
	* Hack code:
	* Simulate editor CRTL+W.
	* Note: 
	* When component being selected, the CRTL+W will falloff to duplicate component command.
	* To solved this problem, manual deselected all selected actors, and reselected again but actor only.
	*/
	if (SelectedMirrorComponent != nullptr)
	{
		UWorld* World = SelectedMirrorComponent->GetWorld();
		if (World)
		{
			/* Recreate command request. */
			if (SelectedMirrorComponent->MirrorActor)
			{
				SelectedMirrorComponent->MirrorActor->Destroy(false, true);
			}

			AActor* Owner = SelectedMirrorComponent->GetOwner();

			/* Deselected current selected component to avoid duplicate command duplicate components only. */
			GEditor->GetSelectedActors()->DeselectAll();

			/* Simulate current component's owner actor being selected. */
			GEditor->SelectActor(Owner, true, false, false, false);

			/* Execute duplicate command with selected actor. */
			GEditor->Exec(World, TEXT("ACTOR DUPLICATE"));

			/* Actually only duplicate actor being selected currently. */
			for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
			{
				AActor* Actor = static_cast<AActor*>(*It);
				checkSlow(Actor->IsA(AActor::StaticClass()));

				FVector MirrorLocation;
				FRotator MirrorRotation;

				if (SelectedMirrorComponent->GetMirrorLocationAndRotation(MirrorLocation, MirrorRotation))
				{
					Actor->Modify();
					if (SelectedMirrorComponent->MirrorSpace == EMirrorSpace::CenterSpace)
					{
						Actor->SetActorLocationAndRotation(MirrorLocation, MirrorRotation, false, nullptr, ETeleportType::TeleportPhysics);
						Actor->PostEditMove(true);
					}

					SelectedMirrorComponent->Modify();
					SelectedMirrorComponent->MirrorActor = Actor;

					/*
					* FixedMe:
					* Cause of duplicate mirror actor by CRTL+W, and issue deselected operation with current component, the layout builder was changed.
					* Can not send PostEditChangeProperty with mirror component.
					*/
					//LastDetailLayoutBuilder->ForceRefreshDetails();
					//TSharedRef<IPropertyHandle> MirrorProperty = LastDetailLayoutBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(UMirrorComponent, MirrorActor));
					//FPropertyChangedEvent ChangedEvt = MirrorProperty->GetProperty();
					//SelectedMirrorComponent->PostEditChangeProperty(ChangedEvt);

					UMirrorComponent* MirrorComponent = Actor->FindComponentByClass<UMirrorComponent>();
					SelectedMirrorComponent->CopyMirrorSetting(MirrorComponent); // manual setting.
					SelectedMirrorComponent->PostEditChange();
					MirrorComponent->PostEditChange();
				}
			}
		}
	}

	return FReply::Handled();
}

FReply FMirrorComponentDetails::OnDeleteMirrorClicked()
{
	if (SelectedMirrorComponent != nullptr && 
		SelectedMirrorComponent->MirrorActor)
	{
		SelectedMirrorComponent->Modify();
		SelectedMirrorComponent->MirrorActor->Destroy(false, true);
		SelectedMirrorComponent->MirrorActor = nullptr;
		SelectedMirrorComponent->PostEditChange();
	}

	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE