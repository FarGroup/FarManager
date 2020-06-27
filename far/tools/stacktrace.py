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


def show_stack():
	BaseAddress = None
	map_data = dict()

	with open(sys.argv[1], encoding="utf-8") as map_file:
		for line in map_file:
			if BaseAddress is None:
				m = re.search(r"^ +Preferred load address is ([0-9A-Fa-f]+)\s+$", line)
				if m is not None:
					BaseAddress = int(m.group(1), 16)
					continue

			m = re.search(r"^ +([0-9A-Fa-f]+):([0-9A-Fa-f]+) +([^ ]+) +([0-9A-Fa-f]+) .+ ([^ ]+)\s+$", line)
			if m is not None:
				map_data[int(m.group(4), 16) - BaseAddress] = (m.group(3), m.group(5))

	keys = sorted(map_data.keys())

	for i in sys.argv[2:]:
		address = int(i, 16)
		key_index = bisect.bisect(keys, address)
		if keys[key_index] > address:
			key_index -= 1;
		data = map_data[keys[key_index]]
		print("{0}: {1:X}+{2:04X} {3} ({4})".format(i, keys[key_index], address - keys[key_index], undecorate(data[0]), data[1]))


if __name__ == "__main__":
	show_stack()
