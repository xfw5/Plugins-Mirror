// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "MirrorEditor.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "MirrorComponentDetails.h"

#define LOCTEXT_NAMESPACE "FMirrorEditorModule"

void FMirrorEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("MirrorComponent", FOnGetDetailCustomizationInstance::CreateStatic(&FMirrorComponentDetails::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FMirrorEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("MirrorComponent");
		PropertyModule.NotifyCustomizationModuleChanged();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMirrorEditorModule, MirrorEditor)