// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AnimationSystemEditorTarget : TargetRules
{
	public AnimationSystemEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
        			
		ExtraModuleNames.AddRange( new string[] { "AnimationSystem", "ALSV4_CPP" } );
	}
}
