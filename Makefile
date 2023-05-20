compile:
	g++ -w -g Source/EntryPoint.cpp Source/Private*.cpp -o Sphere360.bin -ISource/Public -lGLEW -lglfw3 -lavcodec -lavformat -lavutil -lswscale -lGL