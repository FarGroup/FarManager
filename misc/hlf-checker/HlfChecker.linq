<Query Kind="Program">
  <Reference>&lt;RuntimeDirectory&gt;\System.Windows.Forms.dll</Reference>
  <Namespace>System.Runtime.CompilerServices</Namespace>
</Query>

private static readonly DirectoryInfo FarDirectory = new DirectoryInfo(Path.Combine(Util.CurrentQuery.Location, "..", "..", "far"));
private static readonly DirectoryInfo PluginsDirectory = new DirectoryInfo(Path.Combine(Util.CurrentQuery.Location, "..", "..", "plugins"));
private static readonly string MainLanguage = "Eng";
private static readonly HashSet<string> IgnoredPlugins = new HashSet<string>{ "common", "ftp" };

public void Main(string[] parameters)
{
	var languageFilter = GetLanguageFilter(parameters);
	if (languageFilter == default) return;

	ValidateDirectories(GetDirectories(parameters), MainLanguage, languageFilter);
}

private IList<DirectoryInfo> GetDirectories(string[] parameters)
{
	var directories = (parameters ?? Enumerable.Empty<string>())
		.Where(param => !string.IsNullOrEmpty(param) && param[0] != '+' && param[0] != '-')
		.Select(directory => new DirectoryInfo(directory))
		.ToList();

	return directories.Count > 0
		? directories
		: PluginsDirectory.EnumerateDirectories().Where(pd => !IgnoredPlugins.Contains(pd.Name)).Prepend(FarDirectory).ToList();
}

private static readonly Regex ValidLanguageFilter = new Regex(@"^[+-]\p{L}{3}$", RegexOptions.Compiled | RegexOptions.CultureInvariant);

private static Regex GetLanguageFilter(string[] parameters)
{
	var requested = parameters?.Where(param => param?.StartsWith("+") == true).ToList();
	var excluded = parameters?.Where(param => param?.StartsWith("-") == true).ToList();

	if ((requested?.Count ?? 0) == 0 && (excluded?.Count ?? 0) == 0)
	{
		return new Regex(@"\p{L}{3}\.hlf", RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase);
	}

	if (requested.Count > 0 && excluded.Count > 0)
	{
		$"Cannot apply requested ({string.Join(", ", requested)}) and exculded ({string.Join(", ", excluded)}) language filters together. Exiting.".Trace();
		return default;
	}

	var filters = requested.Count > 0 ? requested : excluded;

	var invalid = filters.Where(lng => !ValidLanguageFilter.Match(lng).Success).ToList();
	if (invalid.Count != 0)
	{
		$"Invalid language filter(s) specified: ({string.Join(", ", invalid)}). Exiting.".Trace();
		return default;
	}

	$"Language filters applied: ({string.Join(", ", filters)}).".Trace();

	return new Regex(
		(requested.Count > 0 ? "(" : @"\p{L}{3}(?<!") + string.Join("|", filters.Select(f => f.Substring(1))) + @")\.hlf",
		RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase);
}

private static void ValidateDirectories(IEnumerable<DirectoryInfo> directories, string mainLanguage, Regex languageFilter)
{
	foreach (var directory in directories)
	{
		ValidateDirectory(directory, mainLanguage, languageFilter);
	}
}

private static void ValidateDirectory(DirectoryInfo directory, string mainLanguage, Regex languageFilter)
{
	var engHlfFile = directory.EnumerateFiles($@"*{mainLanguage}.hlf.*").SingleOrDefault();
	if (engHlfFile == default)
	{
		$"Could not find {mainLanguage}.hlf file in {directory.Name}. Skippng.".Trace(printSeparator: true);
		return;
	}

	$"Validating diretory {directory.Name}...".Trace(printSeparator: true);

	var engHlf = ParseAndPrintHlf(engHlfFile.FullName);
	if (engHlf == default)
	{
		$"Errors parsing {engHlfFile.Name}. Exiting.".Trace();
		return;
	}

	foreach (var lngHlfFile
			in directory.EnumerateFiles(@"*.hlf.*").Where(fi => fi.Name != engHlf.HlfName && languageFilter.Match(fi.Name).Success))
	{
		var lngHlf = ParseAndPrintHlf(lngHlfFile.FullName);
		if (lngHlf == default)
		{
			$"Errors parsing {lngHlfFile.Name}. Skipping validation.".Trace();
			continue;
		}

		new HlfComparer(engHlf, lngHlf).Compare();
	}
}

private static Hlf ParseAndPrintHlf(string hlfPath)
{
	var parser = new HlfParser(hlfPath);

	foreach (var diagnostic in parser.EnumerateDiagnostics())
	{
		diagnostic.Dump();
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
	$"Writing back {hlf.HlfName}...".Trace();
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
		$"Comparing {EngHlf.HlfName} and {LngHlf.HlfName}...".Trace();

		var result = true;

		if (!ComparePrologue()) result = false;
		if (!CompareAliases()) result = false;
		if (!CompareTopics()) result = false;

		if (!result) $"*** Issues in {LngHlf.HlfName}".Trace();

		return result;
	}

	private static readonly Regex PrologueLanguage = new Regex(@"^\.Language=\p{L}+,\p{L}+(\s\(\p{L}+\))?$", RegexOptions.Compiled | RegexOptions.ExplicitCapture);
	private static readonly Regex ProloguePluginContents = new Regex(@"^\.PluginContents=.+$", RegexOptions.Compiled | RegexOptions.ExplicitCapture);

	private bool ComparePrologue()
	{
		return CompareLists(EngHlf.Prologue, LngHlf.Prologue,
							prologue => ProloguePluginContents.Replace(PrologueLanguage.Replace(prologue, ".Language="), ".PluginContents="),
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
							EngHlf.HlfName, LngHlf.HlfName, "Topics")
			&& CompareLists(EngHlf.Topics, LngHlf.Topics,
							topic => topic.Paragraphs.Count,
							topic => $"[\"{topic.Title}\"].Paragraphs.Count",
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
			$@"Mismatch:
    {engHlfName}: {entityName}.Count: {eng.Count}
    {lngHlfName}: {entityName}.Count: {lng.Count}".Trace();
			return false;
		}

		var result = true;

		foreach (var comp in eng.Zip(lng, (e, l) => (eng: e, lng: l))
			.Select((c, i) => (i: i, eng: (ent: c.eng, elem: Element(c.eng)), lng: (ent: c.lng, elem: Element(c.lng))))
			.Where(c => !c.eng.elem.Equals(c.lng.elem)))
		{
			$@"Mismatch: {entityName}[{comp.i}]:
    {engHlfName}: {entityName}{Context(comp.eng.ent)}: {comp.eng.elem}
    {lngHlfName}: {entityName}{Context(comp.lng.ent)}: {comp.lng.elem}".Trace();
				
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
		$"Parsing {Hlf.HlfName}...".Trace();

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
			case Token.Kind.Empty: return AfterPrologues;
			case Token.Kind.Normal: Hlf.Prologue.Add(token.line); return Prologue;
		}

		AddUnexpected(token, new[]{ Token.Kind.Empty, Token.Kind.Normal });
		return SkipToAliasesOrTopic;
	}

	private Parser SkipToAliasesOrTopic(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Alias: StartAliases(token); return Aliases;
			case Token.Kind.Title: StartTopic(token); return Header;
		}

		return SkipToAliasesOrTopic;
	}

	private Parser AfterPrologues(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return AfterPrologues;
			case Token.Kind.Alias: StartAliases(token); return Aliases;
			case Token.Kind.Title: StartTopic(token); return Header;
		}

		AddUnexpected(token, new[]{ Token.Kind.Empty, Token.Kind.Alias });
		return SkipToAliasesOrTopic;
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
			case Token.Kind.Eof: AddEmptyTopic(token); return Eof;
		}

		AddUnexpected(token, new[]{ Token.Kind.Header, Token.Kind.Normal });
		return SkipToTopic;
	}

	private Parser AfterHeader(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Empty: return AfterHeader;
			case Token.Kind.Title: AddEmptyTopic(token); StartTopic(token); return Header;
			case Token.Kind.Normal: StartParagraph(token, 0); return Paragraph;
			case Token.Kind.Eof: AddEmptyTopic(token); return Eof;
		}

		AddUnexpected(token, new[] { Token.Kind.Empty, Token.Kind.Title, Token.Kind.Normal });
		return SkipToTopic;
	}

	private void StartParagraph(Token token, int expectedBlankLinesBefore)
	{
		CheckBlankLinesBefore(token, expectedBlankLinesBefore, "Paragraph");
		Hlf.Topics.Last().Paragraphs.Add(new Paragraph
		{
			Lines = new List<string> { token.line },
			ExpectedBlankLinesBefore = expectedBlankLinesBefore
		});
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
			AddWarning(token, $"Expected {expected} empty line(s) before {entity}; actual {token.blankLinesBefore}. Corrected");
		}
	}

	private Token ValidateToken(Token token)
	{
		switch (token.kind)
		{
			case Token.Kind.Alias:
			case Token.Kind.Title:
			case Token.Kind.Header:
			case Token.Kind.Normal:
				if (token.line[token.line.Length - 1] == ' ')
				{
					AddWarning(token, "Trailing space(s) removed");
					return new Token
					{
						line = token.line.TrimEnd(),
						kind = token.kind,
						lineNumber = token.lineNumber,
						blankLinesBefore = token.blankLinesBefore
					};
				}
				break;
		}

		return token;
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

	private void AddEmptyTopic(Token token)
	{
		AddWarning(token, $"Empty Topic {Hlf.Topics.Last().Title}");
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
	public bool Warnings => diagnostics.Any(diagnostic => diagnostic.severity == Diagnostics.Severity.Warning);

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
	public int ExpectedBlankLinesBefore;
}

public static class Extensions
{
	private static readonly string[] TwoEmpty = new[] { string.Empty, string.Empty };
	private static readonly string Separator = new string('-', 80);

	public static IEnumerable<string> Print(this Hlf hlf)
	{
		return hlf.Prologue
			.Concat(hlf.Aliases.Count > 0 ? TwoEmpty.Concat(hlf.Aliases) : Enumerable.Empty<string>())
			.Concat(hlf.Topics.Print());
	}

	public static IEnumerable<string> Print(this IList<Topic> topics)
	{
		return topics.SelectMany(
			topic => TwoEmpty.Append(topic.Title).Concat(topic.Headers).Concat(topic.Paragraphs.Print()));
	}

	public static IEnumerable<string> Print(this IList<Paragraph> paragraphs)
	{
		return paragraphs.SelectMany(
			paragraph => Enumerable.Range(0, paragraph.ExpectedBlankLinesBefore)
				.Select(i => string.Empty)
				.Concat(paragraph.Lines));
	}

	public static void Trace(this string line, bool printSeparator = false)
	{
		string.Empty.Dump();
		if (printSeparator) Separator.Dump();
		line.Dump();
	}
}