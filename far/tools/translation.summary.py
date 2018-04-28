def ignore(line):
	return line.startswith("m4_include") or line.startswith("#") or line.startswith("l:") or line.startswith("h:") or not len(line)


def print_summary(summary):
	for language, data in summary.items():
		with open(language.split()[1] + ".txt", "w", encoding='utf-8-sig') as outFile:
			keyLen = max(map(len, data))
			valueLen = max(map(len, data.values()))
			for item, str in data.items():
				print(item.ljust(keyLen), "|", str.ljust(valueLen), file = outFile)


def generate():
	with open(r"..\farlang.templ.m4", "r", encoding='utf-8-sig') as feedFile:
		lines = [l.rstrip() for l in feedFile.readlines() if not ignore(l.rstrip())]

	count = int(lines[1])
	languages = lines[2 : 2 + count]

	summary = {}

	for i, line in enumerate(lines):
		if line.startswith("upd:"):
			labelIndex = i - 1
			while not lines[labelIndex].startswith("M"):
				labelIndex = labelIndex - 1
			lngIndex = i - labelIndex - 1

			if languages[lngIndex] not in summary.keys():
				summary[languages[lngIndex]] = {}

			summary[languages[lngIndex]][lines[labelIndex]] = lines[i][4 :]

	print_summary(summary)


if __name__=="__main__":
	generate()
