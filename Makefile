all:
	gcc -g main.c screenshoteditor.c -o easycapture `pkg-config --libs --cflags gtk+-2.0 cairo`
buildrun:
	gcc -g main.c screenshoteditor.c -o easycapture `pkg-config --libs --cflags gtk+-2.0 cairo`
	./easycapture
