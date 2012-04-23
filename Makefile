all:
	g++ Image.cpp MeshParser_release3.cpp  -o rasterize
clean:
	rm rasterize
