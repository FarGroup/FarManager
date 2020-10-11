import bisect
import ctypes
import re
import sys

def undecorate(name):
	UNDNAME_32_BIT_DECODE = 0x800
	UNDNAME_NAME_ONLY = 0x1000
	flags = UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY
	buffer_size = 2048
	out = ctypes.create_string_buffer(buffer_size)
	return str(out.value, "utf-8") if ctypes.windll.dbghelp.UnDecorateSymbolName(ctypes.create_string_buffer(bytes(name, "utf-8")), out, buffer_size, flags) != 0 else name


def parse_vc(map_file, map_data):
	re_base = re.compile(r"^ +Preferred load address is ([0-9A-Fa-f]+)\s+$")
	re_symbol = re.compile(r"^ +([0-9A-Fa-f]+):([0-9A-Fa-f]+) +([^ ]+) +([0-9A-Fa-f]+) .+ ([^ ]+)\s+$")
	BaseAddress = None

	for line in map_file:
		if BaseAddress is None:
			m = re_base.search(line)
			if m is not None:
				BaseAddress = int(m.group(1), 16)
				continue

		m = re_symbol.search(line)
		if m is not None:
			map_data[int(m.group(4), 16) - BaseAddress] = (m.group(3), m.group(5))


def parse_clang(map_file, map_data):
	re_object = re.compile(r"^[0-9A-Fa-f]+ [0-9A-Fa-f]+ +[0-9]+         (.+)\s+$")
	re_symbol = re.compile(r"^([0-9A-Fa-f]+) [0-9A-Fa-f]+     0                 (.+)\s+$")
	objname = None

	for line in map_file:
		m = re_symbol.search(line)
		if m is not None:
			map_data[int(m.group(1), 16)] = (m.group(2), objname)
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
			map_data[int(m.group(1), 16) + BaseAddress] = (m.group(2), file_name)
			continue

		last_line = line


def show_stack():
	map_data = dict()

	with open(sys.argv[1], encoding="utf-8") as map_file:
		header = map_file.readline()
		map_file.seek(0)
		if header.startswith("Address  Size     Align Out     In      Symbol"):
			parse_clang(map_file, map_data)
		elif len(header.strip()) == 0:
			parse_gcc(map_file, map_data)
		else:
			parse_vc(map_file, map_data)

	keys = sorted(map_data.keys())

	for i in sys.argv[2:]:
		address = int(i, 16)
		key_index = bisect.bisect(keys, address)
		if keys[key_index] > address:
			key_index -= 1;
		data = map_data[keys[key_index]]
		print("{0}: {1:06X}+{2:04X} {3} ({4})".format(i, keys[key_index], address - keys[key_index], undecorate(data[0]), data[1]))


if __name__ == "__main__":
	show_stack()
