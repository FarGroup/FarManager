
<#
.Synopsis
	Tests href and src links in HTML pages.
	Author: Roman Kuzmin (NightRoman)

.Description
	It scans local href and src links in HTML and checks that targets exist.
	For a missed target the script writes text info or an object with
	properties: Path, LineNumber, Link.

.Example
	# Test links in two directories, show text info
	Test-HtmlLink enc_eng\meta, enc_rus\meta

	# Ditto, but emit objects and format them as list
	Test-HtmlLink enc_eng\meta, enc_rus\meta -Emit | Format-List

	# Emitted objects can be used effectively, for example in PowerShellFar we
	# can get bad links, show list of them and for a selected one open the
	# source HTML in editor right at the line with a link
	Test-HtmlLink -Emit | Out-FarList -Text Link | Start-FarEditor

.Parameter Path
		Location(s) of HTML pages. See Get-ChildItem -Path. Default is the
		current location.

.Parameter Include
		See Get-ChildItem -Include. Default: *.htm and *.html.

.Parameter Emit
		Tells to emit objects.
#>

param
(
	$Path = '.',
	$Include = @('*.htm', '*.html'),
	[switch]$Emit
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

Get-ChildItem $Path -Include $Include -Recurse | Select-String '(?:href|src)\s*=\s*["'']([^"''#]+?)["''#]' -AllMatches | .{process{
	$directory = [IO.Path]::GetDirectoryName($_.Path)
	foreach($m in $_.Matches) {
		$link = $m.Groups[1].Value
		if ($link -notmatch '^(?:http:|mailto:|ftp:|news:)' -and ![IO.File]::Exists("$directory\$link")) {
			if ($Emit) {
				$e = 1 | Select-Object Path, LineNumber, Link
				$e.Path = $_.Path; $e.LineNumber = $_.LineNumber; $e.Link = $link
				$e
			}
			else {
				'{0}:{1}:{2}' -f $_.Path, $_.LineNumber, $link
			}
		}
	}
}}
