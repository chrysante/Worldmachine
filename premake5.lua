include "Utility/premakeCommon.lua"

-----------------------------------------------------------------------------------------
-- Workspace Worldmachine
-----------------------------------------------------------------------------------------
basicWorkspace "Worldmachine"
filter { "configurations:Debug" }
    defines { "WM_DEBUGLEVEL=2", "WM_LOGLEVEL=2" }
filter { "configurations:Development" }
    defines { "WM_DEBUGLEVEL=1", "WM_LOGLEVEL=1" }
filter { "configurations:Release"  }
    defines { "WM_DEBUGLEVEL=0", "WM_LOGLEVEL=0" }
filter {}

includedirs {
    "worldmachine"
}

sysincludedirs {
    "Utility", "vendor"
}

filter "system:macosx"
    defines { "WM_PLATFORM_MACOS" }
    sysincludedirs "vendor/Metal-cpp"
    xcodebuildsettings { 
        ["MTL_HEADER_SEARCH_PATHS"] = "../../Worldmachine ../../Utility"
    }
filter "system:windows"
    defines { "WM_PLATFORM_WINDOWS" }
filter {}

targetdir("Build/Bin/%{cfg.longname}")
objdir("Build/Obj/%{cfg.longname}")

-----------------------------------------------------------------------------------------
-- Project Worldmachine
-----------------------------------------------------------------------------------------
project "Worldmachine"
location "Worldmachine/App"
kind "WindowedApp"
language "C++"

dependson "WMBuiltinNodes"

files { 
    "Worldmachine/App/**.hpp",
    "Worldmachine/App/**.cpp",
    "Worldmachine/App/Resource/**.ttf",
    "Worldmachine/App/Resource/**.png",
    "Worldmachine/App/Resource/**.json"
}
filter "system:macosx"
    files { 
        "Worldmachine/App/**.metal",
        "Worldmachine/App/Platform/macOS/MainMenu.storyboard",
        "Worldmachine/App/Platform/macOS/Worldmachine.entitlements",
        "Worldmachine/App/Platform/macOS/info.plist"
    }
    xcodebuildsettings {
        ["INFOPLIST_FILE"] = "$(SRCROOT)/Platform/macOS/info.plist"
    }
filter { "system:macosx", "configurations:Debug or Development" }
    xcodebuildsettings {
        ["ONLY_ACTIVE_ARCH"] = "YES"
    }
filter {}

-- Copy Resources
filter "system:macosx"
    postbuildcommands {
        "{COPY} %{prj.location}/Resource/** %{cfg.targetdir}/Worldmachine.app/Contents/Resources",
        --"{COPY} %{prj.location}/../Framework/Platform/MacOS/MainMenu.storyboard %{cfg.targetdir}/Worldmachine.app/Contents/Resources/MainMenu.storyboard",
        "{COPY} %{cfg.targetdir}/libWMBuiltinNodes.dylib %{cfg.targetdir}/Worldmachine.app/Contents/MacOS",
    }
filter {}

xcodebuildresources {
    "Worldmachine/App/Resource/**.ttf",
    "Worldmachine/App/Resource/**.png",
    "Worldmachine/App/Resource/**.json"
}

links { 
    "WMFramework",
    "WMCore",
    "Utility",
    "ImGui",
    "YAML"
}

filter "system:macosx"
    links {
        "AppKit.framework",
        "Metal.framework",
        "MetalKit.framework",
        "GameController.framework",
        "CoreImage.framework",
        "UniformTypeIdentifiers.framework"
    }
filter {}

-----------------------------------------------------------------------------------------
-- Project WMCore
-----------------------------------------------------------------------------------------
project "WMCore"
location "Worldmachine/Core"
kind "StaticLib"
language "C++"

files { 
    "Worldmachine/Core/**.hpp",
    "Worldmachine/Core/**.cpp"
}


-----------------------------------------------------------------------------------------
-- Project WMCoreUT
-----------------------------------------------------------------------------------------
filter "options:tests"

    project "WMCoreUnitTests"
    location "UnitTests/Core"
    kind "ConsoleApp"
    language "C++"

    files { 
        "UnitTests/Core/main.cpp",
        "UnitTests/Core/**.t.hpp",
        "UnitTests/Core/**.t.cpp"
    }

    links { "WMCore", "WMFramework", "Utility", "YAML", "ImGui" }
    filter "system:macosx"
    links {
        "AppKit.framework",
        "Metal.framework",
        "MetalKit.framework",
        "GameController.framework",
        "CoreImage.framework",
        "UniformTypeIdentifiers.framework"
    }
    filter {}

filter {}

-----------------------------------------------------------------------------------------
-- Project WMFramework
-----------------------------------------------------------------------------------------
project "WMFramework"
location "Worldmachine/Framework"
kind "StaticLib"
language "C++"

filter "system:macosx" 
    xcodebuildsettings { 
        ["CLANG_ENABLE_OBJC_ARC"] = "YES"
    }
filter {}

-- 
includedirs {
    "Vendor/imgui"
}

files { 
    "Worldmachine/Framework/**.hpp",
    "Worldmachine/Framework/**.cpp"
}

removefiles "Worldmachine/Framework/Platform/**"

filter "system:macosx"
    files "Worldmachine/Framework/Platform/MacOS/**"
filter "system:windows"
    files "Worldmachine/Framework/Platform/Windows/**"
filter {}

filter "system:macosx"
    files {
        "Worldmachine/Framework/**.mm" 
    }
filter {}

-----------------------------------------------------------------------------------------
-- Project WMBuiltinNodes
-----------------------------------------------------------------------------------------
project "WMBuiltinNodes"
location "Worldmachine/BuiltinNodes"
kind "SharedLib"
language "C++"

links { "WMCore", "Utility", "ImGui", "YAML" }

files { 
    "Worldmachine/BuiltinNodes/**.hpp",
    "Worldmachine/BuiltinNodes/**.cpp"
}

postbuildcommands {
    "{COPY} %{cfg.targetdir}/libWMBuiltinNodes.dylib %{cfg.targetdir}/Worldmachine.app/Contents/MacOS",
}

-----------------------------------------------------------------------------------------
-- Project WMPlayground
-----------------------------------------------------------------------------------------
project "WMPlayground"
location "Playground"
kind "ConsoleApp"
language "C++"

files { 
    "Playground/**.hpp",
    "Playground/**.cpp"
}

links { 
    "WMCore",
    "Utility",
    "ImGui",
    "YAML"
}

-----------------------------------------------------------------------------------------
-- Project Utility
-----------------------------------------------------------------------------------------
include "Utility/utility_project.lua"

-----------------------------------------------------------------------------------------
-- Project ImGui
-----------------------------------------------------------------------------------------
project "ImGui"
location "Vendor/imgui"
kind "StaticLib"
language "C++"
filter "system:macosx" 
    xcodebuildsettings { 
        ["CLANG_ENABLE_OBJC_ARC"] = "YES"
    }
filter {}

includedirs {
    "Vendor/imgui"
}

files { 
    "Vendor/imgui/imgui.cpp",
    "Vendor/imgui/imgui_demo.cpp",
    "Vendor/imgui/imgui_draw.cpp",
    "Vendor/imgui/imgui_tables.cpp",
    "Vendor/imgui/imgui_widgets.cpp",
    "Vendor/imgui/imgui.cpp",
}

filter "system:macosx" 
    files {
        "Vendor/imgui/backends/imgui_impl_osx.mm",
        "Vendor/imgui/backends/imgui_impl_metal.mm"
    }
filter {}

-----------------------------------------------------------------------------------------
-- Project YAML
-----------------------------------------------------------------------------------------
project "YAML"
location "Vendor/yaml-cpp"
kind "StaticLib"
language "C++"
filter "system:macosx" 
    xcodebuildsettings { 

    }
filter {}
    

files {
    "Vendor/yaml-cpp/src/**.cpp"
}

-- to disable warnings from xcode to update project settings
project "*"
filter "system:macosx" 
    xcodebuildsettings { 
        ["GCC_ENABLE_FIX_AND_CONTINUE"] = nil,
        ["CLANG_ENABLE_OBJC_WEAK"] = "YES",
        ["CODE_SIGN_IDENTITY"] = "-",
        ["CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING"] = "YES",
        ["CLANG_WARN_BOOL_CONVERSION"] = "YES",
        ["CLANG_WARN_CONSTANT_CONVERSION"] = "YES",
        ["CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS"] = "YES",
        ["CLANG_WARN_EMPTY_BODY"] = "YES",
        ["CLANG_WARN_ENUM_CONVERSION"] = "YES",
        ["CLANG_WARN_INFINITE_RECURSION"] = "YES",
        ["CLANG_WARN_INT_CONVERSION"] = "YES",
        ["CLANG_WARN_NON_LITERAL_NULL_CONVERSION"] = "YES",
        ["CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF"] = "YES",
        ["CLANG_WARN_OBJC_LITERAL_CONVERSION"] = "YES",
        ["CLANG_WARN_QUOTED_INCLUDE_IN_FRAMEWORK_HEADER"] = "YES",
        ["CLANG_WARN_RANGE_LOOP_ANALYSIS"] = "YES",
        ["CLANG_WARN_STRICT_PROTOTYPES"] = "YES",
        ["CLANG_WARN_SUSPICIOUS_MOVE"] = "YES",
        ["CLANG_WARN_UNREACHABLE_CODE"] = "YES",
        ["CLANG_WARN__DUPLICATE_METHOD_MATCH"] = "YES",
        ["CLANG_WARN_COMMA"] = "YES",
        ["ENABLE_TESTABILITY"] = "YES",
        ["ENABLE_STRICT_OBJC_MSGSEND"] = "YES",
        ["GCC_NO_COMMON_BLOCKS"] = "YES",
        ["GCC_WARN_UNDECLARED_SELECTOR"] = "YES",
        ["GCC_WARN_UNINITIALIZED_AUTOS"] = "YES",
        ["GCC_WARN_UNUSED_FUNCTION"] = "YES",
        ["GCC_WARN_UNUSED_VARIABLE"] = "YES",
        ["GCC_WARN_64_TO_32_BIT_CONVERSION"] = "YES",
    }
filter {}

