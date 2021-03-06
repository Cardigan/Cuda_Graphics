//basic program to read in a mesh file (of .m format from H. Hoppe)
//Hh code modified by ZJW for csc 471
#include "Rasterizer.h"
#define ERRORCHECK   error = cudaGetLastError();\
					if(error != cudaSuccess){\
					printf("CUDA Error: %s\n", cudaGetErrorString(error));\
							\
					return;\
					}\

cudaError_t error;

//stl vector to store all the triangles in the mesh
vector<Tri *> Triangles;
//stl vector to store all the vertices in the mesh
vector<Vector3 *> Vertices;

//for computing the center point and extent of the model
Vector3 center;
float max_x, max_y, max_z, min_x, min_y, min_z;
float max_extent;

//other globals
int GW;
int GH;
int display_mode;
int view_mode;

//forward declarations of functions
void readLine(char* str);
void readStream(istream& is);
void drawTri(Tri * t);
void drawObjects();
void display();


//color_t* img_buffer2 = (color_t*)malloc(sizeof(color_t)*2000*2000);

Vector3 normalize(Vector3 v) {
  double mag = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); 
  if (mag > 0) {
   return Vector3(v.x/mag, v.y/mag, v.z/mag);
  }
  else {
   return Vector3(0,0,0);
  }
}

double clamp(double num, double floor, double ceiling) {
  if (num < floor) { return floor; }
  if (num > ceiling) { return ceiling; }
  return num;
}

double dot(Vector3 v1, Vector3 v2) {
  return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

double window_size = 1.0;

double p2wy(double y, int height) {
  return (((height - window_size) / 2) * y) + ((height-window_size)/2);
}

double p2wx(double x, int width) {
  return (((width - window_size) / 2) * x) + ((width-window_size)/2);
}

void Convert_to_Window(vector<Vector3 *> & Vertices, int width, int height, int scale, int x_offset, int y_offset) {

  for (int i = 0; i < Vertices.size(); i++) {
    (*(Vertices[i])).x = p2wx( (*(Vertices[i])).x*scale, width) + x_offset;
    (*(Vertices[i])).y = p2wy( ((*(Vertices[i])).y*scale), height) + y_offset;
  }

}

// Returns bounding 
int min_val(double x1, double x2, double x3) {
   double lowest = x1;
   if (x2 < lowest) {
      lowest = x2;
   }
   if (x3 < lowest) {
      lowest = x3;
   }
   return (int)lowest;
}

int max_val(double x1, double x2, double x3) {
   double highest = x1;
   if (x2 > highest) { highest = x2; }
   if (x3 > highest) { highest = x3; }
   return (int)highest+1;
}

int in_triangle(double alpha, double gamma, double beta) {
   return (alpha > -0.0) && (alpha < 1.01) && (gamma > -0.00) && (gamma < 1.01) && (beta > -0.0) && (beta < 1.01);
}

void RasterizeTriangles(vector<Tri *> & Triangles, vector<Vector3 *> & Vertices, int width, int height) {

   color_t* img_buffer = (color_t*)malloc(sizeof(color_t)*width*height);

   for (int i = 0; i < width*height; i++) {
    img_buffer[i].r = 0.0;
    img_buffer[i].g = 0.0;
    img_buffer[i].b = 0.0;
   }

   double* depth_buffer = (double*)malloc(sizeof(double)*width*height);

   // Initialize depth_buffer to low number;
   for (int i = 0; i < width*height; i++) {
      depth_buffer[i] = -100000;
   }

   Image img(width, height);

   //Iterate through each Triangle
   for (int i = 0; i < Triangles.size(); i++) {

      int t = Vertices[Triangles[i]->v1]->x;

      if (Triangles[i]->normal.z < 0) {

      }
      else {
         // Get minimum and maximum x
         int minx = min_val(Vertices[Triangles[i]->v1]->x, Vertices[Triangles[i]->v2]->x, Vertices[Triangles[i]->v3]->x);
         if (minx < 0) { minx = 0; }
         int maxx = max_val(Vertices[Triangles[i]->v1]->x, Vertices[Triangles[i]->v2]->x, Vertices[Triangles[i]->v3]->x);
         if (maxx + 1 > width ) { maxx = width-1; }

         // Get minimum and maximum y
         int miny = min_val(Vertices[Triangles[i]->v1]->y, Vertices[Triangles[i]->v2]->y, Vertices[Triangles[i]->v3]->y);
         if (miny < 0) { miny = 0; }
         int maxy = max_val(Vertices[Triangles[i]->v1]->y, Vertices[Triangles[i]->v2]->y, Vertices[Triangles[i]->v3]->y);
         if (maxy + 1 > width) { maxy = width-1; }

         for (int y = miny; y < maxy+1; y++) {
            for (int x = minx; x < maxx+1; x++) {
               // Calculate alpha, beta, gamma, 
               
               double A = (Vertices[Triangles[i]->v2]->x - Vertices[Triangles[i]->v1]->x) * (Vertices[Triangles[i]->v3]->y - Vertices[Triangles[i]->v1]->y)  ;
               A -= (Vertices[Triangles[i]->v3]->x - Vertices[Triangles[i]->v1]->x) * (Vertices[Triangles[i]->v2]->y - Vertices[Triangles[i]->v1]->y)   ;

               double beta =  (Vertices[Triangles[i]->v1]->x - Vertices[Triangles[i]->v3]->x) * (y - Vertices[Triangles[i]->v3]->y)  ;
               beta -=  (x - Vertices[Triangles[i]->v3]->x) * (Vertices[Triangles[i]->v1]->y - Vertices[Triangles[i]->v3]->y)  ;
               beta /= A;

               double gamma =  (Vertices[Triangles[i]->v2]->x - Vertices[Triangles[i]->v1]->x) * (y - Vertices[Triangles[i]->v1]->y)  ;
               gamma -= (x - Vertices[Triangles[i]->v1]->x) * (Vertices[Triangles[i]->v2]->y - Vertices[Triangles[i]->v1]->y)  ;
               gamma /= A;  

               double alpha = 1.0 - beta - gamma;

               // Is it inside?
               if (in_triangle(alpha, gamma, beta)) {
                  // Color
                  color_t col;
                  col.r = alpha * (Triangles[i]->c1.r) + beta * ( Triangles[i]->c2.r ) + gamma * ( Triangles[i]->c3.r  );
                  if (col.r > 1.0) { col.r = 1.0; }
                  if (col.g > 1.0) { col.g = 1.0; }
                  if (col.b > 1.0) { col.b = 1.0; }
                  col.g = alpha * (Triangles[i]->c1.g) + beta * ( Triangles[i]->c2.g ) + gamma * ( Triangles[i]->c3.g  );
                  col.b = alpha * (Triangles[i]->c1.b) + beta * ( Triangles[i]->c2.b ) + gamma * ( Triangles[i]->c3.b  );


                  // Is the depth higher than the current depth?
                     // Calculate interpolated z
                     float depth = alpha * (Vertices[Triangles[i]->v1]->z) + beta * ( Vertices[Triangles[i]->v2]->z) + gamma * (  Vertices[Triangles[i]->v3]->z );
                     if (depth > depth_buffer[x*width+y]) {
                        depth_buffer[x*width+y] = depth;
                        img_buffer[x*width+y] = col;
                     }
               }
            }
         }


      }

   }

  // Calculate statistics
  /*double r_diff = 0.0;
  double r_diff2 = 0.0;
  double g_diff = 0.0;
  double g_diff2 = 0.0;
  double b_diff = 0.0;
  double b_diff2 = 0.0;
   for (int i = 0; i < (2000*2000); i++) {
      // Get difference
      r_diff += img_buffer[i].r;
      r_diff2 += img_buffer2[i].r;
      g_diff += img_buffer[i].g;
      g_diff2 += img_buffer2[i].g;
      b_diff += img_buffer[i].b;
      b_diff2 += img_buffer2[i].b;
   }

   r_diff /= (2000.0*2000);
   g_diff /= (2000.0*2000);
   b_diff /= (2000.0*2000);
   r_diff2 /= (2000.0*2000);
   g_diff2 /= (2000.0*2000);
   b_diff2 /= (2000.0*2000);
   printf("Avg.\n R: %lf G: %lf B: %lf\n", r_diff, g_diff, b_diff);
   printf("Avg.\n R: %lf G: %lf B: %lf\n", r_diff2, g_diff2, b_diff2);
   free(img_buffer2);*/


   for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
         img.pixel(x,y, img_buffer[x*width+y]);
      }
   }

   img.WriteTga("awesome.tga", false);
   free(img_buffer);
   free(depth_buffer);
}

void ColorVertices1(vector<Tri *> & Triangles, vector<Vector3 *> & Vertices) {

  Vector3 light_pos = Vector3(0.2, 0.2, 0.2);
  double material = 0.9;
  Color light = Color(0.9, 0.2, 0.2);

  int size = Triangles.size();
  for (int i = 0; i < size; i++) {
    // Change indexing from 0 start to 1
    Triangles[i]->v1 = Triangles[i]->v1-1;
    Triangles[i]->v2 = Triangles[i]->v2-1;
    Triangles[i]->v3 = Triangles[i]->v3-1;


    int i1 = (*(Triangles[i])).v1;
    int i2 = (*(Triangles[i])).v2;
    int i3 = (*(Triangles[i])).v3;

    // Calculate and save the normal

    Vector3 v1 = Vector3(Vertices[i1]->x - Vertices[i2]->x,
                           Vertices[i1]->y - Vertices[i2]->y,
                           Vertices[i1]->z - Vertices[i2]->z);

    Vector3 v2 = Vector3( Vertices[i1]->x - Vertices[i3]->x,
                           Vertices[i1]->y - Vertices[i3]->y,
                           Vertices[i1]->z - Vertices[i3]->z
                           );



    Vector3 cross = Vector3();
    cross.x = v1.y*v2.z - v1.z*v2.y;
    cross.y = v1.z*v2.x - v1.x*v2.z;
    cross.z = v1.x*v2.y - v1.y*v2.x;
    if (cross.x < 0.0) { cross.x = 0.0; }
    if (cross.y < 0.0) { cross.y = 0.0; }
    Triangles[i]->normal = normalize(cross); 

  }

  printf("Done calculating normals.\n");
  for (int i = 0; i < Triangles.size(); i++) {
    int i1 = (*(Triangles[i])).v1;
    Tri tri = (*(Triangles[i]));
    Vector3 vertex = (*(Vertices[i1]));

    Vector3 to_light = normalize(Vector3(
                                  light_pos.x - vertex.x,
                                  light_pos.y- vertex.y,
                                  light_pos.z - vertex.z ));


    // Get color for vector 1
    Color color = Color();
    color.r = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.r;
    //color.r = Triangles[i]->normal.x;
    color.g = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.g;
    //color.g = Triangles[i]->normal.y;
    color.b = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.b;
    //color.b = Triangles[i]->normal.z;
    (*(Triangles[i])).c1 = color;

    int i2 = (*(Triangles[i])).v2;
    tri = (*(Triangles[i]));
    vertex = (*(Vertices[i2]));

    to_light = normalize(Vector3(
                                  light_pos.x - vertex.x,
                                  light_pos.y- vertex.y,
                                  light_pos.z - vertex.z ));


    // Get color for vector 1
    color = Color();
    color.r = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.r;
    //color.r = Triangles[i]->normal.x;
    color.g = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.g;
    //color.g = Triangles[i]->normal.y;
    color.b = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.b;
    //color.b = Triangles[i]->normal.z;
    (*(Triangles[i])).c2 = color;
    
    
    int i3 = (*(Triangles[i])).v3;
    tri = (*(Triangles[i]));
    vertex = (*(Vertices[i3]));

    to_light = normalize(Vector3(
                                  light_pos.x - vertex.x,
                                  light_pos.y- vertex.y,
                                  light_pos.z - vertex.z ));


    // Get color for vector 1
    color = Color();
    color.r = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.r;
    //color.r = Triangles[i]->normal.x;
    color.g = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.g;
    //color.g = Triangles[i]->normal.y;
    color.b = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * light.b;
    //color.b = Triangles[i]->normal.z;
    (*(Triangles[i])).c3 = color; 

  }

}

//open the file for reading
void ReadFile(char* filename) {
  
  printf("Reading coordinates from %s\n", filename);
  
  ifstream in_f(filename);
  if (!in_f)  {
    printf("Could not open file %s\n", filename);
  } else {
    readStream(in_f);
  }
  
}

//process the input stream from the file
void readStream(istream& is)
{
  char str[256];
  for (;is;) {
    is >> ws;
    is.get(str,sizeof(str));
    if (!is) break;
    is.ignore(9999,'\n');
    readLine(str);
  }
}

//process each line of input save vertices and faces appropriately
void readLine(char* str) {
  
  //int indx = 0;
  int vi;
  float x, y, z;
  float r, g, b;
  int mat;
  
  if (str[0]=='#') return;
  //read a vertex or face
  if (str[0]=='V' && !strncmp(str,"Vertex ",7)) {
    Vector3* v;
    if (sscanf(str,"Vertex %d %g %g %g",&vi,&x,&y,&z) !=4)
    {
      printf("an error occurred in reading vertices\n");
#ifdef _DEBUG
      exit(EXIT_FAILURE);
#endif
    }
    v = new Vector3(x, y, z);
    //store the vertex
    Vertices.push_back(v);
    //house keeping to display in center of the scene
    center.x += v->x;
    center.y += v->y;
    center.z += v->z;
    if (v->x > max_x) max_x = v->x; if (v->x < min_x) min_x = v->x;
    if (v->y > max_y) max_y = v->y; if (v->y < min_y) min_y = v->y;
    if (v->z > max_z) max_z = v->z; if (v->z < min_z) min_z = v->z;
  } else if (str[0]=='F' && !strncmp(str,"Face ",5)) {
    Tri* t;
    t = new Tri();
    char* s=str+4;
    int fi=-1;
    for (int t_i = 0;;t_i++) {
      while (*s && isspace(*s)) s++;
      //if we reach the end of the line break out of the loop
      if (!*s) break;
      //save the position of the current character
      char* beg=s;
      //advance to next space
      while (*s && isdigit(*s)) s++;
      //covert the character to an integer
      int j=atoi(beg);
      //the first number we encounter will be the face index, don't store it
      if (fi<0) { fi=j; continue; }
      //otherwise process the digit we've grabbed in j as a vertex index
      //the first number will be the face id the following are vertex ids
      if (t_i == 1)
        t->v1 = j;
      else if (t_i == 2)
        t->v2 = j;
      else if (t_i == 3)
        t->v3 = j;
      //if there is more data to process break out
      if (*s =='{') break;
    }
    //possibly process colors if the mesh has colors
    if (*s && *s =='{'){
      char *s1 = s+1;
      cout << "trying to parse color " << !strncmp(s1,"rgb",3) << endl;
      //if we're reading off a color
      if (!strncmp(s1,"rgb=",4)) {
        //grab the values of the string
        if (sscanf(s1,"rgb=(%g %g %g) matid=%d",&r,&g,&b,&mat)!=4)
        {
           printf("error during reading rgb values\n");
#ifdef _DEBUG
           exit(EXIT_FAILURE);
#endif
        }
        t->c1.r = r; t->c1.g = g; t->c1.b = b;

        cout << "set color to: " << r << " " << g << " " << b << endl;
      }
    }
    //store the triangle read in
    Triangles.push_back(t);
  }
}

//testing routine for parser - left in just as informative code about accessing data
void printFirstThree() {
  printf("first vertex: %f %f %f \n", Vertices[0]->x, Vertices[0]->y, Vertices[0]->z);
  printf("first face: %d %d %d \n", Triangles[0]->v1, Triangles[0]->v2, Triangles[0]->v3);    
  printf("second vertex: %f %f %f \n", Vertices[1]->x, Vertices[1]->y, Vertices[1]->z);
  //printf("second face: %d %d %d \n", Triangles[1]->v1, Triangles[1]->v2, Triangles[1]->v3);
  printf("third vertex: %f %f %f \n", Vertices[2]->x, Vertices[2]->y, Vertices[2]->z);
  //printf("third face: %d %d %d \n", Triangles[2]->v1, Triangles[2]->v2, Triangles[2]->v3);
}



//bluring cuda fucntion call
__global__ void cuBlur(cudaPixel * cuda_pixel_buffer,int width, int height){
	
	//float weight[5] = { 0.9, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };
	
	int index =  (blockIdx.x*THREADSPERBLOCK + threadIdx.x);
	//int lim = width * height;
	//int y = (blockIdx.x*THREADSPERBLOCK + threadIdx.x) / width ;
	//int x = (blockIdx.x*THREADSPERBLOCK + threadIdx.x) - y ;

  int x = index / width;
  int y = index % height;
	
	//main if statment to test if the thread is range
	if(index < (width*height)){

		for(int i=0;i<100;i++){
             float adds = 1;
             cudaPixel tmp;
			 tmp.r = 0.0;
             tmp.g = 0.0;
             tmp.b = 0.0;
             // Loop through five times
 
             for (int z = 0; z < 20; z+=2) {
                 if (y + z < height ) {
                    tmp.r += cuda_pixel_buffer[index+z].r;
                    tmp.g += cuda_pixel_buffer[index+z].g;
                    tmp.b += cuda_pixel_buffer[index+z].b;

                    adds += 1.0;
                 }
                 if (y - z > 0) {
                    tmp.r += cuda_pixel_buffer[index-z].r;
                    tmp.g += cuda_pixel_buffer[index-z].g;
                    tmp.b += cuda_pixel_buffer[index-z].b;
                    adds += 1.0;
                 }
             }
             //printf("Red is %f.\n", cuda_pixel_buffer[index].r);
             tmp.r /= adds;
             tmp.g /= adds;
             tmp.b /= adds;

             __syncthreads();
             cuda_pixel_buffer[index] = tmp;
             
             tmp.r = 0.0;
             tmp.g = 0.0;
             tmp.b = 0.0;

             adds = 0.0;

             // Loop through five times
             for (int z = 0; z < 20; z+=2) {
                 if (x + z < width) {
                    tmp.r += cuda_pixel_buffer[(x+z)*width+y].r ;
                    tmp.g += cuda_pixel_buffer[(x+z)*width+y].g ;
                    tmp.b += cuda_pixel_buffer[(x+z)*width+y].b ;
                    adds++;
                 }
                 if (x - z > 0) {
                    tmp.r += cuda_pixel_buffer[(x-z)*width+y].r  ;
                    tmp.g += cuda_pixel_buffer[(x-z)*width+y].g  ;
                    tmp.b += cuda_pixel_buffer[(x-z)*width+y].b  ;
                    adds++;
                 }
             }
             tmp.r /= adds;
             tmp.g /= adds;
             tmp.b /= adds;


             __syncthreads();
             cuda_pixel_buffer[index] = tmp;

		}//end loop 100 times
	}

}


//main cuda fucntion call
__global__ void cuRaster(cudaTri* pcudaTri, cudaVector3* pcudaVector3, cudaPixel * cuda_pixel_buffer,int width, int height, int numTriangles){


//ddprintf("-");	
//dprintd(blockIdx.x);

	int i;
	//simple test
	//int index;


	//bounding the threads
	int lim = numTriangles/THREADSPERBLOCK  + ((numTriangles % THREADSPERBLOCK )==0 ? 0 :1) ;

  if (blockIdx.x < lim) {
    i = blockIdx.x*THREADSPERBLOCK + threadIdx.x;


    // Check if normal is facing towards you
    if (pcudaTri[i].normal.z < 0 || i > numTriangles) {

    }
    else {

      for (int row = 0; row < 2; row++) {


  
      // Get minimum and maximum x
   		// returns the smaller of 3 numbers (x < y ? (x < z ? x : z) : (z < y ? z : y))
	   	int minx = (pcudaVector3[pcudaTri[i].v1].x < pcudaVector3[pcudaTri[i].v2].x ? (pcudaVector3[pcudaTri[i].v1].x < pcudaVector3[pcudaTri[i].v3].x ? pcudaVector3[pcudaTri[i].v1].x : pcudaVector3[pcudaTri[i].v3].x) : (pcudaVector3[pcudaTri[i].v3].x < pcudaVector3[pcudaTri[i].v2].x ? pcudaVector3[pcudaTri[i].v3].x : pcudaVector3[pcudaTri[i].v2].x));
      if (minx < 0) { minx = 0; }     
      // returns the larger of 3 numbers (x > y ? (x > z ? x : z) : (z > y ? z : y))
		  int maxx = (pcudaVector3[pcudaTri[i].v1].x > pcudaVector3[pcudaTri[i].v2].x ? (pcudaVector3[pcudaTri[i].v1].x > pcudaVector3[pcudaTri[i].v3].x ? pcudaVector3[pcudaTri[i].v1].x : pcudaVector3[pcudaTri[i].v3].x) : (pcudaVector3[pcudaTri[i].v3].x > pcudaVector3[pcudaTri[i].v2].x ? pcudaVector3[pcudaTri[i].v3].x : pcudaVector3[pcudaTri[i].v2].x));
  		if (maxx + 1 > width ) { maxx = width-1; }
  		int miny = pcudaVector3[pcudaTri[i].v1].y  < pcudaVector3[pcudaTri[i].v2].y  ? (pcudaVector3[pcudaTri[i].v1].y  < pcudaVector3[pcudaTri[i].v3].y ? pcudaVector3[pcudaTri[i].v1].y  : pcudaVector3[pcudaTri[i].v3].y) : (pcudaVector3[pcudaTri[i].v3].y < pcudaVector3[pcudaTri[i].v2].y  ? pcudaVector3[pcudaTri[i].v3].y : pcudaVector3[pcudaTri[i].v2].y );
  		if (miny < 0) { miny = 0; }
   		int maxy = pcudaVector3[pcudaTri[i].v1].y > pcudaVector3[pcudaTri[i].v2].y  ? (pcudaVector3[pcudaTri[i].v1].y > pcudaVector3[pcudaTri[i].v3].y ? pcudaVector3[pcudaTri[i].v1].y : pcudaVector3[pcudaTri[i].v3].y) : (pcudaVector3[pcudaTri[i].v3].y > pcudaVector3[pcudaTri[i].v2].y  ? pcudaVector3[pcudaTri[i].v3].y : pcudaVector3[pcudaTri[i].v2].y );
      if (maxy + 1 > width) { maxy = width-1; }



        //for (int col = 0; col < 5; col++) {
          for (int y = miny; y < maxy+1; y++) {
            for (int x = minx; x < maxx+1; x++) {


                double A = (pcudaVector3[pcudaTri[i].v2].x - pcudaVector3[pcudaTri[i].v1].x) * (pcudaVector3[pcudaTri[i].v3].y - pcudaVector3[pcudaTri[i].v1].y)  ;
                A -= (pcudaVector3[pcudaTri[i].v3].x - pcudaVector3[pcudaTri[i].v1].x) * (pcudaVector3[pcudaTri[i].v2].y - pcudaVector3[pcudaTri[i].v1].y)   ;

                double beta =  (pcudaVector3[pcudaTri[i].v1].x - pcudaVector3[pcudaTri[i].v3].x) * (y - pcudaVector3[pcudaTri[i].v3].y)  ;
                beta -=  (x - pcudaVector3[pcudaTri[i].v3].x) * (pcudaVector3[pcudaTri[i].v1].y - pcudaVector3[pcudaTri[i].v3].y)  ;
                beta /= A;

                double gamma =  (pcudaVector3[pcudaTri[i].v2].x - pcudaVector3[pcudaTri[i].v1].x);
                gamma *= (y - pcudaVector3[pcudaTri[i].v1].y);
                gamma -= (x - pcudaVector3[pcudaTri[i].v1].x) * (pcudaVector3[pcudaTri[i].v2].y - pcudaVector3[pcudaTri[i].v1].y)  ;
                gamma /= A;  
                
                double alpha = 1.0;
                alpha -= beta;
                alpha -= gamma;
                
                //int check = alpha > 0.0;
                int check = (alpha > -0.0) && (alpha < 1.01);
                check = check &&  (gamma > -0.00) && (gamma < 1.01);
                check = check && (beta > -0.0) && (beta < 1.01);
                if (check) {

                  cuda_pixel_buffer[x*width+y].r = alpha * (pcudaTri[i].c1.r) + beta * ( pcudaTri[i].c2.r ) + gamma * ( pcudaTri[i].c3.r  );
                  cuda_pixel_buffer[x*width+y].g = alpha * (pcudaTri[i].c1.g) + beta * ( pcudaTri[i].c2.g ) + gamma * ( pcudaTri[i].c3.g  );
                  cuda_pixel_buffer[x*width+y].b = alpha * (pcudaTri[i].c1.b) + beta * ( pcudaTri[i].c2.b ) + gamma * ( pcudaTri[i].c3.b  );

                }
            }
          }
        //}
      }



    }

  }

	/*if(blockIdx.x < lim){
		if((threadIdx.x) < (THREADSPERBLOCK)){
			i = blockIdx.x*THREADSPERBLOCK + threadIdx.x;
			//magical stuff will happens here
			







		int t = pcudaVector3[pcudaTri[i].v1].x;

      if (pcudaTri[i].normal.z < 0) {

      }
      else {
         // Get minimum and maximum x
         //int minx = min_val(pcudaVector3[pcudaTri[i].v1].x, pcudaVector3[pcudaTri[i].v2].x, pcudaVector3[pcudaTri[i].v3].x);
		// returns the smaller of 3 numbers (x < y ? (x < z ? x : z) : (z < y ? z : y))
		int minx = (pcudaVector3[pcudaTri[i].v1].x < pcudaVector3[pcudaTri[i].v2].x ? (pcudaVector3[pcudaTri[i].v1].x < pcudaVector3[pcudaTri[i].v3].x ? pcudaVector3[pcudaTri[i].v1].x : pcudaVector3[pcudaTri[i].v3].x) : (pcudaVector3[pcudaTri[i].v3].x < pcudaVector3[pcudaTri[i].v2].x ? pcudaVector3[pcudaTri[i].v3].x : pcudaVector3[pcudaTri[i].v2].x));
	

         if (minx < 0) { minx = 0; }
         //int maxx = max_val(pcudaVector3[pcudaTri[i].v1].x, pcudaVector3[pcudaTri[i].v2].x, pcudaVector3[pcudaTri[i].v3].x);
        // returns the larger of 3 numbers (x > y ? (x > z ? x : z) : (z > y ? z : y))
		int maxx = (pcudaVector3[pcudaTri[i].v1].x > pcudaVector3[pcudaTri[i].v2].x ? (pcudaVector3[pcudaTri[i].v1].x > pcudaVector3[pcudaTri[i].v3].x ? pcudaVector3[pcudaTri[i].v1].x : pcudaVector3[pcudaTri[i].v3].x) : (pcudaVector3[pcudaTri[i].v3].x > pcudaVector3[pcudaTri[i].v2].x ? pcudaVector3[pcudaTri[i].v3].x : pcudaVector3[pcudaTri[i].v2].x));
 

		if (maxx + 1 > width ) { maxx = width-1; }

         // Get minimum and maximum y
         //int miny = min_val(pcudaVector3[pcudaTri[i].v1].y, pcudaVector3[pcudaTri[i].v2].y, pcudaVector3[pcudaTri[i].v3].y);
         // returns the smaller of 3 numbers (x < y ? (x < z ? x : z) : (z < y ? z : y))
		int miny = pcudaVector3[pcudaTri[i].v1].y  < pcudaVector3[pcudaTri[i].v2].y  ? (pcudaVector3[pcudaTri[i].v1].y  < pcudaVector3[pcudaTri[i].v3].y ? pcudaVector3[pcudaTri[i].v1].y  : pcudaVector3[pcudaTri[i].v3].y) : (pcudaVector3[pcudaTri[i].v3].y < pcudaVector3[pcudaTri[i].v2].y  ? pcudaVector3[pcudaTri[i].v3].y : pcudaVector3[pcudaTri[i].v2].y );


		if (miny < 0) { miny = 0; }
         //int maxy = max_val(pcudaVector3[pcudaTri[i].v1].y, pcudaVector3[pcudaTri[i].v2].y, pcudaVector3[pcudaTri[i].v3].y);
		// returns the larger of 3 numbers (x > y ? (x > z ? x : z) : (z > y ? z : y))
		int maxy = pcudaVector3[pcudaTri[i].v1].y > pcudaVector3[pcudaTri[i].v2].y  ? (pcudaVector3[pcudaTri[i].v1].y > pcudaVector3[pcudaTri[i].v3].y ? pcudaVector3[pcudaTri[i].v1].y : pcudaVector3[pcudaTri[i].v3].y) : (pcudaVector3[pcudaTri[i].v3].y > pcudaVector3[pcudaTri[i].v2].y  ? pcudaVector3[pcudaTri[i].v3].y : pcudaVector3[pcudaTri[i].v2].y );
         

if (maxy + 1 > width) { maxy = width-1; }

		// iterates through the triangle pixel, then calculates the alpha beta and gamma
         for (int y = miny; y < maxy+1; y++) {
            for (int x = minx; x < maxx+1; x++) {
               // Calculate alpha, beta, gamma, 
               
               double A = (pcudaVector3[pcudaTri[i].v2].x - pcudaVector3[pcudaTri[i].v1].x) * (pcudaVector3[pcudaTri[i].v3].y - pcudaVector3[pcudaTri[i].v1].y)  ;
               A -= (pcudaVector3[pcudaTri[i].v3].x - pcudaVector3[pcudaTri[i].v1].x) * (pcudaVector3[pcudaTri[i].v2].y - pcudaVector3[pcudaTri[i].v1].y)   ;

               double beta =  (pcudaVector3[pcudaTri[i].v1].x - pcudaVector3[pcudaTri[i].v3].x) * (y - pcudaVector3[pcudaTri[i].v3].y)  ;
               beta -=  (x - pcudaVector3[pcudaTri[i].v3].x) * (pcudaVector3[pcudaTri[i].v1].y - pcudaVector3[pcudaTri[i].v3].y)  ;
               beta /= A;

               double gamma =  (pcudaVector3[pcudaTri[i].v2].x - pcudaVector3[pcudaTri[i].v1].x) * (y - pcudaVector3[pcudaTri[i].v1].y)  ;
               gamma -= (x - pcudaVector3[pcudaTri[i].v1].x) * (pcudaVector3[pcudaTri[i].v2].y - pcudaVector3[pcudaTri[i].v1].y)  ;
               gamma /= A;  

               double alpha = 1.0 - beta - gamma;

               // Is the pixel inside the triangle? then color the pixel
               if ((alpha > -0.0) && (alpha < 1.01) && (gamma > -0.00) && (gamma < 1.01) && (beta > -0.0) && (beta < 1.01) ) {
                  // Color
                  color_t col;
                  col.r = alpha * (pcudaTri[i].c1.r) + beta * ( pcudaTri[i].c2.r ) + gamma * ( pcudaTri[i].c3.r  );
                  col.g = alpha * (pcudaTri[i].c1.g) + beta * ( pcudaTri[i].c2.g ) + gamma * ( pcudaTri[i].c3.g  );
                  col.b = alpha * (pcudaTri[i].c1.b) + beta * ( pcudaTri[i].c2.b ) + gamma * ( pcudaTri[i].c3.b  );
                  //col.r = 1.0; col.g = 0.0; col.b = 0.0;
                  //col.r = pcudaTri[i].normal.x;
                  //col.g = pcudaTri[i].normal.y;
                  //col.b = pcudaTri[i].normal.z;

                  // Is the depth higher than the current depth?
                     // Calculate interpolated z
                     float depth = alpha * (pcudaVector3[pcudaTri[i].v1].z) + beta * ( pcudaVector3[pcudaTri[i].v2].z) + gamma * (  pcudaVector3[pcudaTri[i].v3].z );
                     if (depth > cuda_pixel_buffer[x*width+y].depth) {
                        //depth_buffer[x*width+y] = depth;
						cuda_pixel_buffer[x*width+y].depth = depth;						
                        //printf("Coloring %d, %d.\n", x, y);
                        //img.pixel(x,y, col);
                        // img_buffer[x*width+y] = col;
						//printf("coloring a pixel\n");
						cuda_pixel_buffer[x*width+y].r = col.r;
						cuda_pixel_buffer[x*width+y].g = col.g;
						cuda_pixel_buffer[x*width+y].b = col.b;

                     }
               }
            }
         }


      }























		}
	}*/
}



void transform(cudaVector3* pcudaVert,  int numVertices,int xoffset, int yoffset ) {
  for (int i = 0; i < numVertices; i++) {
    pcudaVert[i].x += xoffset;
    pcudaVert[i].y += yoffset;
  }
}

void transformVector(vector<Vector3 *> & Vertices,  int numVertices,int xoffset, int yoffset ) {
  for (int i = 0; i < Vertices.size(); i++) {
    Vertices[i]->x += xoffset;
    Vertices[i]->y += yoffset;
  }
}




void CudaRasterizeTriangles(vector<Tri *> & Triangles, vector<Vector3 *> & Vertices, int width, int height){

	//malloc array space for cuda triangle vector
	cudaTri* pcudaTri = (cudaTri*)malloc(Triangles.size()*sizeof(cudaTri));
	//mallocing space for vertices array
	cudaVector3* pcudaVector3 = (cudaVector3*)malloc(Vertices.size()*sizeof(cudaVector3));
	
	//place the data into the new arrays
	for(int i=0; i<Triangles.size();i++ ){
		pcudaTri[i].v1 = Triangles[i]->v1;
		pcudaTri[i].v2 = Triangles[i]->v2; 
		pcudaTri[i].v3 = Triangles[i]->v3; 
    pcudaTri[i].normal = Triangles[i]->normal;
    pcudaTri[i].c1 = Triangles[i]->c1;
    pcudaTri[i].c2 = Triangles[i]->c2;
    pcudaTri[i].c3 = Triangles[i]->c3;

	}
	
	for(int i=0; i<Vertices.size(); i++){
		pcudaVector3[i].x =  Vertices[i]->x; 
		pcudaVector3[i].y =  Vertices[i]->y; 
		pcudaVector3[i].z =  Vertices[i]->z; 
	}

  

	color_t* img_buffer = (color_t*)malloc(sizeof(color_t)*width*height);

   Image img(width, height);

   //cuda pixel buffer, will load into shared memory space
   cudaPixel* cuda_pixel_buffer = (cudaPixel*)malloc(sizeof(cudaPixel)*width*height);
	if(cuda_pixel_buffer == NULL){ 
		printf("OH NO Malloc Failed!!!!\n");
	}else{
		printf("Malloc ok!\n");
	}


	//mallocing space on card
	//mallocing cudabuffer
	cudaPixel* d_cuda_pixel_buffer;
	cudaMalloc((void**)&d_cuda_pixel_buffer, width*height*sizeof(cudaPixel));
	ERRORCHECK;
	cudaMemcpy( d_cuda_pixel_buffer, cuda_pixel_buffer, width*height*sizeof(cudaPixel),cudaMemcpyHostToDevice );
	ERRORCHECK 
	//mallocing trianglebuffer
	cudaTri* d_cudaTri;
	cudaMalloc((void**)&d_cudaTri,Triangles.size()*sizeof(cudaTri));
	ERRORCHECK
	cudaMemcpy( d_cudaTri, pcudaTri,Triangles.size()*sizeof(cudaTri) ,cudaMemcpyHostToDevice ); 
	ERRORCHECK	
	//mallocing vertices buffer
	cudaVector3* d_cudaVector3;
	cudaMalloc((void**)&d_cudaVector3,Vertices.size()*sizeof(cudaVector3));
	ERRORCHECK



	//defining the size of the array size
	int blocksize = ((Triangles.size())/THREADSPERBLOCK) + ( ((Triangles.size())%THREADSPERBLOCK)==0 ? 0 : 1);
	printf("Before the Kernel call\n");
dprintd(blocksize);
dprintd(THREADSPERBLOCK);
  //for (int row = 0; row < 5; row++) {
    //for (int col = 0; col < 5; col++) {
        transform(pcudaVector3, Vertices.size(), 400, -800);
        cudaMemcpy( d_cudaVector3, pcudaVector3 ,Vertices.size()*sizeof(cudaVector3) ,cudaMemcpyHostToDevice ); 
        ERRORCHECK


		blocksize = (width*height)/THREADSPERBLOCK;
		printf("\nabout to call cuBlur ");
		dprintd(blocksize);
		dprintd(THREADSPERBLOCK);

        //calling the cuda code
        cuRaster<<<blocksize, THREADSPERBLOCK>>>(d_cudaTri, d_cudaVector3, d_cuda_pixel_buffer, width, height,Triangles.size() );
          ERRORCHECK


	//calling the cuda bluring 
	printf("\nBefore the bluring call\n");
    cuBlur<<<blocksize, THREADSPERBLOCK>>>( d_cuda_pixel_buffer, width, height );
	ERRORCHECK



		printf("Making the second bunny\n");
        transform(pcudaVector3, Vertices.size(), 400, -600);
        cudaMemcpy( d_cudaVector3, pcudaVector3 ,Vertices.size()*sizeof(cudaVector3) ,cudaMemcpyHostToDevice ); 
        ERRORCHECK

		blocksize = (width*height)/THREADSPERBLOCK;
        //calling the cuda code
        cuRaster<<<blocksize, THREADSPERBLOCK>>>(d_cudaTri, d_cudaVector3, d_cuda_pixel_buffer, width, height,Triangles.size() );
          ERRORCHECK













	printf("About to the memcpy call\n");
	//copying th memory back from the kernel 
    cudaMemcpy(cuda_pixel_buffer, d_cuda_pixel_buffer, width*height*sizeof(cudaPixel),cudaMemcpyDeviceToHost);
ERRORCHECK	

	//copy the cuda pixel buffer into the image buffer
	printf("returned from kernel ok\n");
	color_t col;
	int index=0;
	col.r = 1; col.g = 0; col.b=0;
 
   
	printf("done copying from the cudabuffer\n");

 // float weight[5] = { 0.9, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };

  //serial version of blur
  /*	
  // RUN the GAUSSIAN BLUR 100 times.
  for (int i = 0; i < 100; i++) {
      
      // Horizontal blur
       for (int y = 0; y < height; y++) {
          for (int x = 0; x < width; x++) {
             index = x*width+y;
             int adds = 0;
             // Loop through five times
             for (int z = 0; z < 3; z++) {
                 if (y + z < width) {
                    cuda_pixel_buffer[index].r += cuda_pixel_buffer[index+z].r;
                    cuda_pixel_buffer[index].g += cuda_pixel_buffer[index+z].g;
                    cuda_pixel_buffer[index].b += cuda_pixel_buffer[index+z].b;
                    adds++;
                 }
                 if (y - z > 0) {
                    cuda_pixel_buffer[index].r += cuda_pixel_buffer[index-z].r;
                    cuda_pixel_buffer[index].g += cuda_pixel_buffer[index-z].g;
                    cuda_pixel_buffer[index].b += cuda_pixel_buffer[index-z].b;
                    adds++;
                 }
             }
             cuda_pixel_buffer[index].r /= adds;
             cuda_pixel_buffer[index].g /= adds;
             cuda_pixel_buffer[index].b /= adds;
          }
       }//end of h blur

       // Verticle blur
       for (int y = 0; y < height; y++) {
          for (int x = 0; x < width; x++) {
             index = x*width+y;
             int adds = 0;
             //cuda_pixel_buffer[index].r *= weight[0];
             //cuda_pixel_buffer[index].g *= weight[0];
             //cuda_pixel_buffer[index].b *= weight[0];
             // Loop through five times
             for (int z = 0; z < 2; z++) {
                 if (x + z < width) {
                    cuda_pixel_buffer[index].r += cuda_pixel_buffer[(x+z)*width+y].r ;
                    cuda_pixel_buffer[index].g += cuda_pixel_buffer[(x+z)*width+y].g ;
                    cuda_pixel_buffer[index].b += cuda_pixel_buffer[(x+z)*width+y].b ;
                    adds++;
                 }
                 if (x - z > 0) {
                    cuda_pixel_buffer[index].r += cuda_pixel_buffer[(x-z)*width+y].r  ;
                    cuda_pixel_buffer[index].g += cuda_pixel_buffer[(x-z)*width+y].g  ;
                    cuda_pixel_buffer[index].b += cuda_pixel_buffer[(x-z)*width+y].b  ;
                    adds++;
                 }
             }
             cuda_pixel_buffer[index].r /= adds;
             cuda_pixel_buffer[index].g /= adds;
             cuda_pixel_buffer[index].b /= adds;


          }
       }//end of vert blur

  }//end of serial blur
*/


//copy cuda_pixles to buffer 
   for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
  		index = x*width+y;
  		col.r = cuda_pixel_buffer[index].r;
  		col.g = cuda_pixel_buffer[index].g;
  		col.b = cuda_pixel_buffer[index].b;
  		img_buffer[index] = col;

        img.pixel(x,y, img_buffer[x*width+y]);


      }
   }

   img.WriteTga("awesome.tga", false);
   free(img_buffer);
	free(pcudaVector3);
	free(pcudaTri);
   cudaFree(d_cuda_pixel_buffer);
   cudaFree(d_cudaVector3);

}



int main( int argc, char** argv ) {
  

  //make sure a file to read is specified
  if (argc > 1) {
    cout << "file " << argv[1] << endl;
   
    

    //read-in the mesh file specified
    ReadFile(argv[1]);
    //printFirstThree();
    ColorVertices1(Triangles, Vertices);
    printf("Done Coloring...\n"); 
    Convert_to_Window(Vertices, 2000, 2000, 6, -600, 500);
		
    //RasterizeTriangles(Triangles, Vertices, 2000, 2000);
    
  	//cuda version of rasterize triangle
    CudaRasterizeTriangles(Triangles, Vertices, 2000, 2000);
 
  }
  return 0;
}




