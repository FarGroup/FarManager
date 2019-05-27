# An ad hoc script to convert an lng file to the arclite format

import sys

def read_header():
	with open("../bootstrap/msg.h", encoding="utf-8") as f:
		RawContent = f.read()
		content = RawContent.splitlines()
		return [i[len("#define MSG_"):].split(" ") for i in content]


def read_lng(name):
	with open(name, encoding="utf-8") as f:
		RawContent = f.read()
		content = RawContent.splitlines()
		return content[0], filter(lambda x: x.startswith('"'), content)


def read_en():
	with open("../en.msg", encoding="utf-8") as f:
		RawContent = f.read()
		return RawContent.splitlines()


def to_key(string):
	return string.split("=")[0].strip().upper().replace(".", "_") + " = "

if __name__ == "__main__":
	hdr = read_header()
	lng_tuple = read_lng(sys.argv[1])
	lng = list(lng_tuple[1])

	if len(lng) != len(hdr):
		raise "Wrong number of lines"

	out = read_en()

	out[0] = lng_tuple[0]

	lines_replaced = 0

	for i in range(len(lng)):
		if int(hdr[i][1]) != i:
			raise "Wrong line number"

		begin = hdr[i][0] + " = "

		for n in range(len(out)):
			if to_key(out[n]).startswith(begin):
				out[n] = out[n][0:len(begin)] + lng[i][1:-1]
				lines_replaced += 1
				break

	print(lines_replaced)

	with open(sys.argv[2], "w", encoding="utf-8") as f:
		f.write("\n".join(out))
