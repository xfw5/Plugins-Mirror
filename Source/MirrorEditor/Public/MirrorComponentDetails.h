// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IDetailCustomization.h"

class UMirrorComponent;
class FReply;

/**
 * 
 */
class MIRROREDITOR_API FMirrorComponentDetails: public IDetailCustomization
{
public:
	FMirrorComponentDetails();

	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
	FReply OnCreateMirrorClicked();
	FReply OnDeleteMirrorClicked();

protected:
	UMirrorComponent* FindMirrorComponent(IDetailLayoutBuilder& DetailBuilder);

	UMirrorComponent* SelectedMirrorComponent;
	IDetailLayoutBuilder* LastDetailLayoutBuilder;
};
