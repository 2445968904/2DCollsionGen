using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class Box2DEditorPlugin : ModuleRules
    {
        public Box2DEditorPlugin(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivatePCHHeaderFile = "Private/Box2DEditorPluginPCH.h";
            PCHUsage = PCHUsageMode.UseSharedPCHs;
            DefaultBuildSettings = BuildSettingsVersion.V5;

            PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd",
                "Box2DPlugin",
                "RenderCore",
                "RHI"
            });

            PrivateDependencyModuleNames.AddRange(new string[] {
                "Slate",
                "SlateCore",
                "EditorStyle"
            });
        }
    }
}
