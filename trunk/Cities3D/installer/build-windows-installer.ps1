# Build Install_Cities3D_v0.10.0.exe from the Release tree in distribute/.
# Requires NSIS 3 and a finished "cmake --build --preset release".
$ErrorActionPreference = "Stop"

$installerDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $installerDir "..\..\..")
$dist = Join-Path $repoRoot "trunk\Cities3D\distribute"
$build = Join-Path $installerDir "build"
$nsi = Join-Path $installerDir "Cities3D.nsi"
$version = "0.10.0"
$setupName = "Install_Cities3D_v$version.exe"

$makensis = @(
    "${env:ProgramFiles(x86)}\NSIS\makensis.exe",
    "$env:ProgramFiles\NSIS\makensis.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $makensis) {
    throw "NSIS 3 is not installed (makensis.exe not found)."
}
if (-not (Test-Path (Join-Path $dist "Cities3D.exe"))) {
    throw "Missing $dist\Cities3D.exe. Build the Release preset first."
}

New-Item -ItemType Directory -Force -Path $build | Out-Null

$gpl = Join-Path $repoRoot "trunk\wxWidgets\2.8.10\docs\gpl.txt"
$licenseText = [IO.File]::ReadAllText($gpl)
$licenseText = $licenseText -replace "`r`n", "`n" -replace "`n", "`r`n"
[IO.File]::WriteAllText((Join-Path $build "license.txt"), $licenseText)

$readme = @"
Cities3D $version

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

The license is in COPYING, next to this file.

Source code for this version (tag v$version):
https://github.com/skyblaster/Cities3D

64-bit Windows 10 or later is required.
"@
$readme = $readme -replace "`r`n", "`n" -replace "`n", "`r`n"
[IO.File]::WriteAllText((Join-Path $build "README.txt"), $readme)

# BSD, MIT, and libtiff require these notices beside a binary distribution.
$share = Join-Path $repoRoot "trunk\build\release\vcpkg_installed\x64-windows\share"
$noticePackages = @(
    "wxwidgets", "glew", "zlib", "libpng", "libjpeg-turbo", "tiff",
    "libwebp", "expat", "pcre2", "liblzma", "miniaudio", "nanosvg",
    "boost-headers"
)
$notices = New-Object System.Text.StringBuilder
[void]$notices.AppendLine("Cities3D $version third-party notices")
[void]$notices.AppendLine("")
[void]$notices.AppendLine("Cities3D is free software under the GNU General Public License, version 2 or later. The license is in COPYING. Source for this version (tag v$version):")
[void]$notices.AppendLine("https://github.com/skyblaster/Cities3D")
[void]$notices.AppendLine("")
[void]$notices.AppendLine("json_spirit, compiled into the program, is copyright 2007-2009 John W. Wilkinson and may be used for any purpose as long as that notice stays in the source.")
[void]$notices.AppendLine("")
[void]$notices.AppendLine("This installer also contains the Microsoft Visual C++ runtime (msvcp140.dll, vcruntime140.dll, and vcruntime140_1.dll). Those files are not open source. Microsoft permits redistributing them with an application under the Visual Studio license. They are not covered by the GPL.")
[void]$notices.AppendLine("")
[void]$notices.AppendLine("The setup program is Nullsoft Scriptable Install System. NSIS is open source under the zlib/libpng license. Its LZMA compression module is under the Common Public License 1.0, and the module's authors permit linking it into an installer without placing the rest of the program under that license. NSIS source: https://nsis.sourceforge.io/Special:Browse/:source")
[void]$notices.AppendLine("")
[void]$notices.AppendLine("Notices for the open-source libraries linked into this build follow.")
foreach ($pkg in $noticePackages) {
    $copyright = Join-Path $share "$pkg\copyright"
    if (-not (Test-Path $copyright)) {
        throw "Missing license text for $pkg ($copyright)"
    }
    [void]$notices.AppendLine("")
    [void]$notices.AppendLine(("=" * 72))
    [void]$notices.AppendLine($pkg)
    [void]$notices.AppendLine(("=" * 72))
    [void]$notices.AppendLine([IO.File]::ReadAllText($copyright).TrimEnd())
    [void]$notices.AppendLine("")
}
$noticeText = $notices.ToString() -replace "`r`n", "`n" -replace "`n", "`r`n"
$noticePath = Join-Path $build "THIRD-PARTY-LICENSES.txt"
[IO.File]::WriteAllText($noticePath, $noticeText)

function Invoke-Makensis {
    param([string[]]$Defines, [string]$Label)
    Write-Host "Compiling $Label"
    & $makensis @Defines $nsi
    if ($LASTEXITCODE -ne 0) {
        throw "makensis failed for $Label (exit $LASTEXITCODE)"
    }
}

$testExe = Join-Path $build "Install_Cities3D_v${version}_usertest.exe"
$testDir = Join-Path $env:TEMP "Cities3DSetupTest"
$staleUninstall = Join-Path $testDir "uninstall.exe"
if (Test-Path $staleUninstall) {
    cmd /c "`"$staleUninstall`" /S _?=$testDir"
    Start-Sleep -Seconds 1
}
if (Test-Path $testDir) {
    Remove-Item -Recurse -Force $testDir
}

Invoke-Makensis -Label "install check" -Defines @(
    "/DUSERLEVEL",
    "/DFASTTEST",
    "/DOUTFILE=$testExe",
    "/DLICENSE=$(Join-Path $build 'license.txt')",
    "/DREADME=$(Join-Path $build 'README.txt')",
    "/DNOTICES=$(Join-Path $build 'THIRD-PARTY-LICENSES.txt')"
)

Write-Host "Installing to $testDir"
cmd /c "`"$testExe`" /S /D=$testDir"
if ($LASTEXITCODE -ne 0) {
    throw "Silent test install failed (exit $LASTEXITCODE)"
}

$required = @(
    "Cities3D.exe",
    "GLCore.dll",
    "UICore.dll",
    "RulesCore.dll",
    "DatabaseCore.dll",
    "ModelCore.dll",
    "glew32.dll",
    "msvcp140.dll",
    "vcruntime140.dll",
    "vcruntime140_1.dll",
    "wxmsw333u_core_vc_x64_custom.dll",
    "wxbase333u_vc_x64_custom.dll",
    "COPYING",
    "README.txt",
    "THIRD-PARTY-LICENSES.txt",
    "plugins\CitiesRules.dll",
    "plugins\CoreRules.dll",
    "plugins\NetworkRules.dll",
    "plugins\SeafarersRules.dll",
    "plugins\StandardRules.dll",
    "plugins\VolcanoRules.dll",
    "data\maps.db",
    "data\mapkey.db",
    "options\Standard_Basic_Game.xml",
    "tex\Standard\gltilestandard.tex",
    "tex\Standard_Small\gltilestandard.tex",
    "tex\Impressionist\gltilestandard.tex",
    "sounds\Standard\EndTurn.mp3",
    "skins\Standard\Cities3D.xml",
    "skins\Standard\gtkrc",
    "skins\Brushed_Metal\Cities3D.xml",
    "locale\en_US\Cities3D.mo"
)
foreach ($rel in $required) {
    $path = Join-Path $testDir $rel
    if (-not (Test-Path $path)) {
        throw "Test install is missing $rel"
    }
}
$noticeBody = [IO.File]::ReadAllText((Join-Path $testDir "THIRD-PARTY-LICENSES.txt"))
foreach ($phrase in @("wxWindows Library Licence", "PCRE2 Licence", "libjpeg-turbo", "Microsoft Visual C++ runtime")) {
    if ($noticeBody -notlike "*$phrase*") {
        throw "Third-party notices are missing '$phrase'"
    }
}

$banned = Get-ChildItem $testDir -Recurse -File | Where-Object {
    $_.Extension -eq ".pdb" -or
    $_.Extension -eq ".po" -or
    $_.Name -like "CitiesOnline.exe" -or
    $_.Name -like "*Online*.dll" -or
    $_.Name -like "wx*d_*.dll" -or
    $_.Name -eq "fmod.dll"
}
if ($banned) {
    $names = ($banned | ForEach-Object { $_.FullName.Substring($testDir.Length + 1) }) -join ", "
    throw "Test install contains files that must not ship: $names"
}

$pluginExtra = Get-ChildItem (Join-Path $testDir "plugins") -File |
    Where-Object { $_.Name -notlike "*Rules.dll" }
if ($pluginExtra) {
    throw "plugins\ contains non-rules files: $($pluginExtra.Name -join ', ')"
}

$dumpbin = Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC" -Recurse -Filter dumpbin.exe |
    Where-Object { $_.FullName -match "Hostx64\\x64\\dumpbin.exe" } |
    Select-Object -First 1 -ExpandProperty FullName
if ($dumpbin) {
    $roots = @((Join-Path $testDir "Cities3D.exe")) + (Get-ChildItem (Join-Path $testDir "plugins\*.dll")).FullName
    $seen = @{}
    $queue = New-Object System.Collections.Generic.Queue[string]
    foreach ($root in $roots) { $queue.Enqueue($root); $seen[$root] = $true }
    $missing = New-Object System.Collections.Generic.List[string]
    while ($queue.Count -gt 0) {
        $path = $queue.Dequeue()
        $out = & $dumpbin /nologo /dependents $path
        foreach ($line in $out) {
            if ($line -match '^\s+([A-Za-z0-9_.+-]+\.dll)\s*$') {
                $name = $Matches[1]
                $local = Join-Path $testDir $name
                if (Test-Path $local) {
                    if (-not $seen.ContainsKey($local)) {
                        $seen[$local] = $true
                        $queue.Enqueue($local)
                    }
                } elseif ($name -notlike "api-ms-win-*" -and $name -notlike "ext-ms-win-*") {
                    $system = Join-Path "$env:SystemRoot\System32" $name
                    if (-not (Test-Path $system) -and -not $missing.Contains($name)) {
                        $missing.Add($name)
                    }
                }
            }
        }
    }
    if ($missing.Count -gt 0) {
        throw "Installed binaries import DLLs that are not installed or in System32: $($missing -join ', ')"
    }
}

Write-Host "Uninstalling test copy"
cmd /c "`"$testDir\uninstall.exe`" /S _?=$testDir"
if ($LASTEXITCODE -ne 0) {
    throw "Silent test uninstall failed (exit $LASTEXITCODE)"
}
Start-Sleep -Seconds 1
if (Test-Path (Join-Path $testDir "Cities3D.exe")) {
    throw "Test uninstall left Cities3D.exe in place"
}
if (Test-Path $testDir) {
    Remove-Item -Recurse -Force $testDir -ErrorAction SilentlyContinue
}
Remove-Item -Force $testExe

$setup = Join-Path $build $setupName
Invoke-Makensis -Label "release installer" -Defines @(
    "/DOUTFILE=$setup",
    "/DLICENSE=$(Join-Path $build 'license.txt')",
    "/DREADME=$(Join-Path $build 'README.txt')",
    "/DNOTICES=$(Join-Path $build 'THIRD-PARTY-LICENSES.txt')"
)

# The setup is already solid-LZMA compressed, so a zip does not get smaller.
$downloads = Join-Path $env:USERPROFILE "Downloads"
$published = Join-Path $downloads $setupName
Copy-Item $setup $published -Force
foreach ($stale in @(
    (Join-Path $downloads "Install_Cities3D_v$version.zip"),
    (Join-Path $downloads "Install_Cities3D_v$version")
)) {
    if (Test-Path $stale) { Remove-Item -Recurse -Force $stale }
}

$setupItem = Get-Item $published
"Setup {0:N2} MB  {1}" -f ($setupItem.Length / 1MB), $setupItem.FullName
