using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace HlfChecker
{
	public class HlfComparer
	{
		private Hlf EngHlf;
		private Hlf LngHlf;
		private ProcessingOptions Options;

		public HlfComparer(Hlf engHlf, Hlf lngHlf, ProcessingOptions options)
		{
			EngHlf = engHlf;
			LngHlf = lngHlf;
			Options = options;
		}

		public bool Compare()
		{
			if (Options.IsVerbose()) $"Comparing {EngHlf.HlfName} and {LngHlf.HlfName}...".Trace();

			var result = true;

			if (!ComparePrologue()) result = false;
			if (!CompareAliases()) result = false;
			if (!CompareTopics()) result = false;
			if (!CompareParagraphs()) result = false;

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
			var result = true;

			if (!CompareLists(EngHlf.Topics, LngHlf.Topics,
								topic => topic.Title,
								topic => ".Title",
								EngHlf.HlfName, LngHlf.HlfName, "Topics"))
			{
				result = false;
			}

			if (!CompareLists(EngHlf.Topics, LngHlf.Topics,
								topic => topic.Headers.Count,
								topic => $"[\"{topic.Title}\"].Headers.Count",
								EngHlf.HlfName, LngHlf.HlfName, "Topics"))
			{
				result = false;
			}

			return result;
		}

		private static Regex Reference = new Regex(@"@[\w.]+@", RegexOptions.Compiled);

		private string References(Paragraph paragraph)
		{
			var references = paragraph.Lines.SelectMany(line => Reference.Matches(line).Cast<Match>().Select(match => match.Value));

			if (!Options.IsStrictRefs())
			{
				references = references.Distinct().OrderBy(reference => reference);
			}

			return string.Join(", ", references);
		}

		private bool CompareParagraphs()
		{
			var result = true;

			foreach (var topicPair in EngHlf.Topics.Zip(LngHlf.Topics, (e, l) => (eng: e, lng: l)))
			{
				if (!CompareLists(topicPair.eng.Paragraphs, topicPair.lng.Paragraphs,
								  paragraph => References(paragraph),
								  paragraph => $"[{paragraph.Lines[0].Substring(0, 16)}...].References",
								  EngHlf.HlfName, LngHlf.HlfName, $"Topics[{topicPair.eng.Title}].Paragraphs"))
				{
					result = false;
				}
			}

			return result;
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
}
