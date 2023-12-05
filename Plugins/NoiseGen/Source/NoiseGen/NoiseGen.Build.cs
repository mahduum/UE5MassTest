// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class NoiseGen : ModuleRules
{
	public NoiseGen(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "CoreUObject",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		// get base folder
		string ModulePath = ModuleDirectory + '/';
		ModulePath = ModulePath.Replace('\\', '/');

		// add sdk include paths
		string IncludeDir = Path.Combine(ModulePath, "sdk", "include");
		PublicIncludePaths.Add(IncludeDir);

		// add sdk src paths (source code license only)
		string SrcDir = Path.Combine(ModulePath, "sdk", "src");
		PrivateIncludePaths.Add(SrcDir);
	}
}
