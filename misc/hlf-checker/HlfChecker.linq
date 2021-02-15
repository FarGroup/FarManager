<Query Kind="Program">
  <Reference>&lt;RuntimeDirectory&gt;\System.Windows.Forms.dll</Reference>
  <Namespace>System.Runtime.CompilerServices</Namespace>
</Query>

private static readonly DirectoryInfo FarFolder = new DirectoryInfo(Path.Combine(Util.CurrentQuery.Location, "..", "..", "far"));

public void Main()
{
	var engHlfFileInfo = FarFolder.EnumerateFiles(@"FarEng.hlf.m4").Single();
	var engHlf = ParseAndPrint(engHlfFileInfo.FullName);
	if (engHlf == default)
	{
		$"Errors parsing {engHlfFileInfo.Name}. Exiting.".Dump();
		return;
	}

	foreach (var lngHlfFileInfo in FarFolder.EnumerateFiles(@"*.hlf.m4").Where(fi => fi.Name != engHlf.HlfName))
	{
		var lngHlf = ParseAndPrint(lngHlfFileInfo.FullName);
		if (lngHlf == default)
		{
			$"Errors parsing {lngHlfFileInfo.Name}. Skipping validation.".Dump();
			continue;
		}

		new HlfComparer(engHlf, lngHlf).Compare();
	}
}

private Hlf ParseAndPrint(string hlfPath)
{
	var parser = new HlfParser(hlfPath);

	foreach (var diagnostic in parser.EnumerateDiagnostics())
	{
		diagnostic.Dump();
	}

	if (!parser.Success)
	{
		return default;
	}

	var hlf = parser.Hlf;

	// Use "git diff" to make sure the file was parsed correcly
	File.WriteAllLines(hlfPath, hlf.Print(), Encoding.UTF8);
	return hlf;
}

public class HlfComparer
{
	private Hlf EngHlf;
	private Hlf LngHlf;

	public HlfComparer(Hlf engHlf, Hlf lngHlf)
	{
		EngHlf = engHlf;
		LngHlf = lngHlf;
	}

	public bool Compare()
	{
		$@"
Comparing {EngHlf.HlfName} and {LngHlf.HlfName}"
			.Dump();

		var result = true;

		if (!ComparePrologue()) result = false;
		if (!CompareAliases()) result = false;
		if (!CompareTopics()) result = false;

		return result;
	}

	private static readonly Regex PrologueLanguage = new Regex(@"\.Language=\w+,\w+(\s\(\w+\))?", RegexOptions.Compiled | RegexOptions.ExplicitCapture);

	private bool ComparePrologue()
	{
		return CompareLists(EngHlf.Prologue, LngHlf.Prologue,
							prologue => PrologueLanguage.Replace(prologue, ".Language="),
							prologue => "line",
							EngHlf.HlfName, LngHlf.HlfName, "Prologue");
	}

	private bool CompareAliases()
	{
		return CompareLists(EngHlf.Aliases, LngHlf.Aliases,
							alias => alias,
							alias => string.Empty,
							EngHlf.HlfName, LngHlf.HlfName, "Aliases");
	}

	private bool CompareTopics()
	{
		return CompareLists(EngHlf.Topics, LngHlf.Topics,
							topic => topic.Title,
							topic => ".Title",
							EngHlf.HlfName, LngHlf.HlfName, "Topics")
			&& CompareLists(EngHlf.Topics, LngHlf.Topics,
							topic => topic.Headers.Count,
							topic => $"[\"{topic.Title}\"].Headers.Count",
							EngHlf.HlfName, LngHlf.HlfName, "Topics");
	}

	private static bool CompareLists<Ent, Elem>(List<Ent> eng, List<Ent> lng,
		Func<Ent, Elem> Element,
		Func<Ent, string> Context,
		string engHlfName, string lngHlfName, string entityName)
		where Elem : IEquatable<Elem>
	{
		if (eng.Count != lng.Count)
		{
			$@"
Mismatch:
    {engHlfName}: {entityName}.Count: {eng.Count}
    {lngHlfName}: {entityName}.Count: {lng.Count}"
				.Dump();
			return false;
		}

		var result = true;

		foreach (var comp in eng.Zip(lng, (e, l) => (eng: e, lng: l))
			.Select((c, i) => (i: i, eng: (ent: c.eng, elem: Element(c.eng)), lng: (ent: c.lng, elem: Element(c.lng))))
			.Where(c => !c.eng.elem.Equals(c.lng.elem)))
		{
			$@"
Mismatch: {entityName}[{comp.i}]:
    {engHlfName}: {entityName}{Context(comp.eng.ent)}: {comp.eng.elem}
    {lngHlfName}: {entityName}{Context(comp.lng.ent)}: {comp.lng.elem}"
				.Dump();
				
			result = false;
		}

		return result;
	}
}

public class HlfParser
{
	public readonly Hlf Hlf;

	public HlfParser(string hlfPath)
	{
		Hlf = new Hlf{ HlfName = Path.GetFileName(hlfPath) };
		$@"
Parsing {Hlf.HlfName}"
			.Dump();

		Parser parser = Start;
		Token token = new Token();

		foreach (var line in File.ReadLines(hlfPath))
		{
			token = new Token(line, token);
			ValidateToken(token);
			parser = parser(token);
		}

		parser(new Token(default, token));
	}

	private Parser Start(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Normal: Hlf.Prologue.Add(token.line); return Prologue;
		}

		AddUnexpected(token, new[]{ Token.Kind.Normal });
		return Start;
	}

	private Parser Prologue(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return BeforeAliases;
			case Token.Kind.Normal: Hlf.Prologue.Add(token.line); return Prologue;
		}

		AddUnexpected(token, new[]{ Token.Kind.Empty, Token.Kind.Normal });
		return SkipToAliases;
	}

	private Parser SkipToAliases(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Alias: StartAliases(token); return Aliases;
		}

		return SkipToAliases;
	}

	private Parser BeforeAliases(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return BeforeAliases;
			case Token.Kind.Alias: StartAliases(token); return Aliases;
		}

		AddUnexpected(token, new[]{ Token.Kind.Empty, Token.Kind.Alias });
		return SkipToAliases;
	}

	private void StartAliases(Token token)
	{
		CheckBlankLinesBefore(token, 2, "Aliases");
		Hlf.Aliases.Add(token.line);
	}

	private Parser Aliases(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return BeforeTopic;
			case Token.Kind.Alias: Hlf.Aliases.Add(token.line); return Aliases;
		}

		AddUnexpected(token, new[]{ Token.Kind.Empty, Token.Kind.Alias });
		return SkipToTopic;
	}

	private Parser SkipToTopic(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Title: StartTopic(token); return Header;
		}

		return SkipToTopic;
	}

	private Parser BeforeTopic(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return BeforeTopic;
			case Token.Kind.Title: StartTopic(token); return Header;
		}

		AddUnexpected(token, new[]{ Token.Kind.Empty, Token.Kind.Title });
		return SkipToTopic;
	}

	private void StartTopic(Token token)
	{
		CheckBlankLinesBefore(token, 2, "Topic");
		Hlf.Topics.Add(new Topic{ Title = token.line });
	}

	private Parser Header(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return AfterHeader;
			case Token.Kind.Header: Hlf.Topics.Last().Headers.Add(token.line); return Header;
			case Token.Kind.Normal: StartParagraph(token, 0); return Paragraph;
		}

		AddUnexpected(token, new[]{ Token.Kind.Header, Token.Kind.Normal });
		return SkipToTopic;
	}

	private Parser AfterHeader(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return AfterHeader;
			case Token.Kind.Normal: StartParagraph(token, 0); return Paragraph;
		}

		AddUnexpected(token, new[] { Token.Kind.Empty, Token.Kind.Title, Token.Kind.Normal });
		return SkipToTopic;
	}

	private void StartParagraph(Token token, int expectedBlankLinesBefore)
	{
		CheckBlankLinesBefore(token, expectedBlankLinesBefore, "Paragraph");
		Hlf.Topics.Last().Paragraphs.Add(new Paragraph { Lines = new List<string> { token.line }, BlankLinesBefore = token.blankLinesBefore });
	}

	private Parser Paragraph(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return AfterParagraph;
			case Token.Kind.Normal: Hlf.Topics.Last().Paragraphs.Last().Lines.Add(token.line); return Paragraph;
			case Token.Kind.Eof: return Eof;
		}

		AddUnexpected(token, new[]{ Token.Kind.Empty, Token.Kind.Normal });
		return SkipToTopic;
	}

	private Parser AfterParagraph(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return AfterParagraph;
			case Token.Kind.Title: StartTopic(token); return Header;
			case Token.Kind.Normal: StartParagraph(token, 1); return Paragraph;
		}

		AddUnexpected(token, new[]{ Token.Kind.Empty, Token.Kind.Title, Token.Kind.Normal });
		return SkipToTopic;
	}

	private Parser Eof(Token token)
	{
		AddUnexpected(token, new[]{ Token.Kind.None });
		return Eof;
	}

	private void CheckBlankLinesBefore(Token token, int expected, string entity)
	{
		if (token.blankLinesBefore != expected)
		{
			AddWarning(token, $"Expected {expected} empty line(s) before {entity}; actual {token.blankLinesBefore}");
		}
	}

	private void ValidateToken(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Alias:
			case Token.Kind.Title:
			case Token.Kind.Header:
			case Token.Kind.Normal:
				if (token.line[token.line.Length - 1] == ' ')
				{
					AddWarning(token, "Trailing space");
				}
				break;
		}
	}

	private static Token.Kind ClassifyLine(string line)
	{
		if (line == default) return Token.Kind.Eof;
		if (line.Length == 0) return Token.Kind.Empty;

		if (line[0] == '@')
		{
			if (line.Length == 1 || !char.IsLetter(line[1])) return Token.Kind.Normal;
			return line.IndexOf('=', 2) < 0 ? Token.Kind.Title : Token.Kind.Alias;
		}

		if (line[0] == '$' || line.Length >= 2 && line[0] == '`' && line[1] == '$') return Token.Kind.Header;

		return Token.Kind.Normal;
	}

	private struct Token
	{
		public enum Kind { None, Empty, Alias, Title, Header, Normal, Eof }

		public string line;
		public Kind kind;
		public int lineNumber;
		public int blankLinesBefore;

		public Token(string line, Token prevToken)
		{
			this.line = line;
			this.lineNumber = prevToken.lineNumber + 1;

			this.kind = ClassifyLine(line);

			switch (prevToken.kind)
			{
				case Kind.Empty: this.blankLinesBefore = prevToken.blankLinesBefore + 1; return;

				case Kind.None:
				case Kind.Alias:
				case Kind.Title:
				case Kind.Header:
				case Kind.Normal: this.blankLinesBefore = 0; return;

				case Kind.Eof: this.blankLinesBefore = prevToken.blankLinesBefore; return;

				default: throw new Exception($"Line {this.lineNumber}: Invalid token kind {kind}");
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

	private void AddDiagnostics(Diagnostics.Severity severity, Token token, string message)
	{
		diagnostics.Add(new Diagnostics(severity, token, message));
	}

	private void AddWarning(Token token, string message)
	{
		AddDiagnostics(Diagnostics.Severity.Warning, token, message);
	}

	private void AddError(Token token, string message)
	{
		AddDiagnostics(Diagnostics.Severity.Error, token, message);
	}

	private void AddUnexpected(Token token, Token.Kind[] expected, [CallerMemberName] string parser = default)
	{
		AddError(token, $"Unexpected token kind '{token.kind}' while parsing '{parser}'. Expected: {string.Join(", ", expected)}");
	}

	public IEnumerable<string> EnumerateDiagnostics()
	{
		return diagnostics
			.Select(diagnostic => $"{Hlf.HlfName}({diagnostic.token.lineNumber}): {diagnostic.severity}: {diagnostic.message}");
	}

	public bool Success => diagnostics.All(diagnostic => diagnostic.severity < Diagnostics.Severity.Error);

	private delegate Parser Parser(Token token);
	private List<Diagnostics> diagnostics = new List<Diagnostics>();
}

public class Hlf
{
	public string HlfName;
	public List<string> Prologue = new List<string>();
	public List<string> Aliases = new List<string>();
	public List<Topic> Topics = new List<Topic>();
}

public class Topic
{
	public string Title;
	public List<string> Headers = new List<string>();
	public List<Paragraph> Paragraphs = new List<Paragraph>();
}

public class Paragraph
{
	public List<string> Lines;
	public int BlankLinesBefore;
}

public static class Extensions
{
	private static readonly string[] TwoEmpty = new[] { string.Empty, string.Empty };

	public static IEnumerable<string> Print(this Hlf hlf)
	{
		return hlf.Prologue.Concat(TwoEmpty).Concat(hlf.Aliases).Concat(hlf.Topics.Print());
	}

	public static IEnumerable<string> Print(this IList<Topic> topics)
	{
		return topics.SelectMany(
			topic => TwoEmpty.Append(topic.Title).Concat(topic.Headers).Concat(topic.Paragraphs.Print()));
	}

	public static IEnumerable<string> Print(this IList<Paragraph> paragraphs)
	{
		return paragraphs.SelectMany(
			paragraph => Enumerable.Range(0, paragraph.BlankLinesBefore).Select(i => string.Empty).Concat(paragraph.Lines));
	}
}