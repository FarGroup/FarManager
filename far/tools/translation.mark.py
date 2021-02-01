import sys

def check(in_filename, out_filename):
	with open(in_filename, encoding="utf-8") as f:
		RawContent = f.read()
		content = RawContent.splitlines()

		langs_index = list.index(content, "#number of languages") + 1;
		lang_count = int(content[langs_index])

		i = langs_index + lang_count

		while i < len(content):
			while not content[i].startswith("M"):
				i += 1

			while not content[i].startswith('"'):
				i += 1

			en_str = content[i + 1]

			l = 0
			while l != lang_count:
				if l == 1:
					l += 1
					continue
				s = content[i + l]
				if s.startswith("upd:"):
					s = s[4:]
				if s == en_str and s != '""':
					content[i + l] = "upd:" + s
				l += 1

			i += lang_count


		with open(out_filename, "w", encoding='utf-8') as outFile:
			for line in content:
				outFile.write(line + '\n')


if __name__ == "__main__":
	check(sys.argv[1], sys.argv[2])
