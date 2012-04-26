all:
	nvcc -g -G -pg --ptxas-options="-v" -arch=compute_20 -code=sm_20 MeshParser_release3.cu Image.cpp  -o rasterize
tst:
	nvcc MeshParser_release3.cu Image.cpp  -o rasterize
clean:
	rm rasterize
