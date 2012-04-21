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

void ColorVertices(vector<Tri *> & Triangles, vector<Vector3 *> & Vertices) {

  Vector3 light_pos = Vector3(1.0, 1.0, 1.0);
  float material = 0.2;
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
