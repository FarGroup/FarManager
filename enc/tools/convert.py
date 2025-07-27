#!/usr/bin/env python3

import sys

def postprocess(data):
	tokens = {
		"ENC_URL": "ms-its:FarEncyclopedia.ru.chm::/html"
	}

	for (k, v) in tokens.items():
		data = data.replace(f"$({k})", v)

	return data


def convert(from_filename, from_encoding, to_filename, to_encoding):
	data = None

	with open(from_filename, encoding=from_encoding) as in_file:
		data = in_file.read()

	data = postprocess(data)

	with open(to_filename, "w", encoding=to_encoding) as out_file:
		out_file.write(data)

if __name__=="__main__":
	convert(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
