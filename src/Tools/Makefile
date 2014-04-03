CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

../../build/ImageSegmenter : ImageSegmenter.cpp
	g++ $(CFLAGS) $(LIBS) -Wall -Wextra -o $@ $< ../GUI/GUI.cpp ../GUI/Draw.cpp ../ImageProcessing/ImageProcessing.cpp -I ../
