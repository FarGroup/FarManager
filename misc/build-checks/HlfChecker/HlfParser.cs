using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;

namespace HlfChecker
{
	public class HlfParser
	{
		public readonly Hlf Hlf;

		public HlfParser(string hlfPath, ProcessingOptions options)
		{
			Hlf = new Hlf(Path.GetFileName(hlfPath));
			if (options.IsVerbose()) $"Parsing {Hlf.HlfName}...".Trace();

			Parser parser = Start;
			Token token = new Token();

			foreach (var line in File.ReadLines(hlfPath))
			{
				token = new Token(line, token);
				token = ValidateToken(token);
				parser = parser(token);
			}

			parser(new Token(default, token));
		}

		private Parser Start(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Normal: Hlf.Prologue.Add(token.Line); return Prologue;
			}

			AddUnexpected(token, new[] { TokenKind.Normal });
			return Start;
		}

		private Parser Prologue(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Empty: return AfterPrologues;
				case TokenKind.Normal: Hlf.Prologue.Add(token.Line); return Prologue;
			}

			AddUnexpected(token, new[] { TokenKind.Empty, TokenKind.Normal });
			return SkipToAliasesOrTopic;
		}

		private Parser SkipToAliasesOrTopic(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Alias: StartAliases(token); return Aliases;
				case TokenKind.Title: StartTopic(token); return Header;
			}

			return SkipToAliasesOrTopic;
		}

		private Parser AfterPrologues(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Empty: return AfterPrologues;
				case TokenKind.Alias: StartAliases(token); return Aliases;
				case TokenKind.Title: StartTopic(token); return Header;
			}

			AddUnexpected(token, new[] { TokenKind.Empty, TokenKind.Alias });
			return SkipToAliasesOrTopic;
		}

		private void StartAliases(Token token)
		{
			CheckBlankLinesBefore(token, 2, "Aliases");
			Hlf.Aliases.Add(token.Line);
		}

		private Parser Aliases(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Empty: return BeforeTopic;
				case TokenKind.Alias: Hlf.Aliases.Add(token.Line); return Aliases;
			}

			AddUnexpected(token, new[] { TokenKind.Empty, TokenKind.Alias });
			return SkipToTopic;
		}

		private Parser SkipToTopic(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Title: StartTopic(token); return Header;
			}

			return SkipToTopic;
		}

		private Parser BeforeTopic(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Empty: return BeforeTopic;
				case TokenKind.Title: StartTopic(token); return Header;
			}

			AddUnexpected(token, new[] { TokenKind.Empty, TokenKind.Title });
			return SkipToTopic;
		}

		private void StartTopic(Token token)
		{
			CheckBlankLinesBefore(token, 2, "Topic");
			Hlf.Topics.Add(new Topic(token.Line));
		}

		private Parser Header(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Empty: return AfterHeader;
				case TokenKind.Header: Hlf.Topics.Last().Headers.Add(token.Line); return Header;
				case TokenKind.Normal: StartParagraph(token, 0); return Paragraph;
				case TokenKind.Eof: AddEmptyTopic(token); return Eof;
			}

			AddUnexpected(token, new[] { TokenKind.Header, TokenKind.Normal });
			return SkipToTopic;
		}

		private Parser AfterHeader(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Empty: return AfterHeader;
				case TokenKind.Title: AddEmptyTopic(token); StartTopic(token); return Header;
				case TokenKind.Normal: StartParagraph(token, 0); return Paragraph;
				case TokenKind.Eof: AddEmptyTopic(token); return Eof;
			}

			AddUnexpected(token, new[] { TokenKind.Empty, TokenKind.Title, TokenKind.Normal });
			return SkipToTopic;
		}

		private void StartParagraph(Token token, int expectedBlankLinesBefore)
		{
			CheckBlankLinesBefore(token, expectedBlankLinesBefore, "Paragraph");
			Hlf.Topics.Last().Paragraphs.Add(new Paragraph(token.Line, expectedBlankLinesBefore));
		}

		private Parser Paragraph(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Empty: return AfterParagraph;
				case TokenKind.Normal: Hlf.Topics.Last().Paragraphs.Last().Lines.Add(token.Line); return Paragraph;
				case TokenKind.Eof: return Eof;
			}

			AddUnexpected(token, new[] { TokenKind.Empty, TokenKind.Normal });
			return SkipToTopic;
		}

		private Parser AfterParagraph(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Empty: return AfterParagraph;
				case TokenKind.Title: StartTopic(token); return Header;
				case TokenKind.Normal: StartParagraph(token, 1); return Paragraph;
			}

			AddUnexpected(token, new[] { TokenKind.Empty, TokenKind.Title, TokenKind.Normal });
			return SkipToTopic;
		}

		private Parser Eof(Token token)
		{
			AddUnexpected(token, new[] { TokenKind.None });
			return Eof;
		}

		private void CheckBlankLinesBefore(Token token, int expected, string entity)
		{
			if (token.BlankLinesBefore != expected)
			{
				AddWarning(token, $"Expected {expected} empty line(s) before {entity}; actual {token.BlankLinesBefore}. Corrected");
			}
		}

		private Token ValidateToken(Token token)
		{
			switch (token.Kind)
			{
				case TokenKind.Alias:
				case TokenKind.Title:
				case TokenKind.Header:
				case TokenKind.Normal:
					if (token.Line.Text[token.Line.Text.Length - 1] == ' ')
					{
						AddWarning(token, "Trailing space(s) removed");
						return new Token(token.Line.Text.TrimEnd(), token.Line.Number, token.Kind, token.BlankLinesBefore);
					}
					break;
			}

			return token;
		}

		private static TokenKind ClassifyLine(string line)
		{
			if (line == default) return TokenKind.Eof;
			if (line.Length == 0) return TokenKind.Empty;

			if (line[0] == '@')
			{
				if (line.Length == 1 || !char.IsLetter(line[1])) return TokenKind.Normal;
				return line.IndexOf('=', 2) < 0 ? TokenKind.Title : TokenKind.Alias;
			}

			if (line[0] == '$' || (line.Length >= 2 && line[0] == '`' && line[1] == '$')) return TokenKind.Header;

			return TokenKind.Normal;
		}

		private enum TokenKind { None, Empty, Alias, Title, Header, Normal, Eof }

		private readonly struct Token
		{
			public readonly Line Line;
			public readonly TokenKind Kind;
			public readonly int BlankLinesBefore;

			public Token(string line, int lineNumber, TokenKind kind, int blankLinesBefore)
			{
				Line = new Line(line, lineNumber);
				Kind = kind;
				BlankLinesBefore = blankLinesBefore;
			}

			public Token(string line, Token prevToken)
			{
				Line = new Line(line, prevToken.Line.Number + 1);

				Kind = ClassifyLine(Line.Text);

				switch (prevToken.Kind)
				{
					case TokenKind.Empty: BlankLinesBefore = prevToken.BlankLinesBefore + 1; return;

					case TokenKind.None:
					case TokenKind.Alias:
					case TokenKind.Title:
					case TokenKind.Header:
					case TokenKind.Normal: BlankLinesBefore = 0; return;

					case TokenKind.Eof: BlankLinesBefore = prevToken.BlankLinesBefore; return;

					default: throw new Exception($"Line {Line.Number}: Invalid token kind {Kind}");
				}
			}
		}

		private struct Diagnostics
		{
			public enum Severity { None, Warning, Error }

			public Severity severity;
			public Token token;
			public string message;

			public Diagnostics(Severity severity, Token token, string message)
			{
				this.severity = severity;
				this.token = token;
				this.message = message;
			}
		}

		private void AddDiagnostics(Diagnostics.Severity severity, Token token, string message) => diagnostics.Add(new Diagnostics(severity, token, message));

		private void AddWarning(Token token, string message) => AddDiagnostics(Diagnostics.Severity.Warning, token, message);

		private void AddError(Token token, string message) => AddDiagnostics(Diagnostics.Severity.Error, token, message);

		private void AddEmptyTopic(Token token) => AddWarning(token, $"Empty Topic {Hlf.Topics.Last().Title}");

		private void AddUnexpected(Token token, TokenKind[] expected, [CallerMemberName] string parser = default) =>
			AddError(token, $"Unexpected token kind '{token.Kind}' while parsing '{parser}'. Expected: {string.Join(", ", expected)}");

		public IEnumerable<string> EnumerateDiagnostics() =>
			diagnostics.Select(diagnostic => $"{Hlf.HlfName}({diagnostic.token.Line.Number}): {diagnostic.severity}: {diagnostic.message}");

		public bool Success => diagnostics.All(diagnostic => diagnostic.severity < Diagnostics.Severity.Error);
		public bool Warnings => diagnostics.Any(diagnostic => diagnostic.severity == Diagnostics.Severity.Warning);

		private delegate Parser Parser(Token token);
		private List<Diagnostics> diagnostics = new();
	}
}
