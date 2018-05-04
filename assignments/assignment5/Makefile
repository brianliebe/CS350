FLAGS=-std=c++11 -pthread -g

all: ssfs_mkdsk ssfs

ssfs_mkdsk: ssfs_mkdsk.cpp ssfs_file.h
	g++ ssfs_mkdsk.cpp -o ssfs_mkdsk $(FLAGS)

ssfs: ssfs.cpp
	g++ ssfs.cpp -o ssfs $(FLAGS)

clean:
	rm -f ssfs_mkdsk ssfs
