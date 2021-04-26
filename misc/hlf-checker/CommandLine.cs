using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace HlfChecker
{
	[Flags]
	public enum ProcessingOptions
	{
		None = 0,
		Verbose = 1,
		StrictRefs = 2
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

		private static readonly Regex ValidLanguageFilter = new Regex(@"^[+-]\p{L}{3}$", RegexOptions.Compiled | RegexOptions.CultureInvariant);

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
				$"Cannot apply requested ({string.Join(", ", requested)}) and exculded ({string.Join(", ", excluded)}) language filters together. Exiting.".Trace();
				return default;
			}

			var filters = requested.Count > 0 ? requested : excluded;

			var invalid = filters.Where(lng => !ValidLanguageFilter.IsMatch(lng)).ToList();
			if (invalid.Count != 0)
			{
				$"Invalid language filter(s) specified: ({string.Join(", ", invalid)}). Exiting.".Trace();
				return default;
			}

			$"Language filters applied: ({string.Join(", ", filters)}).".Trace();

			var outParamList = paramList.Except(filters).ToList();
			paramList.Clear();
			paramList.AddRange(outParamList);

			return new Regex(
				(requested.Count > 0 ? "(" : @"\p{L}{3}(?<!") + string.Join("|", filters.Select(f => f.Substring(1))) + @")\.hlf",
				RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase);
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
	}
}
