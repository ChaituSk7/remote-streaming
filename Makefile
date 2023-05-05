CC = g++
path = src
header = ./include/
LIBS = `pkg-config --cflags --libs gstreamer-1.0 gstreamer-pbutils-1.0`

all: hostsrc.o hostmp3.o hostwebm.o hostavi.o metadata.o hostsrc.so hostmp3.so hostwebm.so hostavi.so metadata.so exe

hostsrc.o: $(path)/hostsrc.cpp
	$(CC) -c $(path)/hostsrc.cpp $(LIBS) -fPIC -I $(header)

hostmp3.o:	$(path)/hostmp3.cpp
	$(CC) -c $(path)/hostmp3.cpp $(LIBS) -fPIC -I $(header)

hostwebm.o: $(path)/hostwebm.cpp
	$(CC) -c $(path)/hostwebm.cpp $(LIBS) -fPIC -I $(header)

hostavi.o: $(path)/hostavi.cpp
	$(CC) -c $(path)/hostavi.cpp $(LIBS) -fPIC -I $(header)

metadata.o: $(path)/metadata.cpp 
	$(CC) -c $(path)/metadata.cpp $(LIBS) -fPIC -I $(header)

hostsrc.so:	hostsrc.o
	$(CC) -shared -o libhostsrc.so hostsrc.o $(LIBS)

hostmp3.so:	hostmp3.o
	$(CC) -shared -o libhostmp3.so hostmp3.o $(LIBS)

hostwebm.so: hostwebm.o
	$(CC) -shared -o libhostwebm.so hostwebm.o $(LIBS)

hostavi.so: hostavi.o
	$(CC) -shared -o libhostavi.so hostavi.o $(LIBS)

metadata.so: metadata.o
	$(CC) -shared -o libmetadata.so metadata.o $(LIBS)

exe: $(path)/main.cpp 
	$(CC) -o exe $(path)/main.cpp -lhostsrc -lhostmp3 -lhostwebm -lhostavi -lmetadata $(LIBS) -I $(header) -L .

clean:
	rm -rf *.o *.so exe
