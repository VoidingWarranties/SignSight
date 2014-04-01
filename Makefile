CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

build/SignSight : src/SignSight.cpp
	g++ $(CFLAGS) $(LIBS) -Wall -Wextra -o $@ $< src/GUI/GUI.cpp src/GUI/Draw.cpp src/ImageProcessing/ImageProcessing.cpp -I src/
