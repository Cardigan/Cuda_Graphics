#include "Rasterizer.h"

Vector3 normalize(Vector3 v) {
  double mag = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); 
  return Vector3(v.x/mag, v.y/mag, v.z/mag);
}

void ColorVertices(vector<Tri *> Triangles, vector<Vector3 *> Vertices) {

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
    // Get color for vector 1
    Color x = Color();

    //Vector3 to_light = normalize(Vector3())
  }

}
