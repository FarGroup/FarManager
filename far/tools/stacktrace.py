import bisect
import ctypes
import os
import re
import sys


def undecorate(name):
	UNDNAME_NO_FUNCTION_RETURNS = 0x0004
	UNDNAME_NO_ALLOCATION_MODEL = 0x0008
	UNDNAME_NO_ALLOCATION_LANGUAGE = 0x0010
	UNDNAME_NO_ACCESS_SPECIFIERS = 0x0080
	UNDNAME_NO_MEMBER_TYPE = 0x0200
	UNDNAME_32_BIT_DECODE = 0x800
	UNDNAME_NAME_ONLY = 0x1000

	flags =\
		UNDNAME_NO_FUNCTION_RETURNS |\
		UNDNAME_NO_ALLOCATION_MODEL |\
		UNDNAME_NO_ALLOCATION_LANGUAGE |\
		UNDNAME_NO_ACCESS_SPECIFIERS |\
		UNDNAME_NO_MEMBER_TYPE |\
		UNDNAME_32_BIT_DECODE;

	buffer_size = 2048
	out = ctypes.create_string_buffer(buffer_size)
	return str(out.value, "utf-8") if ctypes.windll.dbghelp.UnDecorateSymbolName(ctypes.create_string_buffer(bytes(name, "utf-8")), out, buffer_size, flags) != 0 else name


def parse_mini(map_file, map_data):
	re_object = re.compile(r"^(\d+) (.+)\s+$")
	re_symbol = re.compile(r"^([0-9A-Fa-f]+) (\d+) (.+)\s+$")

	obj_names = {}
	files_seen = False
	symbols_seen = False

	for line in map_file:
		if len(line) == 0:
			continue

		if not files_seen:
			files_seen = line.startswith("FILES")

		if not symbols_seen:
			symbols_seen = line.startswith("SYMBOLS")

		if files_seen and not symbols_seen:
			m = re_object.search(line)
			if m is not None:
				obj_names.setdefault(int(m.group(1)), m.group(2))
				continue;

		if symbols_seen:
			m = re_symbol.search(line)
			if m is not None:
				Address = int(m.group(1), 16);
				file_name = obj_names[int(m.group(2))]
				map_data.setdefault(Address, (m.group(3), file_name))


def parse_vc(map_file, map_data):
	re_base = re.compile(r"^ +Preferred load address is ([0-9A-Fa-f]+)\s+$")
	re_symbol = re.compile(r"^ +[0-9A-Fa-f]+:[0-9A-Fa-f]+ +([^ ]+) +([0-9A-Fa-f]+) .+ ([^ ]+)\s+$")
	BaseAddress = None

	for line in map_file:
		if BaseAddress is None:
			m = re_base.search(line)
			if m is not None:
				BaseAddress = int(m.group(1), 16)
				continue

		m = re_symbol.search(line)
		if m is not None:
			Address = int(m.group(2), 16);
			if Address >= BaseAddress:
				Address -= BaseAddress
			map_data.setdefault(Address, (m.group(1), m.group(3)))


def parse_clang(map_file, map_data):
	re_object = re.compile(r"^[0-9A-Fa-f]+ [0-9A-Fa-f]+ +[0-9]+         (.+):(.+)\s+$")
	re_symbol = re.compile(r"^([0-9A-Fa-f]+) [0-9A-Fa-f]+     0                 (.+)\s+$")
	objname = None

	for line in map_file:
		m = re_symbol.search(line)
		if m is not None:
			map_data.setdefault(int(m.group(1), 16), (m.group(2), objname))
			continue

		m = re_object.search(line)
		if m is not None:
			objname = m.group(1)


def parse_gcc(map_file, map_data):
	re_file = re.compile(r"^File \s+$")
	re_file_name = re.compile(r"^\[ *[0-9]+\]\(.+\)\(.+\)\(.+\)\(.+\) \(nx 1\) 0x[0-9A-Fa-f]+ (.+)\s+$")
	re_symbol = re.compile(r"^\[ *[0-9]+\]\(.+\)\(.+\)\(.+\)\(.+\) \(nx 0\) 0x([0-9A-Fa-f]+) (.+)\s+$")
	BaseAddress = 0x1000
	last_line = None
	file_name = None

	for line in map_file:
		m = re_file.search(line)
		if m is not None:
			m = re_file_name.search(last_line)
			if m is not None:
				file_name = m.group(1)
				last_line = None
				continue

		m = re_symbol.search(line)
		if m is not None:
			map_data.setdefault(int(m.group(1), 16) + BaseAddress, (m.group(2), file_name))
			continue

		last_line = line


def read_map(map_file_name):
	map_data = dict()

	with open(map_file_name, 'r', encoding="utf-8") as map_file:
		header = map_file.readline()
		map_file.seek(0)
		if header.startswith("MINIMAP"):
			parse_mini(map_file, map_data)
		elif header.startswith("Address  Size     Align Out     In      Symbol"):
			parse_clang(map_file, map_data)
		elif len(header.strip()) == 0:
			parse_gcc(map_file, map_data)
		else:
			parse_vc(map_file, map_data)

	return sorted(map_data.keys()), map_data


def show_stack(map_file_name, frames):
	keys, map_data = read_map(map_file_name)

	for i in frames:
		address = int(i, 16)
		key_index = bisect.bisect(keys, address)
		if keys[key_index] > address:
			key_index -= 1;
		data = map_data[keys[key_index]]
		print("{0}: {1}+{2:X} ({3})".format(i, undecorate(data[0]), address - keys[key_index], data[1]))


def compact_map(map_file_name):
		keys, map_data = read_map(map_file_name)

		tmp_name = map_file_name + ".tmp"
		with open(tmp_name, 'w', encoding="utf-8") as out_file:
			out_file.write("MINIMAP\n")

			files = set()
			for key in keys:
				data = map_data[key]
				file = data[1]
				files.add(file)

			out_file.write("\nFILES\n")
			files_map = {}
			for index, file in enumerate(sorted(files)):
				files_map[file] = index
				out_file.write(f"{index} {file}\n")

			out_file.write("\nSYMBOLS\n")
			for key in keys:
				data = map_data[key]
				out_file.write(f"{key:X} {files_map[data[1]]} {data[0]}\n")

		os.replace(tmp_name, map_file_name)


def compact_maps(directory_name):
	for root, dirnames, filenames in os.walk(directory_name):
		for filename in filenames:
			if filename.lower().endswith(".map"):
				compact_map(os.path.join(root, filename))


def main():
	if len(sys.argv) == 2:
		compact_maps(sys.argv[1])
	else:
		show_stack(sys.argv[1], sys.argv[2:])

if __name__ == "__main__":
	main()
