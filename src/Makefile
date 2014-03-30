CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

SignSight : SignSight.cpp
	g++ $(CFLAGS) $(LIBS) -o $@ $<
