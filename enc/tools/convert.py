import sys

data = None

with open(sys.argv[1], encoding="utf-8-sig") as in_file:
	data = in_file.read()

with open(sys.argv[2], "w", encoding=sys.argv[3]) as out_file:
	out_file.write(data)
