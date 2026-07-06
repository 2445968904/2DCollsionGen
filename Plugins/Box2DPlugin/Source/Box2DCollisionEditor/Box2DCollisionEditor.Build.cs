using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class Box2DCollisionEditor : ModuleRules
    {
        public Box2DCollisionEditor(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivatePCHHeaderFile = "Private/Box2DCollisionEditorPCH.h";
            PCHUsage = PCHUsageMode.UseSharedPCHs;
            DefaultBuildSettings = BuildSettingsVersion.V5;

            PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Box2DPlugin",
                "RenderCore",
                "RHI"
            });

            PrivateDependencyModuleNames.AddRange(new string[] {
                "Slate",
                "SlateCore",
                "UnrealEd",
                "AssetTools",
                "ContentBrowser",
                "WorkspaceMenuStructure",
                "PropertyEditor",
                "EditorFramework",
                "Kismet",
                "InputCore",
                "ToolMenus",
                "ToolWidgets",
                "EditorStyle"
            });

            PrivateIncludePathModuleNames.AddRange(new string[] {
                "Settings",
                "LevelEditor"
            });
        }
    }
}
