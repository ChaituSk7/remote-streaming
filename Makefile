CC = g++
path = src
header = ./include/
LIBS = `pkg-config --cflags --libs gstreamer-1.0 gstreamer-pbutils-1.0`

all: hostmp4.o hostmp3.o hostwebm.o hostavi.o metadata.o padprobe.o keyboardhandler.o demo.o thumbnail.o img_server.o hostmp4.so hostmp3.so hostwebm.so hostavi.so metadata.so padprobe.so keyboard.so demo.so thumbnail.so img_server.so exe

hostmp4.o: $(path)/hostmp4.cpp
	$(CC) -c $(path)/hostmp4.cpp $(LIBS) -fPIC -I $(header)

hostmp3.o:	$(path)/hostmp3.cpp
	$(CC) -c $(path)/hostmp3.cpp $(LIBS) -fPIC -I $(header)

hostwebm.o: $(path)/hostwebm.cpp
	$(CC) -c $(path)/hostwebm.cpp $(LIBS) -fPIC -I $(header)

hostavi.o: $(path)/hostavi.cpp
	$(CC) -c $(path)/hostavi.cpp $(LIBS) -fPIC -I $(header)

metadata.o: $(path)/metadata.cpp 
	$(CC) -c $(path)/metadata.cpp $(LIBS) -fPIC -I $(header)

padprobe.o: $(path)/padprobe.cpp 
	$(CC) -c $(path)/padprobe.cpp $(LIBS) -fPIC -I $(header)

keyboardhandler.o: $(path)/keyboardhandler.cpp
	$(CC) -c $(path)/keyboardhandler.cpp $(LIBS) -fPIC -I $(header)

demo.o:	$(path)/demo.cpp
	$(CC) -c $(path)/demo.cpp $(LIBS) -fPIC -I $(header)

thumbnail.o: $(path)/thumbnail.cpp
	$(CC) -c $(path)/thumbnail.cpp $(LIBS) -fPIC -I $(header)

img_server.o: $(path)/img_server.cpp
	$(CC) -c $(path)/img_server.cpp $(LIBS) -fPIC -I $(header)

hostmp4.so:	hostmp4.o
	$(CC) -shared -o libhostmp4.so hostmp4.o $(LIBS)

hostmp3.so:	hostmp3.o
	$(CC) -shared -o libhostmp3.so hostmp3.o $(LIBS)

hostwebm.so: hostwebm.o
	$(CC) -shared -o libhostwebm.so hostwebm.o $(LIBS)

hostavi.so: hostavi.o
	$(CC) -shared -o libhostavi.so hostavi.o $(LIBS)

metadata.so: metadata.o
	$(CC) -shared -o libmetadata.so metadata.o $(LIBS)

padprobe.so: padprobe.o 
	$(CC) -shared -o libpadprobe.so padprobe.o $(LIBS)

keyboard.so: keyboardhandler.o 
	$(CC) -shared -o libkeyboard.so keyboardhandler.o metadata.o $(LIBS)

demo.so: demo.o
	$(CC) -shared -o libdemo.so demo.o $(LIBS)

thumbnail.so: thumbnail.o
	$(CC) -shared -o libthumbnail.so thumbnail.o $(LIBS)

img_server.so: img_server.o
	$(CC) -shared -o libimg_server.so img_server.o $(LIBS)

exe: main/main.cpp 
	$(CC) -o exe main/main.cpp -lhostmp4 -lhostmp3 -lhostwebm -lhostavi -lmetadata -lpadprobe -lkeyboard -ldemo -lthumbnail -limg_server $(LIBS) -I $(header) -L .

clean:
	rm -rf *.o *.so exe *.jpg
