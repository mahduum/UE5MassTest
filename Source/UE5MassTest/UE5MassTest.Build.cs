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
			"MassNavigation", "MassMovement", "MassRepresentation", "NavigationSystem", "Navmesh"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "MassSpawner", "Core"});

		CppStandard = CppStandardVersion.Cpp20;

		PublicIncludePaths.AddRange(new string[]{"UE5MassTest/MassNavigationExtensions/MassNavigationPath/Public"});
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
