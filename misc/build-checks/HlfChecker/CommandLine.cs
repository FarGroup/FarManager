using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace HlfChecker
{
	[Flags]
	public enum ProcessingOptions
	{
		None = 0x0,
		Verbose = 0x1,
		StrictRefs = 0x2,
		ShowHelp = 0x1000
	}

	public static class CommandLine
	{
		public static bool Parse(
			string[] parameters,
			out List<DirectoryInfo> directories,
			out Regex languageFilter,
			out ProcessingOptions options)
		{
			var paramList = parameters?.ToList() ?? new List<string>();

			options = GetProcessingOptions(paramList);

			directories = default;

			languageFilter = GetLanguageFilter(paramList);
			if (languageFilter == default) return false;

			directories = GetDirectories(paramList);
			return true;
		}

		private static List<string> OptionNames =
			Enum.GetNames(typeof(ProcessingOptions)).Where(n => n != ProcessingOptions.None.ToString()).ToList();

		// Filters paramList
		private static ProcessingOptions GetProcessingOptions(List<string> paramList)
		{
			ProcessingOptions options = default;
			var outParamList = new List<string>(paramList.Count);

			foreach (var parameter in paramList)
			{
				var optionName = OptionNames.Find(opt => opt.Equals(parameter, StringComparison.OrdinalIgnoreCase));

				if (optionName != default)
				{
					options |= (ProcessingOptions)Enum.Parse(typeof(ProcessingOptions), optionName);
				}
				else
				{
					outParamList.Add(parameter);
				}
			}

			paramList.Clear();
			paramList.AddRange(outParamList);

			$"Using processing options: ({options}).".Trace();

			return options;
		}

		// Filters paramList
		private static Regex GetLanguageFilter(List<string> paramList)
		{
			var requested = paramList.Where(param => param?.StartsWith("+") == true).ToList();
			var excluded = paramList.Where(param => param?.StartsWith("-") == true).ToList();

			if (requested.Count == 0 && excluded.Count == 0)
			{
				return new Regex(@"\p{L}{3}\.hlf", RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase);
			}

			if (requested.Count > 0 && excluded.Count > 0)
			{
				$"Cannot apply requested ({string.Join(", ", requested)}) and excluded ({string.Join(", ", excluded)}) language filters together. Exiting.".Trace();
				return default;
			}

			return GetLanguageFilter(paramList, requested.Count > 0 ? (requested, @"({0})\.hlf") : (excluded, @"(?!{0})\p{{L}}{{3}}\.hlf"));
		}

		private static Regex GetLanguageFilter(List<string> paramList, (List<string> filters, string regexFormat) requestedOrExcluded)
		{
			if (!ValidateLanguageFilters(requestedOrExcluded.filters))
			{
				return default;
			}

			ApplyLanguageFilters(paramList, requestedOrExcluded.filters);

			return new Regex(string.Format(requestedOrExcluded.regexFormat, string.Join("|", requestedOrExcluded.filters.Select(f => f.Substring(1)))),
				RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase);
		}

		private static readonly Regex ValidLanguageFilter = new Regex(@"^[+-]\p{L}{3}$", RegexOptions.Compiled | RegexOptions.CultureInvariant);

		private static bool ValidateLanguageFilters(List<string> filters)
		{
			var invalid = filters.Where(lng => !ValidLanguageFilter.IsMatch(lng)).ToList();
			if (invalid.Count == 0)
			{
				return true;
			}

			$"Invalid language filter(s) specified: ({string.Join(", ", invalid)}). Exiting.".Trace();
			return false;
		}

		private static void ApplyLanguageFilters(List<string> paramList, List<string> filters)
		{
			$"Language filters applied: ({string.Join(", ", filters)}).".Trace();

			var outParamList = paramList.Except(filters).ToList();
			paramList.Clear();
			paramList.AddRange(outParamList);
		}

		private static List<DirectoryInfo> GetDirectories(List<string> paramList)
		{
			return paramList.Count > 0
				? paramList.Select(directory => new DirectoryInfo(directory)).ToList()
				: GlobalConstants.PluginsDirectory.EnumerateDirectories()
					.Where(pd => !GlobalConstants.IgnoredPlugins.Contains(pd.Name))
					.Prepend(GlobalConstants.FarDirectory)
					.ToList();
		}

		public static int ShowHelp()
		{
			var hlfCheckerName = Path.GetFileName(Process.GetCurrentProcess().MainModule?.FileName);

			$@"
{hlfCheckerName} checks specified help files in specified languages.

Synopsys:
    {hlfCheckerName} [<directory> ...] {{ [+<lng> ...] | [-<lng> ...] }} [{ProcessingOptions.StrictRefs}]
    {hlfCheckerName} {ProcessingOptions.ShowHelp}

Examples:
  Checks help files of Far and all plugins in all languages:
    {hlfCheckerName}

  Checks help files of Far and Temporary panel plugin in all languages:
    {hlfCheckerName} .\far .\plugins\tmppanel

  Checks help files of Far and all plugins in Polish language:
    {hlfCheckerName} +pol

  Checks help files of FAR Commands plugins in all languages except Russian:
    {hlfCheckerName} .\plugins\farcmds -rus

While comparing paragrpahs, {hlfCheckerName} extracts all references (e.g., ""@About@"") from
corresponding paragrpahs and compares the collections of references. By default, if ""{ProcessingOptions.StrictRefs}""
parameter is NOT specified, the collections are sorted and repeated references are removed before
comparing. If ""{ProcessingOptions.StrictRefs}"" parameter is specified, the collections are compared exactly as extracted.
".Trace();

//If a help file was parsed successfully, the script will overwrite it with the parsed contents.
//If there were errors, the file will not be overwritten.All errors and warnings will be printed.
//If the file was clean, its contents will not change(but timestamp will be updated).
//The script will correct empty lines and remove trailing spaces(while printing corresponding warnings).
//Use ""git diff"" command after running the script.

			return -1;
		}
	}
}
