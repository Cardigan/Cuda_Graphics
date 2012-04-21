#include <stdio.h>
#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <GL/glut.h>
#include <assert.h>
#include <map>

using namespace std;

#define FLT_MIN 1.1754E-38F
#define FLT_MAX 1.1754E+38F

//very simple data structure to store 3d points
typedef struct Vector3
{
  float x;
  float y;
  float z;
  Vector3(float in_x, float in_y, float in_z) : x(in_x), y(in_y), z(in_z) {}
  Vector3() {}
} Vector3;

typedef struct Color
{
  float r;
  float g;
  float b;
  Color(float in_r, float in_g, float in_b) : r(in_r), g(in_g), b(in_b) {}
  Color() {}
} Color;

//data structure to store triangle - 
//note that v1, v2, and v3 are indexes into the vertex array
typedef struct Tri{
  int v1;
  int v2;
  int v3;
  Vector3 normal;
  Color c1, c2, c3;
  Tri(int in_v1, int in_v2, int in_v3) : v1(in_v1), v2(in_v2), v3(in_v3), normal(0, 1, 0){}
  Tri() : normal(0, 1, 0) {}
} Tri;

void ColorVertices(vector<Tri *> Triangles, vector<Vector3 *> Vertices);

