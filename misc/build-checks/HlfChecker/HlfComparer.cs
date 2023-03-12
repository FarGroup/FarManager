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

			if (!CompareTopics()) return false;

			if (!CompareTopicHeaders()) result = false;
			if (!CompareParagraphs()) result = false;

			if (!result) $"*** Issues in {LngHlf.HlfName}".Trace();

			return result;
		}

		private static readonly Regex PrologueLanguage = new Regex(@"^\.Language=\p{L}+,\p{L}+(\s\(\p{L}+\))?$", RegexOptions.Compiled | RegexOptions.ExplicitCapture);
		private static readonly Regex ProloguePluginContents = new Regex(@"^\.PluginContents=.+$", RegexOptions.Compiled | RegexOptions.ExplicitCapture);

		private bool ComparePrologue()
		{
			return CompareLists("Prologue", EngHlf.Prologue, LngHlf.Prologue,
				prologueLine => ProloguePluginContents.Replace(PrologueLanguage.Replace(prologueLine.Text, ".Language="), ".PluginContents="),
				"Prologue line", prologueLine => prologueLine.Number);
		}

		private bool CompareAliases()
		{
			return CompareLists("Aliases", EngHlf.Aliases, LngHlf.Aliases,
				alias => alias.Text,
				"Aliases", alias => alias.Number);
		}

		private bool CompareTopics()
		{
			return CompareLists("Topics", EngHlf.Topics, LngHlf.Topics,
				topic => topic.Title.Text,
				"Topic title", topic => topic.Title.Number);
		}

		private bool CompareTopicHeaders()
		{
			return CompareLists("Topics", EngHlf.Topics, LngHlf.Topics,
				topic => topic.Headers.Count,
				"Topic headers count", topic => topic.Headers.Count > 0 ? topic.Headers[0].Number : topic.Title.Number);
		}

		private static readonly Regex Reference = new Regex(@"@[\w.]+@", RegexOptions.Compiled);

		private string References(Paragraph paragraph)
		{
			var references = paragraph.Lines.SelectMany(line => Reference.Matches(line.Text).Cast<Match>().Select(match => match.Value));

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
				if (!CompareLists($"Topics[{topicPair.eng.Title.Text}].Paragraphs", topicPair.eng.Paragraphs, topicPair.lng.Paragraphs,
					paragraph => References(paragraph),
					"Paragraph references", paragraph => paragraph.Lines[0].Number))
				{
					result = false;
				}
			}

			return result;
		}

		private bool CompareLists<Ent, Elem>(string collectionName, List<Ent> eng, List<Ent> lng,
			Func<Ent, Elem> element,
			string mismatchKind, Func<Ent, int> lineNumber)
			where Elem : IEquatable<Elem>
		{
			if (eng.Count != lng.Count)
			{
				$@"Mismatch: {collectionName}.Count
    {EngHlf.HlfName}: {eng.Count}
    {LngHlf.HlfName}: {lng.Count}".Trace();
				return false;
			}

			var result = true;

			foreach (var comp in eng.Zip(lng, (e, l) => (eng: e, lng: l))
				.Select((c, i) => (index: i, eng: (ent: c.eng, elem: element(c.eng)), lng: (ent: c.lng, elem: element(c.lng))))
				.Where(c => !c.eng.elem.Equals(c.lng.elem)))
			{
				$@"Mismatch: {mismatchKind}:
    {EngHlf.HlfName}({lineNumber(comp.eng.ent)}): {comp.eng.elem}
    {LngHlf.HlfName}({lineNumber(comp.lng.ent)}): {comp.lng.elem}".Trace();

				result = false;
			}

			return result;
		}
	}
}
