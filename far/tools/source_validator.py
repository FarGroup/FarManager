import sys
import os
import os.path
import re
import traceback

class StyleException(Exception):
	def __init__(self, file, line, content, message):
		self.file = file
		self.line = line
		self.content = content
		self.message = message

	def __str__(self):
		return f"{self.file}({self.line}): {self.message}: {self.content}"

NotTabRe = re.compile('[^\t]')

def check(filename):
	#print(filename)
	with open(filename, encoding="utf-8") as f:
		RawContent = f.read()
		content = RawContent.splitlines()
		basename = os.path.basename(filename)
		name, extension = os.path.splitext(basename)
		UuidRe = re.compile('_[0-9A-F]{8}_[0-9A-F]{4}_[0-9A-F]{4}_[0-9A-F]{4}_[0-9A-F]{12}\Z', re.I)
		LineNumber = 0

		IsBom = content[0][0] == '\ufeff'
		if IsBom:
			content[0] = content[0][1:]

		CheckBom = True
		CheckIncludeGuards = True
		CheckSelfInclude = True

		if content[0].startswith("// validator: "):
			if "no-bom" in content[0]:
				CheckBom = False
			if "no-include-guards" in content[0]:
				CheckIncludeGuards = False
			if "no-self-include" in content[0]:
				CheckSelfInclude = False
			content = content[1:]

		def Raise(Message, Line=-1):
			RealLine = LineNumber if Line==-1 else Line
			raise StyleException(filename, RealLine + 1, content[RealLine], Message)

		if CheckBom and not IsBom:
			Raise("No BOM")

		if content[-1] == "":
			Raise("Too many empty lines", len(content) - 1)

		if RawContent[-1][-1] not in ['\r', '\n']:
			Raise("No final EOL", len(content) - 1)

		if extension in [".hpp", ".h"] and CheckIncludeGuards:
			LockGuardName = basename.upper().replace(".", "_")
			LockGuardTemplate1 = "#ifndef " + LockGuardName
			LockGuardTemplate2 = "#define " + LockGuardName
			LockGuardTemplate3 = "#endif // " + LockGuardName

			def CheckLine(Line, Template):
				return Line.startswith(Template) and UuidRe.match(Line[len(Template):])

			if not (CheckLine(content[0], LockGuardTemplate1) and CheckLine(content[1], LockGuardTemplate2) and CheckLine(content[-1], LockGuardTemplate3)):
				Raise("No include guard")
			LineNumber = 2

			if content[LineNumber] != "#pragma once":
				Raise("No #pragma once")
			LineNumber += 1

			if content[LineNumber] != "":
				Raise("No empty line")
			LineNumber += 1

		if content[LineNumber] != "/*":
			Raise("No comment")
		LineNumber += 1

		if content[LineNumber] != basename:
			Raise("Name mismatch")
		LineNumber += 1;

		while content[LineNumber] != "*/":
			LineNumber += 1
		LineNumber += 1

		if content[LineNumber] != "/*":
			Raise("No comment")
		LineNumber += 1

		if not content[LineNumber].startswith("Copyright"):
			Raise("No copyright")
		LineNumber += 1

		if content[LineNumber].startswith("Copyright"):
			LineNumber += 1

		if content[LineNumber] != "All rights reserved.":
			Raise("No copyright")
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
				Raise("License mismatch")
			LicI +=1
			LineNumber += 1

		if extension in [".hpp", ".h"] and content[LineNumber] == "":
			LineNumber += 1
			LicI = 0
			while LicI < len(LicenseException):
				if content[LineNumber] != LicenseException[LicI]:
					Raise("License mismatch")
				LicI +=1
				LineNumber += 1
		if content[LineNumber] != "*/":
			Raise("No comment")
		LineNumber += 1

		if content[LineNumber] != "":
			Raise("No empty line")
		LineNumber += 1

		if CheckSelfInclude and extension in [".cpp"]:
			if content[LineNumber] == "// BUGBUG" and content[LineNumber + 1] == "#include \"platform.headers.hpp\"" and content[LineNumber + 2] == "":
				LineNumber += 3

			if content[LineNumber] != "// Self:":
				Raise("No self comment")
			LineNumber += 1

			include = "#include "
			if not content[LineNumber].startswith(include):
				Raise("No include")

			incName, incExt = os.path.splitext(content[LineNumber][len(include):].strip('"'))
			if name != incName or extension != incExt.replace('h', 'c'):
				Raise("Corresponding header must be included first")
			LineNumber += 1

			if LineNumber < len(content) and content[LineNumber] != "":
				Raise("No empty line")
			LineNumber += 1

		for i, line in enumerate(content):
			TabEnd = NotTabRe.search(line)
			if TabEnd and '\t' in line[TabEnd.start():]:
				LineNumber = i
				Raise("Tabs formatting")

			if line.endswith(' ') or line.endswith('\t'):
				LineNumber = i
				Raise("Trailing whitespace")

def get_list(dir):
	return [ os.path.join(dir, f) for f in os.listdir(dir)]

if __name__ == "__main__":
	extensions = ('.cpp', '.hpp', '.c', '.h')
	files = get_list(".") + get_list("common") + get_list("common/2d") + get_list("platform.sdk")
	IssuesFound = False
	for file in files:
		ext = os.path.splitext(file)[-1].lower()
		if ext in extensions:
			try:
				check(file)
			except StyleException as e:
				IssuesFound = True
				print(e)

	sys.exit(IssuesFound)
