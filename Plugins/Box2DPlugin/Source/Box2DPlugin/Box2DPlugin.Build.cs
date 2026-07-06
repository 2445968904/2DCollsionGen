using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class Box2DPlugin : ModuleRules
    {
        public Box2DPlugin(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivatePCHHeaderFile = "Private/Box2DPluginPCH.h";
            PCHUsage = PCHUsageMode.UseSharedPCHs;
            DefaultBuildSettings = BuildSettingsVersion.V5;

            string ThirdPartyDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty"));
            string Box2DSourceDir = Path.Combine(ThirdPartyDir, "box2d-main");

            // Box2D public API headers only
            // Do NOT add src/ to PublicIncludePaths — Windows is case-insensitive so MSVC would match
            // src/core.h when UE code includes "Core.h", breaking C++ compilation.
            PublicIncludePaths.Add(Path.Combine(Box2DSourceDir, "include"));

            // Do NOT add Box2DSource to PrivateIncludePaths — Windows is case-insensitive
            // so MSVC would match Box2DSource/Core.h when UE code includes "Core.h",
            // breaking C++ compilation. The .c files find their internal headers via
            // same-directory search automatically.

            // Box2D uses C11 features (_Alignas, _Static_assert)
            CStandard = CStandardVersion.C11;

            // Export Box2D symbols from this module's DLL
            PublicDefinitions.Add("box2d_EXPORTS=1");

            // Do NOT enable UnityFileType.C — .c files use C11 syntax (_Alignas)
            // which is invalid in C++ and breaks when unity-built into a .cpp file.
            // C files are compiled individually by UBT.

            IWYUSupport = IWYUSupport.None;

            PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "ProceduralMeshComponent",
                "RenderCore",
                "RHI"
            });

            PrivateDependencyModuleNames.AddRange(new string[] {
                "Slate",
                "SlateCore"
            });
        }
    }
}
