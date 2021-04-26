using System.Collections.Generic;

namespace HlfChecker
{
	public class Hlf
	{
		public Hlf(string hlfName)
		{
			HlfName = hlfName;
			Prologue = new();
			Aliases = new();
			Topics = new();
		}

		public readonly string HlfName;
		public readonly List<string> Prologue;
		public readonly List<string> Aliases;
		public readonly List<Topic> Topics;
	}

	public readonly struct Topic
	{
		public Topic(string title)
		{
			Title = title;
			Headers = new();
			Paragraphs = new();
		}

		public readonly string Title;
		public readonly List<string> Headers;
		public readonly List<Paragraph> Paragraphs;
	}

	public readonly struct Paragraph
	{
		public Paragraph(string firstLine, int expectedBlankLinesBefore)
		{
			Lines = new List<string> { firstLine };
			ExpectedBlankLinesBefore = expectedBlankLinesBefore;
		}

		public readonly List<string> Lines;
		public readonly int ExpectedBlankLinesBefore;
	}
}
