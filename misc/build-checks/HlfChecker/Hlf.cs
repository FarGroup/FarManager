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
		public readonly List<Line> Prologue;
		public readonly List<Line> Aliases;
		public readonly List<Topic> Topics;
	}

	public readonly struct Topic
	{
		public Topic(Line title)
		{
			Title = title;
			Headers = new();
			Paragraphs = new();
		}

		public readonly Line Title;
		public readonly List<Line> Headers;
		public readonly List<Paragraph> Paragraphs;
	}

	public readonly struct Paragraph
	{
		public Paragraph(Line firstLine, int expectedBlankLinesBefore)
		{
			Lines = new List<Line> { firstLine };
			ExpectedBlankLinesBefore = expectedBlankLinesBefore;
		}

		public readonly List<Line> Lines;
		public readonly int ExpectedBlankLinesBefore;
	}

	public readonly struct Line
	{
		public Line(string text, int number)
		{
			Text = text;
			Number = number;
		}

		public readonly string Text;
		public readonly int Number;
	}
}
