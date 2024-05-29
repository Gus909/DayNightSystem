// Illia Huskov 2024

using UnrealBuildTool;
using System.Collections.Generic;

public class SandboxTarget : TargetRules
{
	public SandboxTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "Sandbox" } );
	}
}
