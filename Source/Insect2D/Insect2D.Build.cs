// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Insect2D : ModuleRules
{
	public Insect2D(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Paper2D" });
	}
}
