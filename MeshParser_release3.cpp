//basic program to read in a mesh file (of .m format from H. Hoppe)
//Hh code modified by ZJW for csc 471

#include <stdio.h>
#include <math.h>
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
  
  int indx = 0, vi;
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
  printf("second vertex: %f %f %f \n", Vertices[1]->x, Vertices[1]->y, Vertices[1]);
  printf("second face: %d %d %d \n", Triangles[1]->v1, Triangles[1]->v2, Triangles[1]->v3);
  printf("third vertex: %f %f %f \n", Vertices[2]->x, Vertices[2]->y, Vertices[2]->z);
  printf("third face: %d %d %d \n", Triangles[2]->v1, Triangles[2]->v2, Triangles[2]->v3);
}

int main( int argc, char** argv ) {
  return 0;
}

//drawing routine to draw triangles as wireframe
/*void drawTria(Tri* t) {
  if(display_mode == 0) {
    glBegin(GL_LINE_LOOP);
    glColor3f(0.0, 0.0, 0.5);
    //note that the vertices are indexed starting at 0, but the triangles
    //index them starting from 1, so we must offset by -1!!!
    glVertex3f(Vertices[t->v1 - 1]->x, 
      Vertices[t->v1 - 1]->y,
      Vertices[t->v1 - 1]->z);
    glVertex3f(Vertices[t->v2 - 1]->x, 
      Vertices[t->v2 - 1]->y,
      Vertices[t->v2 - 1]->z);
    glVertex3f(Vertices[t->v3 - 1]->x, 
      Vertices[t->v3 - 1]->y,
      Vertices[t->v3 - 1]->z);
    glEnd();
  }
}

void drawSphere() {
  glColor3f(1.0, 0.0, 0.0);
  glutWireSphere(0.35, 10, 10);
}

//debugging routine which just draws the vertices of the mesh
void DrawAllVerts() {
  
  glColor3f(1.0, 0.0, 1.0);
  glBegin(GL_POINTS);
  for (unsigned int k=0; k < Vertices.size(); k++) {
    glVertex3f(Vertices[k]->x, Vertices[k]->y, Vertices[k]->z);
  }
  glEnd();
  
}

void reshape(int w, int h) {
  GW = w;
  GH = h;
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (view_mode == 0)
    glOrtho( -2.0*(float)w/h, 2.0*(float)w/h, -2.0, 2.0, 1.0, 15.0);
  //else... fill in
  glMatrixMode(GL_MODELVIEW);
  glViewport(0, 0, w, h);
  
  glutPostRedisplay();
}

void drawObjects() {
  
  //transforms for the mesh
  glPushMatrix();
  //leave these transformations in as they center and scale each mesh correctly
  //scale object to window
  glScalef(1.0/(float)max_extent, 1.0/(float)max_extent, 1.0/(float)max_extent);
  //translate the object to the orgin
  glTranslatef(-(center.x), -(center.y), -(center.z));
  //draw the wireframe mesh
  for(unsigned int j = 0; j < Triangles.size(); j++) {
    drawTria(Triangles[j]);
  }
  glPopMatrix();
  
  //transforms for the sphere
  glPushMatrix();
  //draw the glut sphere behind the mesh
  glTranslatef(1.25, 0.0, -2.0);
  drawSphere();
  glPopMatrix();
}

void display() {
  
  float numV = Vertices.size();
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glMatrixMode(GL_MODELVIEW);
    
  glPushMatrix();
  //set up the camera
  gluLookAt(0, 0, 3.0, 0, 0, 0, 0, 1, 0);
    
  drawObjects();

  glPopMatrix();
    
  glutSwapBuffers();
    
}



int main( int argc, char** argv ) {
  
  //set up my window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(300, 300); 
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Mesh display");
  glClearColor(1.0, 1.0, 1.0, 1.0);
  
  //register glut callback functions
  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  //enable z-buffer
  glEnable(GL_DEPTH_TEST);
  
  //initialization
  max_x = max_y = max_z = FLT_MIN;
  min_x = min_y = min_z = FLT_MAX;
  center.x = 0;
  center.y = 0;
  center.z = 0;
  display_mode = 0;
  max_extent = 1.0;
  view_mode = 0;
  
  //make sure a file to read is specified
  if (argc > 1) {
    cout << "file " << argv[1] << endl;
    //read-in the mesh file specified
    ReadFile(argv[1]);
    //only for debugging
    if (Vertices.size() > 4)
      printFirstThree();
    
    //once the file is parsed find out the maximum extent to center and scale mesh
    max_extent = max_x - min_x;
    if (max_y - min_y > max_extent) max_extent = max_y - min_y;
    //cout << "max_extent " << max_extent << " max_x " << max_x << " min_x " << min_x << endl;
    //cout << "max_y " << max_y << " min_y " << min_y << " max_z " << max_z << " min_z " << min_z << endl;
    
    center.x = center.x/Vertices.size();
    center.y = center.y/Vertices.size();
    center.z = center.z/Vertices.size();
    //cout << "center " << center.x << " " << center.y << " " << center.z << endl;
    //cout << "scale by " << 1.0/(float)max_extent << endl;
  } else {
    cout << "format is: meshparser filename" << endl;
  }

  
  glutMainLoop();
}*/


