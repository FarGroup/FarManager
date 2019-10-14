import sys

def patch_feed(feed_file_name, lng_id, new_lines):
	lines = []
	with open(feed_file_name, "r", encoding='utf-8-sig') as feed_file:
		lines = feed_file.readlines();

	for i in new_lines:
		try:
			id = lines.index(i + '\n')
		except:
			print(i + " not in list")
			continue

		id += 1

		while not (lines[id].startswith('"') or lines[id].startswith("upd:")):
			id += 1

		id += lng_id
		lines[id] = new_lines[i] + '\n'

	with open(feed_file_name, "w", encoding='utf-8-sig') as feed_file:
		feed_file.writelines(lines);


def read_summary(summary_file_name):
	lines = {}
	with open(summary_file_name, "r", encoding='utf-8-sig') as summary_file:
		for l in summary_file.readlines():
			ls = l.split('|')
			lines[ls[0].strip()] = ls[1].strip()
	return lines


if __name__=="__main__":
	new_lines = read_summary(sys.argv[1])
	patch_feed(sys.argv[2], int(sys.argv[3]), new_lines)
