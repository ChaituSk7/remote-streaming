CC = g++
LIBS = `pkg-config --cflags --libs gstreamer-1.0 gstreamer-pbutils-1.0`

all: hostsrc.o hostmp3.o hostwebm.o hostavi.o metadata.o hostsrc.so hostmp3.so hostwebm.so hostavi.so metadata.so exe

hostsrc.o: hostsrc.cpp
	$(CC) -c hostsrc.cpp $(LIBS) -fPIC

hostmp3.o:	hostmp3.cpp
	$(CC) -c hostmp3.cpp $(LIBS) -fPIC

hostwebm.o: hostwebm.cpp
	$(CC) -c hostwebm.cpp $(LIBS) -fPIC

hostavi.o: hostavi.cpp
	$(CC) -c hostavi.cpp $(LIBS) -fPIC

metadata.o: metadata.cpp 
	$(CC) -c metadata.cpp $(LIBS) -fPIC

hostsrc.so:	hostsrc.o
	$(CC) -shared -o libhostsrc.so hostsrc.o $(LIBS)

hostmp3.so:	hostmp3.cpp
	$(CC) -shared -o libhostmp3.so hostmp3.o $(LIBS)

hostwebm.so: hostwebm.cpp
	$(CC) -shared -o libhostwebm.so hostwebm.o $(LIBS)

hostavi.so: hostavi.cpp
	$(CC) -shared -o libhostavi.so hostavi.o $(LIBS)

metadata.so: metadata.cpp 
	$(CC) -shared -o libmetadata.so metadata.o $(LIBS)

exe: main.cpp 
	$(CC) -o exe main.cpp -lhostsrc -lhostmp3 -lhostwebm -lhostavi -lmetadata $(LIBS) -I . -L .

clean:
	rm -rf *.o *.so exe
