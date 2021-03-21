print(""".Language=English,English
.Options CtrlColorChar=\\

@Contents=RGB
@RGB""")

x_step = 16
y_step = 16

def print_color(r, g, b):
	print(r"\(T0:T{:02X}{:02X}{:02X})".format(r, g, b))

def line(color, light):
	def out(r, g, b):
		def with_light(x):
			return max(0, min(int(x + light), 255))

		print_color(
			with_light(r),
			with_light(g),
			with_light(b)
		)

	print(" ", end='')

	r = (color & 0xFF0000) >> 16
	g = (color & 0x00FF00) >> 8
	b = color & 0x0000FF

	out(128, 128, 128)
	out(r, g, b)

	limit = int(256 / x_step)

	for i in range(0, limit):
		g += x_step
		out(r, g, b)

	for i in range(0, limit):
		r -= x_step
		out(r, g, b)

	for i in range(0, limit):
		b += x_step
		out(r, g, b)

	for i in range(0, limit):
		g -= x_step
		out(r, g, b)

	for i in range(0, limit):
		r += x_step
		out(r, g, b)

	for i in range(0, limit):
		b -= x_step
		out(r, g, b)

	print(r"\-")

def lines():
	color = 0xff0000

	line(color, 0)

	limit = int(256 / y_step)

	for i in range(limit, 0, -1):
		line(color, i * y_step)

	for i in range(0, -limit, -1):
		line(color, i * y_step)

if __name__ == "__main__":
	lines()
