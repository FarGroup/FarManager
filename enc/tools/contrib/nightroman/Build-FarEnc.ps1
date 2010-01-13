
<#
.SYNOPSIS
	Builds FarEncyclopedia.xx.chm from enc_eng or from enc_rus.
	Author: Roman Kuzmin (NightRoman)

.DESCRIPTION
	It processes and compiles Far API encyclopedia source enc_eng or enc_rus
	into FarEncyclopedia.en.chm or FarEncyclopedia.ru.chm.

	Notes
	- Assume you have: C:\Program Files\HTML Help Workshop\hhc.exe
	- Invoke one instance at a time, it uses the same temp files
	- Temp directory $env:TEMP\far_enc still exists on failures

	Submitted: enc\tools\contrib\nightroman\Build-FarEnc.ps1

.EXAMPLE
	# The current directory is enc_eng or enc_rus and CHM will be here
	Build-FarEnc

	# Build both .en.chm and .ru.chm in batch mode
	Build-FarEnc c:\dev\enc\enc_eng c:\distr\enc -NoInteraction
	Build-FarEnc c:\dev\enc\enc_rus c:\distr\enc -NoInteraction

.PARAMETER InputPath
		Directory where input items: images, meta, styles, plugins?.hhc,
		plugins?.hhp are located. Normally it is enc_eng or enc_rus. Default:
		the current directory.

.PARAMETER OutputPath
		Directory for the output pluginse.chm or pluginsr.chm. If it does not
		exist then it is created. Default: the current directory.

.PARAMETER NoInteraction
		Do not invoke the result CHM.
#>

param
(
	[string]$InputPath = '.',
	[string]$OutputPath = '.',
	[switch]$NoInteraction
)

Add-Type -AssemblyName System.Web
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

# Ensure HTML Help Workshop
Set-Alias hhc (Get-Command "C:\Program Files\HTML Help Workshop\hhc.exe").Definition

### Some paths
$Tmp = "$env:TEMP\far_enc"
$InputPath = [IO.Path]::GetFullPath($InputPath)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (Test-Path "$InputPath\pluginse.hhp") {
	$Chm = 'FarEncyclopedia.en.chm'
	$Hhp = 'pluginse.hhp'
	$Hhk = 'pluginse.hhk'
	$Out = 'distr_chm_pluginse'
}
elseif (Test-Path "$InputPath\pluginsr.hhp") {
	$Chm = 'FarEncyclopedia.ru.chm'
	$Hhp = 'pluginsr.hhp'
	$Hhk = 'pluginsr.hhk'
	$Out = 'distr_chm_pluginsr'
}
else {
	throw "Cannot find 'pluginse.hhp' or 'pluginsr.hhp' in '$InputPath'."
}

### Setup
Write-Host "Starting..."
if (!(Test-Path $OutputPath)) { $null = mkdir $OutputPath }
if (Test-Path "$OutputPath\$Chm") { Remove-Item "$OutputPath\$Chm" }
if (Test-Path $Tmp) { Remove-Item $Tmp -Recurse -Force }
$null = mkdir "$Tmp\$Out"
Write-Host "Copying data..."
Copy-Item "$InputPath\*" $Tmp -Recurse
Move-Item "$Tmp\meta" "$Tmp\html"
Push-Location $Tmp

### Index
Write-Host "Making $Hhk..."
$nbIndex = 0
Set-Content $Hhk $(
	@'
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<BODY>
<UL>
'@
	# index <h1> and <a name=..>
	$map = New-Object Collections.Hashtable
	foreach($file in Get-ChildItem html -Include *.html -Recurse -Name) {
		$h1 = $null
		$path = "html\$file"
		foreach($line in Get-Content $path) {
			$name = $null
			$href = $null
			$page = $null
			# index all <h1>, assume HTML tags are not used inside
			if ($line -match '<h1>(.+?)</h1>') {
				$h1 = [System.Web.HttpUtility]::HtmlDecode($matches[1]).Replace('"', '')
				$name = $h1
				$href = $path
			}
			# index <a> without HTML tags inside
			elseif ($line -match '<a\s+.*?name\s*=\s*["''](.+?)["''].*?>([^<>]+)</a>') {
				$name = [System.Web.HttpUtility]::HtmlDecode($matches[2]).Replace('"', '')
				$href = $path + '#' + $matches[1]
				$page = $h1
			}
			if ($name) {
				++$nbIndex
				$item = 1 | Select-Object href, page
				$item.href = $href
				$item.page = $page
				if ($map.ContainsKey($name)) {
					$map[$name] += $item
				}
				else {
					$map.Add($name, @($item))
				}
			}
		}
	}
	foreach($name in $map.Keys | Sort-Object) {
		$items = $map[$name]
		foreach($item in $items | Sort-Object page) {
			@"
<LI><OBJECT type="text/sitemap">
<param name="Name" value="$(if ($item.page -and $items.Count -ge 2) { $name + ' - ' + $item.page } else { $name })">
<param name="Local" value="$($item.href)">
</OBJECT>
"@
		}
	}
	@'
</UL>
</BODY>
</HTML>
'@
)

### CHM
Write-Host "Building CHM..."
hhc "$Tmp\$Hhp"
Copy-Item "$Out\$Chm" $OutputPath
if (!$NoInteraction) { Invoke-Item "$OutputPath\$Chm" }

### OK
Write-Host "Finishing..."
Pop-Location
Remove-Item $Tmp -Recurse -Force
Write-Host @"
Indices: $nbIndex
OK
"@
