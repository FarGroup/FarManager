using System.Collections.Generic;
using System.IO;

namespace HlfChecker
{
	public static class GlobalConstants
	{
		public static readonly DirectoryInfo FarDirectory = new(Path.Combine(GeneratedConstants.FarManagerRootDir, "far"));
		public static readonly DirectoryInfo PluginsDirectory = new(Path.Combine(GeneratedConstants.FarManagerRootDir, "plugins"));
		public static readonly HashSet<string> IgnoredPlugins = new() { "common", "ftp" };

		public const string MasterLanguage = "Eng";
	}
}
