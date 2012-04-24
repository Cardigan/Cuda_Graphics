//basic program to read in a mesh file (of .m format from H. Hoppe)
//Hh code modified by ZJW for csc 471
#include "Rasterizer.h"


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
                  col.g = alpha * (Triangles[i]->c1.g) + beta * ( Triangles[i]->c2.g ) + gamma * ( Triangles[i]->c3.g  );
                  col.b = alpha * (Triangles[i]->c1.b) + beta * ( Triangles[i]->c2.b ) + gamma * ( Triangles[i]->c3.b  );
                  //col.r = 1.0; col.g = 0.0; col.b = 0.0;
                  //col.r = Triangles[i]->normal.x;
                  //col.g = Triangles[i]->normal.y;
                  //col.b = Triangles[i]->normal.z;

                  // Is the depth higher than the current depth?
                     // Calculate interpolated z
                     float depth = alpha * (Vertices[Triangles[i]->v1]->z) + beta * ( Vertices[Triangles[i]->v2]->z) + gamma * (  Vertices[Triangles[i]->v3]->z );
                     if (depth > depth_buffer[x*width+y]) {
                        depth_buffer[x*width+y] = depth;
                        //printf("Coloring %d, %d.\n", x, y);
                        //img.pixel(x,y, col);
                        img_buffer[x*width+y] = col;
                     }
               }
            }
         }


      }

   }

   for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
         img.pixel(x,y, img_buffer[x*width+y]);
      }
   }

   img.WriteTga("awesome.tga", true);
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
  printf("second vertex: %f %f %f \n", Vertices[1]->x, Vertices[1]->y, Vertices[1]->z);
  //printf("second face: %d %d %d \n", Triangles[1]->v1, Triangles[1]->v2, Triangles[1]->v3);
  printf("third vertex: %f %f %f \n", Vertices[2]->x, Vertices[2]->y, Vertices[2]->z);
  //printf("third face: %d %d %d \n", Triangles[2]->v1, Triangles[2]->v2, Triangles[2]->v3);
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
    Convert_to_Window(Vertices, 2000, 2000, 8, 60, -1000);
    RasterizeTriangles(Triangles, Vertices, 2000, 2000);
   }
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


