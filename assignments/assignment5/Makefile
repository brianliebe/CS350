all: ssfs_mkdsk ssfs

ssfs_mkdsk: ssfs_mkdsk.cpp ssfs_file.h
	g++ ssfs_mkdsk.cpp -o ssfs_mkdsk

ssfs: ssfs.cpp
	g++ ssfs.cpp -o ssfs

clean:
	rm -f ssfs_mkdsk ssfs
