// Copyright Epic Games, Inc. All Rights Reserved.

#include "ETcpPlugin.h"
#include "ETcpConnection.h"

#define LOCTEXT_NAMESPACE "FETcpPluginModule"

void FETcpPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//auto Msg = FString(TEXT("dddd"));
	//UE_LOG(LogTemp, Log, TEXT(__FILE__":%d " "DDD %s"), __LINE__, *Msg);
	FETcpWorker::TestAll();
}

void FETcpPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FETcpPluginModule, ETcpPlugin)