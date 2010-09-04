all:
	gcc -g main.c screenshoteditor.c screenshoteditor-helpers.c -o screensnap `pkg-config --libs --cflags gtk+-2.0 cairo`
buildrun:
	gcc -g main.c screenshoteditor.c screenshoteditor-helpers.c-o screensnap `pkg-config --libs --cflags gtk+-2.0 cairo`
	./easycapture
