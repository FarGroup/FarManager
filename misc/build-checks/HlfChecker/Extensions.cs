using System;
using System.Collections.Generic;
using System.Linq;

namespace HlfChecker
{
	public static class Extensions
	{
		private static readonly string[] TwoEmpty = new[] { string.Empty, string.Empty };
		private static readonly string Separator = new('-', 80);

		public static IEnumerable<string> Print(this Hlf hlf) =>
			hlf.Prologue.Text()
				.Concat(hlf.Aliases.Count > 0 ? TwoEmpty.Concat(hlf.Aliases.Text()) : Enumerable.Empty<string>())
				.Concat(hlf.Topics.Print());

		public static IEnumerable<string> Print(this IList<Topic> topics) =>
			topics.SelectMany(
				topic => TwoEmpty.Append(topic.Title.Text).Concat(topic.Headers.Text()).Concat(topic.Paragraphs.Print()));

		public static IEnumerable<string> Print(this IList<Paragraph> paragraphs) =>
			paragraphs.SelectMany(
				paragraph => Enumerable.Range(0, paragraph.ExpectedBlankLinesBefore)
					.Select(i => string.Empty)
					.Concat(paragraph.Lines.Text()));

		public static void Trace(this string line, bool printSeparator = false, bool skipLine = true)
		{
			if (skipLine) Console.WriteLine(string.Empty);
			if (printSeparator) Console.WriteLine(Separator);
			Console.WriteLine(line);
		}

		public static IEnumerable<string> Text(this IEnumerable<Line> lines) => lines.Select(line => line.Text);

		public static bool IsStrictRefs(this ProcessingOptions options) => options.HasFlag(ProcessingOptions.StrictRefs);

		public static bool IsVerbose(this ProcessingOptions options) => options.HasFlag(ProcessingOptions.Verbose);
	}
}
