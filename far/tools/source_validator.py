import sys
import os
import os.path
import re

def check(filename):
	#print(filename)
	with open(filename, encoding="utf-8") as f:
		content = f.read().splitlines()
		basename = os.path.basename(filename)
		name, extension = os.path.splitext(basename)
		UuidRe = re.compile('_[0-9A-F]{8}_[0-9A-F]{4}_[0-9A-F]{4}_[0-9A-F]{4}_[0-9A-F]{12}\Z', re.I)
		LineNumber = 0

		IsBom = content[0][0] == '\ufeff'
		if IsBom:
			content[0] = content[0][1:]

		CheckBom = True
		CheckIncludeGuards = True

		if content[0].startswith("// validator: "):
			if "no-bom" in content[0]:
				CheckBom = False
			if "no-include-guards" in content[0]:
				CheckIncludeGuards = False
			content = content[1:]

		def Raise():
			raise Exception(name + extension, LineNumber, content[LineNumber])

		if CheckBom and not IsBom:
			Raise()

		if extension in [".hpp", ".h"] and CheckIncludeGuards:
			LockGuardName = "{0}_{1}".format(name.upper(), extension[1:].upper())
			LockGuardTemplate1 = "#ifndef " + LockGuardName
			LockGuardTemplate2 = "#define " + LockGuardName
			LockGuardTemplate3 = "#endif // " + LockGuardName

			def CheckLine(Line, Template):
				return Line.startswith(Template) and UuidRe.match(Line[len(Template):])

			if not (CheckLine(content[0], LockGuardTemplate1) and CheckLine(content[1], LockGuardTemplate2) and CheckLine(content[-1], LockGuardTemplate3)):
				Raise()
			LineNumber = 2

			if content[LineNumber] != "#pragma once":
				Raise()
			LineNumber += 1

			if content[LineNumber] != "":
				Raise()
			LineNumber += 1

		if content[LineNumber] != "/*":
			Raise()
		LineNumber += 1

		if content[LineNumber] != basename:
			Raise()
		LineNumber += 1;

		while content[LineNumber] != "*/":
			LineNumber += 1
		LineNumber += 1

		if content[LineNumber] != "/*":
			Raise()
		LineNumber += 1

		if not content[LineNumber].startswith("Copyright"):
			Raise()
		LineNumber += 1

		if content[LineNumber].startswith("Copyright"):
			LineNumber += 1

		if content[LineNumber] != "All rights reserved.":
			Raise()
		LineNumber += 1

		License = [
"",
"Redistribution and use in source and binary forms, with or without",
"modification, are permitted provided that the following conditions",
"are met:",
"1. Redistributions of source code must retain the above copyright",
"   notice, this list of conditions and the following disclaimer.",
"2. Redistributions in binary form must reproduce the above copyright",
"   notice, this list of conditions and the following disclaimer in the",
"   documentation and/or other materials provided with the distribution.",
"3. The name of the authors may not be used to endorse or promote products",
"   derived from this software without specific prior written permission.",
"",
"THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR",
"IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES",
"OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.",
"IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,",
"INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT",
"NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,",
"DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY",
"THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT",
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF",
"THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."]

		LicenseException = [
"EXCEPTION:",
"Far Manager plugins that use this header file can be distributed under any",
"other possible license with no implications from the above license on them."]

		LicI = 0
		while LicI < len(License):
			if content[LineNumber] != License[LicI]:
				Raise()
			LicI +=1
			LineNumber += 1

		if extension in [".hpp", ".h"] and content[LineNumber] == "":
			LineNumber += 1
			LicI = 0
			while LicI < len(LicenseException):
				if content[LineNumber] != LicenseException[LicI]:
					Raise()
				LicI +=1
				LineNumber += 1
		if content[LineNumber] != "*/":
			Raise()
		LineNumber += 1

		if content[LineNumber] != "":
			Raise()
		LineNumber += 1


def get_list(dir):
	return [ os.path.join(dir, f) for f in os.listdir(dir)]

if __name__ == "__main__":
	extensions = ('.cpp', '.hpp', '.c', '.h')
	files = get_list(".") + get_list("common") + get_list("sdk")
	for file in files:
		ext = os.path.splitext(file)[-1].lower()
		if ext in extensions:
			try:
				check(file)
			except Exception as e:
				print(e)
