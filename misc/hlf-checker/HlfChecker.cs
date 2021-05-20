using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace HlfChecker
{
	public class Program
	{
		public static int Main(string[] parameters)
		{
			if (!CommandLine.Parse(
				parameters,
				out List<DirectoryInfo> directories,
				out Regex languageFilter,
				out ProcessingOptions options))
			{
				return 1;
			}

			if (options.HasFlag(ProcessingOptions.ShowHelp))
			{
				return CommandLine.ShowHelp();
			}

			return ValidateDirectories(directories, GlobalConstants.MasterLanguage, languageFilter, options) ? 0 : 1;
		}

		private static bool ValidateDirectories(
			IEnumerable<DirectoryInfo> directories,
			string masterLanguage,
			Regex languageFilter,
			ProcessingOptions options)
		{
			return directories.Aggregate(0, (issues, directory) => issues += ValidateDirectory(directory, masterLanguage, languageFilter, options) ? 0 : 1) == 0;
		}

		private static bool ValidateDirectory(
			DirectoryInfo directory,
			string masterLanguage,
			Regex languageFilter,
			ProcessingOptions options)
		{
			var engHlfFile = directory.EnumerateFiles($@"*{masterLanguage}.hlf.*").SingleOrDefault();
			if (engHlfFile == default)
			{
				$"Could not find {masterLanguage}.hlf file in {directory.Name}. Skippng.".Trace(printSeparator: true);
				return true;
			}

			if (options.IsVerbose()) $"Validating directory {directory.Name}...".Trace(printSeparator: true);

			var engHlf = ParseAndPrintHlf(engHlfFile.FullName, options);
			if (engHlf == default)
			{
				$"Errors parsing {engHlfFile.Name}. Exiting.".Trace();
				return false;
			}

			return directory
				.EnumerateFiles(@"*.hlf.*").Where(fi => fi.Name != engHlf.HlfName && languageFilter.Match(fi.Name).Success)
				.Aggregate(0, (issues, lngHlfFile) => issues += CompareHlf(engHlf, lngHlfFile, options) ? 0 : 1) == 0;
		}

		private static bool CompareHlf(Hlf engHlf, FileInfo lngHlfFile, ProcessingOptions options)
		{
			var lngHlf = ParseAndPrintHlf(lngHlfFile.FullName, options);
			if (lngHlf == default)
			{
				$"Errors parsing {lngHlfFile.Name}. Skipping validation.".Trace();
				return false;
			}

			return new HlfComparer(engHlf, lngHlf, options).Compare();
		}

		private static Hlf ParseAndPrintHlf(string hlfPath, ProcessingOptions options)
		{
			var parser = new HlfParser(hlfPath, options);

			foreach (var diagnostic in parser.EnumerateDiagnostics())
			{
				diagnostic.Trace(skipLine: false);
			}

			var hlf = parser.Hlf;

			if (!parser.Success)
			{
				$"*** Errors in {hlf.HlfName}".Trace();
				return default;
			}

			if (parser.Warnings)
			{
				$"*** Warnings in {hlf.HlfName}".Trace();
			}

			// Use "git diff" to make sure the file was parsed correcly
			//$"Writing back {hlf.HlfName}...".Trace();
			//File.WriteAllLines(hlfPath, hlf.Print(), Encoding.UTF8);
			return hlf;
		}
	}
}