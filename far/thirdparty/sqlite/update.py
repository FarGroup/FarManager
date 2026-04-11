import csv
import io
import os.path
import re
import urllib.request
import zipfile

BASE_URL = "https://sqlite.org/"

def fetch_url(url):
	with urllib.request.urlopen(url) as response:
		return response.read()


def extract_csv_from_html(html):
	pattern = re.compile(r"<!--\s*Download product data for scripts to read\s*(.*?)-->", re.DOTALL)
	match = pattern.search(html)
	if not match:
		raise RuntimeError("CSV data block not found in HTML")

	return match.group(1).strip()


def main():
	print("Fetching download page")
	html_bytes = fetch_url(f"{BASE_URL}download.html")
	html = html_bytes.decode()

	print("Extracting CSV data from HTML comment")
	csv_text = extract_csv_from_html(html)

	reader = csv.DictReader(csv_text.splitlines())

	print("Searching for sqlite-amalgamation ZIP")
	selected = None

	for row in reader:
		rel_url = row["RELATIVE-URL"]
		if "sqlite-amalgamation" in rel_url and rel_url.endswith(".zip"):
			selected = row
			break

	if not selected:
		raise RuntimeError("No sqlite-amalgamation ZIP found")

	zip_url = BASE_URL + selected["RELATIVE-URL"]
	print(f"Found ZIP")
	print(f"  URL:     {zip_url}")
	print(f"  Version: {selected['VERSION']}")
	print(f"  Size:    {selected['SIZE-IN-BYTES']} bytes")

	print("Downloading ZIP file")
	zip_bytes = fetch_url(zip_url)

	print("Extracting ZIP")
	with zipfile.ZipFile(io.BytesIO(zip_bytes)) as zf:
		root = zf.namelist()[0]
		for name in ("sqlite3.c", "sqlite3.h"):
				print(f"Extracting {name}")
				with zf.open(root + name) as src, open(name, "wb") as dst:
					dst.write(src.read())
	
	print("Done")


if __name__ == "__main__":
	main()
