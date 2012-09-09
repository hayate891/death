X11_LIBDIR := /usr/X11R6/lib
X11_INCDIR := /usr/X11R6/include

prog: prog.c
	${CC} -Wall -Werror -pedantic-errors -I${X11_INCDIR} -L${X11_LIBDIR} -lX11 -D_BSD_SOURCE $< -o $@