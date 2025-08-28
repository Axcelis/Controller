
# build-controller.ps1
# Check for 'clean' argument
param(
    [Parameter(Position=0)]
    [string]$Action = "build"
)

# Set paths
$VcpkgRoot = "$env:USERPROFILE\vcpkg"
$VcpkgExe = "$VcpkgRoot\vcpkg.exe"
$ProjectDir = "Controller"
$MainSrcFiles = Get-ChildItem "$ProjectDir\*.cpp" | Where-Object { $_.Name -ne "test_controller.cpp" } | ForEach-Object { $_.FullName }
$MainOutputExe = "main.exe"

# Build test_controller separately
$TestDir = "$PSScriptRoot\test_controller"
$TestSrcFiles = Get-ChildItem "$TestDir\*.cpp" | ForEach-Object { $_.FullName }
$ControllerImplFiles = Get-ChildItem "$ProjectDir\*.cpp" | Where-Object { $_.Name -notin @("main.cpp", "test_controller.cpp") } | ForEach-Object { $_.FullName }
$TestAllSrcFiles = $TestSrcFiles + $ControllerImplFiles
$TestOutputExe = "test_controller.exe"
$Arch = "x64-windows"
$StaticArch = "x64-windows-static"



if ($Action -eq "clean") {
    Write-Host "Cleaning build artifacts..."
    Remove-Item -Force -ErrorAction SilentlyContinue $OutputExe
    Remove-Item -Force -ErrorAction SilentlyContinue *.obj
    Remove-Item -Force -ErrorAction SilentlyContinue *.exe
    Remove-Item -Force -ErrorAction SilentlyContinue *.dll
    Write-Host "Clean complete."
    exit 0
}

# Check for vcpkg
if (!(Test-Path $VcpkgExe)) {
    Write-Host "vcpkg not found. Cloning and bootstrapping vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git $VcpkgRoot
    & "$VcpkgRoot\bootstrap-vcpkg.bat"
}

# Check for ZeroMQ

# Detect available ZeroMQ library (dynamic or static)
$DynamicLib = Get-ChildItem "$VcpkgRoot\installed\$Arch\lib" -Filter "libzmq-mt-*.lib" | Select-Object -First 1
$StaticLib = Get-ChildItem "$VcpkgRoot\installed\$StaticArch\lib" -Filter "libzmq-mt-s-*.lib" | Select-Object -First 1

if ($DynamicLib) {
    Write-Host "Found dynamic ZeroMQ library: $($DynamicLib.Name)"
    $LibDir = "$VcpkgRoot\installed\$Arch\lib"
    $IncludeDir = "$VcpkgRoot\installed\$Arch\include"
    $ZmqLibName = $DynamicLib.Name
} elseif ($StaticLib) {
    Write-Host "Found static ZeroMQ library: $($StaticLib.Name)"
    $LibDir = "$VcpkgRoot\installed\$StaticArch\lib"
    $IncludeDir = "$VcpkgRoot\installed\$StaticArch\include"
    $ZmqLibName = $StaticLib.Name
} else {
    Write-Host "ZeroMQ library not found. Attempting to install zeromq for $Arch and $StaticArch..."
    & $VcpkgExe install "zeromq:$Arch"
    & $VcpkgExe install "zeromq:$StaticArch"
    $DynamicLib = Get-ChildItem "$VcpkgRoot\installed\$Arch\lib" -Filter "libzmq-mt-*.lib" | Select-Object -First 1
    $StaticLib = Get-ChildItem "$VcpkgRoot\installed\$StaticArch\lib" -Filter "libzmq-mt-s-*.lib" | Select-Object -First 1
    if ($DynamicLib) {
        Write-Host "Found dynamic ZeroMQ library after install: $($DynamicLib.Name)"
        $LibDir = "$VcpkgRoot\installed\$Arch\lib"
        $IncludeDir = "$VcpkgRoot\installed\$Arch\include"
        $ZmqLibName = $DynamicLib.Name
    } elseif ($StaticLib) {
        Write-Host "Found static ZeroMQ library after install: $($StaticLib.Name)"
        $LibDir = "$VcpkgRoot\installed\$StaticArch\lib"
        $IncludeDir = "$VcpkgRoot\installed\$StaticArch\include"
        $ZmqLibName = $StaticLib.Name
    } else {
        Write-Host "ERROR: ZeroMQ library still not found after installation. Please check vcpkg and architecture settings."
        exit 1
    }
}

# Check for cppzmq (C++ bindings for ZeroMQ)
$CppzmqHeader = "$VcpkgRoot\installed\$Arch\include\zmq.hpp"
if (!(Test-Path $CppzmqHeader)) {
    Write-Host "cppzmq (zmq.hpp) not found. Installing via vcpkg..."
    & $VcpkgExe install cppzmq
}

# Find x64 cl.exe and MSVC x64 lib path
$VSPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC"
$MSVCVersion = Get-ChildItem $VSPath | Sort-Object Name -Descending | Select-Object -First 1
$ClPath = "$VSPath\$($MSVCVersion.Name)\bin\Hostx64\x64\cl.exe"
$MSVCLibPath = "$VSPath\$($MSVCVersion.Name)\lib\x64"

# Find Windows SDK x64 lib paths
$WinSdkRoot = "C:\Program Files (x86)\Windows Kits\10"
$WinSdkLibVersion = Get-ChildItem "$WinSdkRoot\lib" | Sort-Object Name -Descending | Select-Object -First 1
$WinSdkUcrtLibPath = "$WinSdkRoot\lib\$($WinSdkLibVersion.Name)\ucrt\x64"
$WinSdkUmLibPath = "$WinSdkRoot\lib\$($WinSdkLibVersion.Name)\um\x64"

if (!(Test-Path $ClPath)) {
    Write-Host "ERROR: x64 cl.exe not found at $ClPath. Please check your Visual Studio installation."
    exit 1
}
if (!(Test-Path $MSVCLibPath)) {
    Write-Host "ERROR: x64 MSVC lib path not found at $MSVCLibPath. Please check your Visual Studio installation."
    exit 1
}
if (!(Test-Path $WinSdkUcrtLibPath)) {
    Write-Host "ERROR: x64 Windows SDK UCRT lib path not found at $WinSdkUcrtLibPath. Please check your Windows SDK installation."
    exit 1
}
if (!(Test-Path $WinSdkUmLibPath)) {
    Write-Host "ERROR: x64 Windows SDK UM lib path not found at $WinSdkUmLibPath. Please check your Windows SDK installation."
    exit 1
}

Write-Host "Using x64 cl.exe: $ClPath"
Write-Host "Compiling main project..."
& $ClPath /EHsc /MD /I"$IncludeDir" $MainSrcFiles /link `
    /LIBPATH:"$LibDir" `
    /LIBPATH:"$MSVCLibPath" `
    /LIBPATH:"$WinSdkUcrtLibPath" `
    /LIBPATH:"$WinSdkUmLibPath" `
    $ZmqLibName /OUT:$MainOutputExe /machine:x64

Write-Host "Compiling test_controller project..."
if ($TestSrcFiles) {
    $TestAllSrcFiles = @($TestSrcFiles) + @($ControllerImplFiles)
    $clArgs = @('/EHsc', '/MD', "/I$IncludeDir") + $TestAllSrcFiles + @(
        '/link',
        "/LIBPATH:$LibDir",
        "/LIBPATH:$MSVCLibPath",
        "/LIBPATH:$WinSdkUcrtLibPath",
        "/LIBPATH:$WinSdkUmLibPath",
        $ZmqLibName,
        "/OUT:$TestOutputExe",
        '/machine:x64'
    )
    Write-Host "cl.exe arguments for test_controller.exe:"
    Write-Host "Type: $($TestAllSrcFiles.GetType().Name), Count: $($TestAllSrcFiles.Count)"
    $clArgs | ForEach-Object { Write-Host $_ }
    & $ClPath @clArgs
    Write-Host "test_controller.exe built."
} else {
    Write-Host "No test_controller.cpp found in $TestDir. Skipping test build."
}

# Check for DLL
$ZmqDll = Get-ChildItem "$VcpkgRoot\installed\$Arch\bin\libzmq*.dll" | Select-Object -First 1
if ($ZmqDll) {
    Copy-Item $ZmqDll.FullName . -Force
    Write-Host "ZeroMQ DLL copied to output directory."
} else {
    Write-Host "ZeroMQ DLL not found. Application may not run."
}

Write-Host "Build complete. Executable: $OutputExe"