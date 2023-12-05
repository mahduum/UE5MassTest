// Copyright Epic Games, Inc. All Rights Reserved.

#include "PluginTest_0206_2001.h"

#define LOCTEXT_NAMESPACE "FPluginTest_0206_2001Module"

void FPluginTest_0206_2001Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FPluginTest_0206_2001Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPluginTest_0206_2001Module, PluginTest_0206_2001)