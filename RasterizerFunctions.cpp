#include "Rasterizer.h"

Vector3 normalize(Vector3 v) {
  double mag = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); 
  return Vector3(v.x/mag, v.y/mag, v.z/mag);
}

double clamp(double num, double floor, double ceiling) {
  if (num < floor) { return floor; }
  if (num > ceiling) { return ceiling; }
  return num;
}

double dot(Vector3 v1, Vector3 v2) {
  return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}


double p2wy(double y, int height) {
  return (((height - 1.0) / 2) * y) + ((height-1.0)/2);
}

double p2wx(double x, int width) {
  return (((width - 1.0) / 2) * x) + ((width-1.0)/2);
}

void Convert_to_Window(vector<Vector3 *> & Vertices, int width, int height) {

  for (int i = 0; i < Vertices.size(); i++) {
    (*(Vertices[i])).x = p2wy( (*(Vertices[i])).x, width);
    (*(Vertices[i])).y = p2wy( (*(Vertices[i])).y, height);
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
   return (alpha > -0.01) && (alpha < 1.01) && (gamma > -0.01) && (gamma < 1.01) && (beta > -0.01) && (beta < 1.01);
}

void RasterizeTriangles(vector<Tri *> & Triangles, vector<Vector3 *> & Vertices, int width, int height) {

   double* img_buffer = (double*)malloc(sizeof(double)*width*height);
   double* depth_buffer = (double*)malloc(sizeof(double)*width*height);

   Image img(width, height);

   //Iterate through each Triangle
   for (int i = 0; i < Triangles.size(); i++) {
      int t = Vertices[Triangles[i]->v1]->x;

      // Get minimum and maximum x
      int minx = min_val(Vertices[Triangles[i]->v1]->x, Vertices[Triangles[i]->v3]->x, Vertices[Triangles[i]->v3]->x);
      int maxx = max_val(Vertices[Triangles[i]->v1]->x, Vertices[Triangles[i]->v3]->x, Vertices[Triangles[i]->v3]->x);

      // Get minimum and maximum y
      int miny = min_val(Vertices[Triangles[i]->v1]->y, Vertices[Triangles[i]->v3]->y, Vertices[Triangles[i]->v3]->y);
      int maxy = max_val(Vertices[Triangles[i]->v1]->y, Vertices[Triangles[i]->v3]->y, Vertices[Triangles[i]->v3]->y);

      for (int i = minx; i < maxx+1; i++) {
         for (int j = miny; j < maxy+1; j++) {
            // Calculate alpha, beta, gamma, 

            double A = (Vertices[Triangles[i]->v2]->x - Vertices[Triangles[i]->v1]->x) * (Vertices[Triangles[i]->v3]->y - Vertices[Triangles[i]->v1]->y)  ;
            A -= (Vertices[Triangles[i]->v3]->x - Vertices[Triangles[i]->v1]->x) * (Vertices[Triangles[i]->v2]->y - Vertices[Triangles[i]->v1]->y)   ;

            double beta =  (Vertices[Triangles[i]->v1]->x - Vertices[Triangles[i]->v3]->x) * (j - Vertices[Triangles[i]->v3]->y)  ;
            beta -=  (i - Vertices[Triangles[i]->v3]->x) * (Vertices[Triangles[i]->v1]->y - Vertices[Triangles[i]->v3]->y)  ;
            beta /= A;

            double gamma =  (Vertices[Triangles[i]->v2]->x - Vertices[Triangles[i]->v1]->x) * (j - Vertices[Triangles[i]->v1]->y)  ;
            gamma -= (i - Vertices[Triangles[i]->v1]->x) * (Vertices[Triangles[i]->v2]->y - Vertices[Triangles[i]->v1]->y)  ;
            gamma /= A;  

            double alpha = 1.0 - beta - gamma;

            // Is it inside?
            if (in_triangle(alpha, gamma, beta)) {
               // Color
               color_t col;
               col.r = alpha * (Triangles[i]->c1.r) + beta * ( Triangles[i]->c2.r ) + gamma * ( Triangles[i]->c3.r  );
               col.g = alpha * (Triangles[i]->c1.g) + beta * ( Triangles[i]->c2.g ) + gamma * ( Triangles[i]->c3.g  );
               col.b = alpha * (Triangles[i]->c1.b) + beta * ( Triangles[i]->c2.b ) + gamma * ( Triangles[i]->c3.b  );
               img.pixel(i, j, col);
            }
         }
      }

   }


   img.WriteTga("awesome.tga", false);
   free(img_buffer);
   free(depth_buffer);
}

void ColorVertices(vector<Tri *> & Triangles, vector<Vector3 *> & Vertices) {

  Vector3 light_pos = Vector3(1.0, 1.0, 1.0);
  double material = 0.2;
  Color color = Color(0.4, 0.1, 0.1);

  for (int i = 0; i < Triangles.size(); i++) {
    int i1 = (*(Triangles[i])).v1;
    int i2 = (*(Triangles[i])).v2;
    int i3 = (*(Triangles[i])).v3;

    // Calculate and save the normal
    Vector3 v1 = Vector3((*(Vertices[i1])).x - (*(Vertices[i2])).x,
                         (*(Vertices[i1])).y - (*(Vertices[i2])).y,
                          (*(Vertices[i1])).z - (*(Vertices[i2])).z);
    Vector3 v2 = Vector3((*(Vertices[i1])).x - (*(Vertices[i3])).x,
                         (*(Vertices[i1])).y - (*(Vertices[i3])).y,
                          (*(Vertices[i1])).z - (*(Vertices[i3])).z);
    Vector3 cross = Vector3();
    cross.x = v1.y*v2.z - v2.y*v1.z;
    cross.y = v2.x*v1.z - v1.x*v2.z;
    cross.z = v1.x*v2.y - v1.y*v2.x;

    (*(Triangles[i])).normal = normalize(cross);
  }

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
    color.r = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.r;
    color.g = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.g;
    color.b = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.b;
    (*(Triangles[i])).c1 = color;

    int i2 = (*(Triangles[i])).v2;
    tri = (*(Triangles[i]));
    vertex = (*(Vertices[i2]));

    to_light = normalize(Vector3(
                                  light_pos.x - vertex.x,
                                  light_pos.y- vertex.y,
                                  light_pos.z - vertex.z ));


    // Get color for vector 2
    color = Color();
    color.r = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.r;
    color.g = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.g;
    color.b = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.b;
    (*(Triangles[i])).c2 = color;    
    
    
    int i3 = (*(Triangles[i])).v3;
    tri = (*(Triangles[i]));
    vertex = (*(Vertices[i3]));

    to_light = normalize(Vector3(
                                  light_pos.x - vertex.x,
                                  light_pos.y- vertex.y,
                                  light_pos.z - vertex.z ));


    // Get color for vector 2
    color = Color();
    color.r = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.r;
    color.g = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.g;
    color.b = material * (clamp((dot(tri.normal, to_light)), 0.0, 1.0)) * color.b;
    (*(Triangles[i])).c3 = color;   

  }

}
