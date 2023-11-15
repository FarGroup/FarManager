import argparse
import configparser
import os
import shutil
import sys
import tempfile
import zlib

class lang:
	full_filename = ""
	filename = ""
	name = ""
	desc = ""
	out_file = None
	crc = 0
	old_crc = 0
	need_update = 0


def unquote(s):
	return s[1:-1] if s.startswith('"') and s.endswith('"') else s


def crc32(file_name):
	with open(file_name, 'rb') as f:
		hash = 0
		while True:
			s = f.read(65536)
			if not s:
				break
			hash = zlib.crc32(s, hash)

	return hash & 0xFFFFFFFF

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("-i", help = "optional ini file with update status")
	parser.add_argument("-ol", help = "language files output path", required = True)
	parser.add_argument("-oh", help = "header file output path", required = True)
	parser.add_argument("-nc", action = "store_true")
	parser.add_argument("feed_file")
	args = parser.parse_args()

	with open(args.feed_file, "r", encoding="utf-8-sig") as feed_file:
		data = feed_file.readlines()

	index = 0

	def skip(i):
		while i != len(data):
			s = data[i].strip()
			if not len(s) or s.startswith("#"):
				i += 1
				continue
			return i

	def take():
		nonlocal index
		index = skip(index)
		s = data[index].strip()
		index += 1
		return s

	def take_comment(name):
		nonlocal index
		ss = None
		index = skip(index)
		while index != len(data):
			s = data[index].strip()
			if not s.startswith(name):
				return ss

			s = s[len(name):]
			index += 1

			if ss is None:
				ss = [s]
			else:
				ss.append(s)


	hdr_name = take()
	lang_count = int(take())

	if lang_count == 0:
		raise Exception("No languages to process")

	hdr_fullname = os.path.join(args.oh, hdr_name)

	old_crc = 0
	config = configparser.ConfigParser()

	if args.i is not None:
		config.read(args.i)

	def get_ini(section):
		return int(config.get(section, "CRC32", fallback = 0))

	def set_ini(section, value):
		if not section in config:
			config[section] = {}
		config[section]["CRC32"] = str(value)

	old_crc = get_ini(hdr_fullname) if os.path.isfile(hdr_fullname) else 0

	hdr_file = tempfile.NamedTemporaryFile(mode = "w", encoding = "utf-8-sig", delete = False)

	langs = [lang() for i in range(lang_count)]

	for l in langs:
		l.filename, l.name, l.desc = [unquote(i) for i in take().split(" ", 2)]

		l.full_filename = os.path.join(args.ol, l.filename)

		if args.i is not None and os.path.isfile(l.full_filename):
			l.old_crc = get_ini(l.full_filename)

		l.out_file = tempfile.NamedTemporaryFile(mode = "w", encoding = "utf-8-sig", delete = False)
		l.out_file.write(f".Language={l.name},{l.desc}\n\n")

	c_hhead = take_comment("hhead:")
	if c_hhead is not None:
		hdr_file.write(c_hhead)

	c_htail = take_comment("htail:")

	c_enum = take_comment("enum:")
	hdr_file.write(f"enum {c_enum[0]}\n{{\n")

	while index != len(data):
		# Unused
		c_h = take_comment("h:")
		# Unused
		c_he = take_comment("he:")

		msg_id = take()

		c_l = take_comment("l:")
		# Unused
		c_le = take_comment("le:")

		if c_h is not None:
			hdr_file.write("{}\n".format("\n".join(c_h)))

		if c_l is not None and msg_id != "MYes":
			for i in c_l:
				hdr_file.write(f"\t{i}\n" if len(i) else "\n")

		hdr_file.write(f"\t{msg_id},\n")

		if c_he is not None:
			hdr_file.write("\n".join(c_he))

		for l in langs:
			msg = take()

			if c_l is not None:
				l.out_file.write("{}\n".format("\n".join(c_l)))

			if msg.startswith("upd:"):
				msg = msg[4:]
				l.out_file.write("// need translation:\n")
				l.need_update += 1

			l.out_file.write(f"//[{msg_id}]\n{msg}\n")


	hdr_file.write("};\n")

	if c_htail is not None:
		hdr_file.write(c_htail)

	def finalize(out_file, full_name, old_crc):
		out_file.close()
		update = True

		new_crc = -1
		if args.i is not None:
			new_crc = crc32(out_file.name)
		if new_crc == old_crc:
			update = False
		else:
			set_ini(full_name, new_crc)
	
		if update:
			shutil.move(out_file.name, full_name)

		if os.path.exists(out_file.name):
			os.remove(out_file.name)


	for l in langs:
		finalize(l.out_file, l.full_filename, l.crc)
		if l.need_update:
			print(f"INFO: There are {l.need_update} strings that require review in {l.name} translation")

	finalize(hdr_file, hdr_fullname, old_crc)

	if args.i is not None:
		with open(args.i, 'w') as configfile:
			config.write(configfile)

if __name__ == '__main__':
	main()
