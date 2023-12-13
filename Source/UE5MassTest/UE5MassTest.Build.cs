// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE5MassTest : ModuleRules
{
	public UE5MassTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore",
			"MassCommon", "MassEntity", "MassCrowd", "MassAIBehavior", "MassGameplayDebug", "PluginTest_0206_2001",
			"MassGameplayExternalTraits", "MassSpawner", "StructUtils", "MassEntityDebugger", "MassSignals", "MassActors",
			"MassNavigation", "MassMovement"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "MassSpawner", "Core"});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
