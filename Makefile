compile:
	g++ -w -g Source/EntryPoint.cpp Source/Private*.cpp -o Sphere360.bin -LLib -ISource/Public -lglew32 -lglfw3dll -lavcodec -lavformat -lavutil -lswscale -lopengl32