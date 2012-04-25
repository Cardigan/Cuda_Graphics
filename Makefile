all:
	nvcc -g -G -pg -arch=compute_20 -code=sm_20 Image.cpp MeshParser_release3.cu  -o rasterize
clean:
	rm rasterize
