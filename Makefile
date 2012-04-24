all:
	g++ Image.cpp MeshParser_release3.cpp -pg -g -o rasterize
clean:
	rm rasterize
