using System.Text.RegularExpressions;

string Separator = new('-', 80);

var AllChangelogs = Directory
	.EnumerateFiles(ChangelogChecker.GeneratedConstants.FarManagerRootDir, "changelog", SearchOption.AllDirectories)
	.Where(changelog => Path.GetFileName(changelog) == "changelog")
	.Select(changelog => Path.GetFullPath(changelog));

Regex TimeIsh = new(@"^\D [^=]+ \d{2}:\d{2}:\d{2}", RegexOptions.Compiled | RegexOptions.IgnoreCase | RegexOptions.IgnorePatternWhitespace);
Regex ChangelogHeader = new(
	@"^\w.+ \s 20\d{2}-\d{2}-\d{2} \s \d{2}:\d{2}:\d{2}[+-]\d{2}:00 .*$",
	RegexOptions.Compiled | RegexOptions.IgnoreCase | RegexOptions.IgnorePatternWhitespace);

var BadChangelogCount = 0;

foreach (var Changelog in AllChangelogs)
{
	Console.WriteLine(Separator);
	Console.WriteLine(Changelog);

	var ErrorCount = CheckOneChangelog(Changelog);
	if (ErrorCount > 0)
	{
		BadChangelogCount++;
		Console.WriteLine($"*** {ErrorCount} errors in {Changelog}");
	}

	Console.WriteLine();
}

return BadChangelogCount;

int CheckOneChangelog(string Changelog)
{
	var ErrorCount = 0;
	bool? LookForSeparators = null;
	bool WasSeparator = false;

	foreach (var Line in File.ReadLines(Changelog))
	{
		if (!LookForSeparators.HasValue)
		{
			LookForSeparators = Line == Separator;
		}

		if (Line == Separator)
		{
			if (!LookForSeparators.Value)
			{
				ErrorCount++;
				Console.WriteLine("--- Unexpected separator");
				continue;
			}

			WasSeparator = true;
			continue;
		}

		var IsChangelogHeader = ChangelogHeader.IsMatch(Line);

		if (WasSeparator)
		{
			WasSeparator = false;

			if (!IsChangelogHeader)
			{
				ErrorCount++;
				Console.WriteLine(Line);
			}

			continue;
		}

		if (LookForSeparators.Value && IsChangelogHeader)
		{
			ErrorCount++;
			Console.WriteLine(Line);
			continue;
		}

		if (TimeIsh.IsMatch(Line) && !IsChangelogHeader)
		{
			ErrorCount++;
			Console.WriteLine(Line);
		}
	}

	return ErrorCount;
}
