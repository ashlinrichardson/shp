#include "ansicolor.h"
#include <iostream>
#include <deque>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "ansicolor.h"
#include "rapidjson/document.h"
#include <boost/assign.hpp>
#include <boost/version.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/adapted/boost_range/filtered.hpp>

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

/* http://www.boost.org/doc/libs/1_47_0/libs/geometry/doc/html/geometry/reference/algorithms/append.html
http://www.boost.org/doc/libs/1_47_0/libs/geometry/doc/html/geometry/reference/algorithms/intersection.html
http://www.boost.org/doc/libs/1_65_0/libs/geometry/doc/html/geometry/reference/algorithms/intersection/intersection_3.html */
#include "hsv.h"
#include "newzpr.h"
#include "pthread.h"
#include "memory.h"
#include "time.h"
#include "vec3d.h"
#define MYFONT GLUT_BITMAP_HELVETICA_12

using namespace std;
using namespace rapidjson;

// point data from shape files
vector< vector<vec3d> > my_vectors;
vector< std::string > my_names;
vector< std::string > my_id;
vector< int > my_class;
vector< int > n_my_class;
vector< int > within_class_index;
vector< int > urx;

vector<vec3d> max_p;
double max_f;
int next_class;
int cur_fire_ind, cur_park_ind;
int n_fire, n_park;

// std::string * orc_to_name;

/* Draw axes */
#define STARTX 500
#define STARTY 500
int fullscreen;
clock_t start_time;
clock_t stop_time;
#define SECONDS_PAUSE 0.4

#define STR_MAX 10000
char console_string[STR_MAX];

int console_position;
int renderflag;

/*convert double to string*/
string dtos(double i){
  std::string number("");
  std::stringstream strstream;
  strstream.precision(12);
  strstream << i;
  strstream >> number;
  return number;
}

long int getFileSize(std::string fn){
  ifstream i;
  i.open(fn.c_str(), ios::binary);
  if(!i.is_open()){
    printf("Error: was unable to open file: %s\n", (fn.c_str()));
    return -1;
  }
  i.seekg(0, ios::end);
  return i.tellg(); // len
}

void poly_info(int my_ind){
  cout << "\t" << KYEL << "(" << KMAG << my_ind << KYEL << ")" << KGRN << "--> " << KYEL << "\t" << my_names[my_ind] << KGRN << endl;
}

void _pick(GLint name){
  if(myPickNames.size() < 1){
    return;
  }
  cout << "------------------------\n\tPickSet:" << endl;
  std::set<GLint>::iterator it;
  for(it = myPickNames.begin(); it != myPickNames.end(); it++){
    int my_ind = *it;
    poly_info(my_ind);

  }
  cout << KNRM << endl;
  cout <<"------------------------"<<endl;
  fflush(stdout);
}

void renderBitmapString(float x, float y, void *font, char *string){
  char *c;
  glRasterPos2f(x,y);
  for(c = string; *c != '\0'; c++){
    glutBitmapCharacter(font, *c);
  }
}

//http://www.codeproject.com/Articles/80923/The-OpenGL-and-GLUT-A-Powerful-Graphics-Library-an
void setOrthographicProjection(){
  int h = WINDOWY, w = WINDOWX;
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  glScalef(1, -1, 1);
  glTranslatef(0, -h, 0);
  glMatrixMode(GL_MODELVIEW);
}

void resetPerspectiveProjection(){
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void drawText(){
  glColor3f(0.0f, 1.0f, 0.0f);
  setOrthographicProjection();
  glPushMatrix();
  glLoadIdentity();
  int lightingState = glIsEnabled(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  renderBitmapString(3, WINDOWY - 3, (void *)MYFONT, console_string);
  if(lightingState){
    glEnable(GL_LIGHTING);
  }
  glPopMatrix();
  resetPerspectiveProjection();
}

/* convert float to string.. from gift meta4::gift */
string ftos(float i){
  std::string number("");
  std::stringstream strstream;
  strstream << i;
  strstream >> number;
  return number;
}

float a1, a2, a3; // what for?

void drawAxes(void){
  cout << KMAG << "drawAxes()" << KGRN << "----------------------------------------" << endl;
  /* draw crosshairs on origin for sanity */
  if(false){

    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glEnd();

    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();
  }

  int picki = -1;
  float r = 0., g = 1., b = 1.;

  //glPushMatrix();
  glColor3f(r,g,b);
  glPointSize(1.);

  if(myPickNames.size() == 1){
    picki = *myPickNames.begin();
    cout << picki << "," << my_names[picki] << ",myclass= " << my_class[picki]
    << "n_my_class" << n_my_class[picki] << endl;
    cout << "within_class_index" << within_class_index[picki] << endl;
  }

  int i = 0, ci = 0, n_labels = my_vectors.size();

  for(i = 0; i < n_labels; i++){

    /* fire centre class */
    if(my_class[i] == 0){
      if(cur_fire_ind >= 0 && cur_fire_ind < n_fire){
        if(cur_fire_ind != within_class_index[i]){
          continue;
        }
      }
    }

    /* parks class */
    if(my_class[i] == 1){
      if(cur_park_ind >= 0 && cur_park_ind < n_park){
        if(cur_park_ind != within_class_index[i]){
          continue;
        }
      }
    }

    float myf = ((float)((within_class_index[i] + 1))) / ((float)n_my_class[i]);
    hsv2rgb(&r, &g, &b, 360. * myf, my_class[i] == 1 ? 1. : 0.5, my_class[i] == 1 ? 1. : 0.15);
    vector<vec3d> * j = &my_vectors[i];
    vector<vec3d>::iterator it;
    ci = 0;

    if(picki == i){
      glColor3f(1. - r, 1. - g, 1. - b);
    }
    else{
      glColor3f(r, g, b);
    }

    glPushName(i);
    glBegin(GL_POLYGON);
    for(it = j->begin(); it != j->end(); it += 1){
      if(true){
        if(picki == i && ci == 0){
          zprReferencePoint[0] = (*it).x;
          zprReferencePoint[1] = (*it).y;
        }
        glVertex3f((*it).x, (*it).y, (*it).z);
      }
      ci ++;
    }
    glEnd();
    glPopName();
  }

  if(cur_fire_ind >= 0 && cur_fire_ind < n_fire && cur_park_ind >= 0 && cur_park_ind < n_park){
    int k = 0, i = cur_fire_ind;

    using boost::make_tuple;
    using boost::geometry::append;
    using boost::assign::tuple_list_of;
    typedef boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double> > polygon;
    polygon f_poly;

    float a = 0., b = 0., c = 0., d = 0.;

    long int skip_frac = 256;
    long int slen = my_vectors[i].size(), sskip = slen / skip_frac;
    if(slen < skip_frac){
      sskip = 1;
    }

    vector<vec3d> * v = &my_vectors[i];
    string wkt_f("POLYGON((");
    long int add_s = 0;

    poly_info(i);
    printf("\tsskip %ld\n", sskip);

    for(k=0; k< slen; k++){
      vec3d x(v->at(k));

      if(k % sskip == 0){
        add_s += 1;
        if(k > 0){
          wkt_f += ",";
        }
        wkt_f += ftos(x.x);
        wkt_f += " ";
        wkt_f += ftos(x.y);
      }
      if(k < 10){
        printf("\t%e %e\n", x.x, x.y);
        if(k == 0){
          a = b = x.x;
          c = d = x.y;
        }
      }
      if(x.x < a) a = x.x;
      if(x.x > b) b = x.x;
      if(x.y < c) c = x.y;
      if(x.y > d) d = x.y;
    }

    wkt_f+= "))";

    if(wkt_f.length() < 9999){
      //cout << wkt_f << endl;
    }

    if(false) printf("%sread_wkt%s()%s n%s=%s(%s%ld%s)%s from %ld\n", KYEL, KBLU, KGRN, KYEL, KRED, KMAG, add_s, KRED, KNRM, slen);

    boost::geometry::read_wkt(wkt_f, f_poly);

    if(false) printf("%scorrect%s()%s\n", KYEL, KBLU, KNRM);

    boost::geometry::correct(f_poly);

    if(false) printf("%sf_poly %si(%d) x(%f, %f) y(%f, %f)\n", KMAG, KNRM, i, a, b, c, d);
    //add first point to end?

    glColor3f(1., 0., 0.);
    glBegin(GL_LINES);
    glVertex3f(a, d, 0); glVertex3f(b, d, 0);
    glVertex3f(b, d, 0); glVertex3f(b, c, 0);
    glVertex3f(b, c, 0); glVertex3f(a, c, 0);
    glVertex3f(a, c, 0); glVertex3f(a, d, 0);
    glEnd();

    glColor3f(.7, .0, .0);
    glBegin(GL_LINES);
    glVertex3f( (a + b) / 2. + 1., (c + d) / 2., 0);
    glVertex3f( (a + b) / 2. - 1., (c + d) / 2., 0);
    glEnd();

    glColor3f(.7, .0, .0);
    glBegin(GL_LINES);
    glVertex3f( (a + b) / 2., (c + d) / 2. + 1., 0);
    glVertex3f( (a + b) / 2., (c + d) / 2. - 1., 0);
    glEnd();

    polygon p_poly;
    a = b = c = d = 0.;
    int j = cur_park_ind + n_fire; // the object index: remember, everything's lumped together in one array (fire centres, first)
    long int clen = my_vectors[j].size();
    long int cskip = (clen >= skip_frac)?(clen / 256):1;

    v = &my_vectors[j];

    string wkt_p("POLYGON((");
    long int add_c = 0;

    poly_info(j);
    printf("\tclen %ld\n", clen);
    printf("\tcskip %ld\n", cskip);

    for(k=0; k< clen; k++){
      vec3d x(v->at(k));

      if(k % cskip == 0){
        add_c += 1;
        if(k > 0){
          wkt_p += ",";
        }
        //cout << "\tx.x= " << x.x << endl;
        wkt_p += ftos(x.x);
        wkt_p += " ";
        wkt_p += ftos(x.y);
      }
      if(k < 10){
        printf("\t%e %e\n", x.x, x.y);
        if(k == 0){
          a = b = x.x;
          c = d = x.y;
        }
      }
      if(x.x < a) a = x.x;
      if(x.x > b) b = x.x;
      if(x.y < c) c = x.y;
      if(x.y > d) d = x.y;
    }
    wkt_p+= "))";
    if(wkt_p.length() < 9999){
      //printf("%shere%s\n", KRED, KGRN);
      //cout << wkt_p << endl;
      //printf("%safter%s\n", KRED, KGRN);

    }

    printf("\t%sread_wkt%s()%s n%s=%s(%s%ld%s)%s from %ld\n", KYEL, KBLU, KGRN, KYEL, KRED, KMAG, add_c, KRED, KNRM, clen);

    boost::geometry::read_wkt(wkt_p, p_poly);

    printf("%scorrect%s()%s\n", KYEL, KBLU, KNRM);

    boost::geometry::correct(p_poly);

    printf("\t%sp_poly %si(%d) x(%f, %f) y(%f, %f)\n", KMAG, KNRM, i, a, b, c, d);
    //add first point to the end?

    glColor3f(0., 0., 1.);
    glBegin(GL_LINES);
    glVertex3f(a, d, 0); glVertex3f(b, d, 0);
    glVertex3f(b, d, 0); glVertex3f(b, c, 0);
    glVertex3f(b, c, 0); glVertex3f(a, c, 0);
    glVertex3f(a, c, 0); glVertex3f(a, d, 0);
    glEnd();

    glColor3f(0, .7, 0.);
    glBegin(GL_LINES);
    glVertex3f( (a + b) / 2. + 1., (c + d) / 2., 0);
    glVertex3f( (a + b) / 2. - 1., (c + d) / 2., 0);
    glEnd();

    glColor3f(0, .7, 0.);
    glBegin(GL_LINES);
    glVertex3f( (a + b) / 2., (c + d) / 2. + 1., 0);
    glVertex3f( (a + b) / 2., (c + d) / 2. - 1., 0);
    glEnd();

    std::deque<polygon> p_result;

    if(false) printf("%sintersection%s()%s\n", KYEL, KBLU, KNRM);
    boost::geometry::intersection(f_poly, p_poly, p_result);

    if(false) printf("%sintersection%s(%ld)%s\n", KYEL, KBLU, p_result.size(), KNRM);
    std::deque<polygon>::iterator it;

    if(false) printf("%sarea%s()%s\n", KYEL, KBLU, KNRM);

    double my_area = 0.;
    int ci = 0;

    for(it = p_result.begin(); it != p_result.end(); it++){
      printf("poly(i=%d)\n", ci++);
      float f = (double)boost::geometry::area(*it);
      my_area += f;
      printf("\t%sarea_i%s(%f)%s\n", KYEL, KBLU, KNRM);
    }

    // calculate area of intersection (of e.g., fire centre poly, and park poly)
    printf("Area of intersection (%e) nbits (%d) i(%d) j(%d)\n\t%s%s%s\n",
    my_area, p_result.size(), i, j, KRED, std::string(my_area<0.000000000001?"NO INTERSECTION":"").c_str(), KNRM);
  }

  //glPopMatrix();

  cout << KGRN << "---------------------------drawAxes()" << endl;
}

/* Callback function for drawing */
void display(void){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawAxes();
  drawText();
  glutSwapBuffers();
  renderflag = false;
}

/* Callback function for pick-event handling from ZPR */

void quitme(){
  exit(0);
}

/* python style split function (from meta4)*/
vector<string> split(const char *str, char c = ' '){
  vector<string> result;
  do{
    const char *begin = str;

    while(*str != c && *str)
    str++;

    result.push_back(string(begin, str));
  }
  while(0 != *str++);
  return result;
}

void special(int key, int x, int y){
  /* Keyboard function */

  switch(key){

    case GLUT_KEY_UP:{
      if(cur_park_ind >= 0){
        cur_park_ind ++;
        if(cur_park_ind >= n_park){
          cur_park_ind = 0;
        }
      }
      else{
        cur_park_ind = 0;
      }
      cout << KGRN << "park(" << KRED << cur_park_ind << KGRN << ")"
      << KMAG << " PROT_NAME " << KGRN << my_names[cur_park_ind + n_fire]
      << endl;
    }
    break;

    case GLUT_KEY_DOWN:{
      if(cur_park_ind >= 0){
        cur_park_ind --;
        if(cur_park_ind < 0){
          cur_park_ind = n_park - 1;
        }
      }
      else{
        cur_park_ind = 0;
      }
      cout << KGRN << "park(" << KRED << cur_park_ind << KGRN << ")"
      << KMAG << " PROT_NAME " << KGRN << my_names[cur_park_ind + n_fire]
      << endl;
    }
    break;

    case GLUT_KEY_LEFT:{
      if(cur_fire_ind >= 0){
        cur_fire_ind --;
        if(cur_fire_ind < 0){
          cur_fire_ind = n_fire - 1;
        }
      }
      else{
        cur_fire_ind = 0;
      }
    }
    break;

    case GLUT_KEY_RIGHT:{
      if(cur_fire_ind >=0){
        cur_fire_ind ++;
        if(cur_fire_ind >= n_fire){
          cur_fire_ind = 0;
        }
      }
      else{
        cur_fire_ind = 0;
      }
    }
    break;
    default: break;
  }

  cout << KRED << "Current Selection: " << KGRN << endl;
  if(cur_fire_ind >= 0){
    poly_info(cur_fire_ind);
  }
  if(cur_park_ind >= 0){
    poly_info(cur_park_ind + n_fire);
  }

  display();
}

void keyboard(unsigned char key, int x, int y){
  /* Keyboard function */

  switch(key){
    case 8 :

    case 127:
    if(console_position > 0){
      console_position --;
      console_string[console_position] = '\0';
      display();
    }
    break;

    // Enter
    case 13: {
      printf("%d Pressed RETURN\n", (char)key);
      string mys(&console_string[0]);
      vector<string> words(split(mys.c_str()));

      if(words.size() == 2){

        if(words[0][0] == 'f'){
          int iii = atoi(words[1].c_str());
          if(iii >=0 && iii < n_fire){
            cur_fire_ind = iii;
          }
          else{
            cur_fire_ind = -1;
          }
        }

        if(words[0][0] == 'p'){
          int iii = atoi(words[1].c_str());
          if(iii >= 0 && iii < n_park){
            cur_park_ind = iii;
          }
          else{
            cur_park_ind = -1;
          }
        }
        cout << "cur_fire_ind " << cur_fire_ind << " cur_park_ind " << cur_park_ind << endl;
      }
      else{
        cout << "command not formed" << endl;
      }
      console_string[0] = '\0';
      console_position = 0;
      display();
    }
    break;

    // Escape
    case 27 : {
      quitme();
      exit(0);
    }
    break;

    default:
    console_string[console_position++] = (char)key;
    console_string[console_position] = '\0';
    display();
    break;
  }
}

void idle(){
  if(renderflag){
    glFlush();
    glutPostRedisplay();
  }
}

int parse(string fn){

  cout << KYEL << "parse(" << KMAG << fn << KYEL << ")" << endl;
  ifstream in(fn.c_str());

  if (!in.is_open()){
    printf("error\n"); exit(1);
  }
  string line;
  getline(in, line);

  cout << KGRN << "\t" << line << KNRM << endl;

  int nclass = 0, i = 0, di = -1;

  while(getline(in, line)){

    di ++;
    vector<vec3d> my_points;
    vector<string> a(split(line.c_str(), ','));
    string feature_name(a[0]);
    string feature_id(a[1].c_str());
    int n_points = atoi(a[2].c_str());
    int i = 0, ci = 3;

    for(i = 0; i < n_points; i++){
      float x = atof((a[ci++]).c_str());
      float y = atof((a[ci++]).c_str());
      my_points.push_back(vec3d(x, y, 0.));
    }

    within_class_index.push_back(di);
    my_vectors.push_back(my_points);
    my_names.push_back(feature_name);
    my_id.push_back(feature_id.c_str());
    my_class.push_back(next_class);
    nclass++;
  }

  for(i = 0; i < nclass; i++){
    n_my_class.push_back(nclass);
  }

  next_class++;
  return nclass; // number of points in this file (associate a class with each file)
}

int parse_JSON(string fn);

int main(int argc, char ** argv){

  cout << KYEL << "BOOST INFO" << endl;
  cout << KGRN << "\tBOOST_VERSION " << KMAG << BOOST_VERSION << endl;
  cout << KGRN << "\tBOOST_PATCH_LEVEL " << KMAG << BOOST_VERSION % 100 << endl;
  cout << KGRN << "\tBOOST_MINOR_VERSION " << KMAG << BOOST_VERSION / 100 << endl;
  cout << KGRN << "\tBOOST_MAJOR_VERSION " << KMAG << BOOST_VERSION / 100000 << endl;
  cout << KGRN << "\tBOOST_LIB_VERSION " << KMAG << BOOST_LIB_VERSION << KNRM << endl;

  max_f = 0.;
  cur_fire_ind = cur_park_ind = -1;

  my_vectors.clear();
  int nc0 = parse(string("firec.dat"));
  int nc1 = parse_JSON(string("TA_PEP_SVW_polygon.json"));
  n_fire = nc0;
  n_park = nc1;
  cout << KGRN << "number of park shp entries " << KRED << nc1 << endl;
  cout << KGRN << "number of fire centre shp entries " << KRED << nc0 << endl << KNRM;

  if(true){

    int i;
    next_class = 0;
    float minx = 0., maxx = 0., miny = 0., maxy = 0.;

    for(i = 0; i < my_vectors.size(); i++){
      vector<vec3d> * j = &my_vectors[i];
      vector<vec3d>::iterator it;
      for(it = j->begin(); it != j->end(); it++){
        float x = (*it).x; float y = (*it).y;
        if(x > maxx) maxx = x;
        if(x < minx) minx = x;
        if(y > maxy) maxy = y;
        if(y < miny) miny = y;
      }
    }

    for(i = 0; i < my_vectors.size(); i++){
      vector<vec3d> * j = &my_vectors[i];
      vector<vec3d>::iterator it;
      for(it = j->begin(); it != j->end(); it++){
        (*it).x -= minx; (*it).x /= (maxx - minx);
        (*it).y -= miny; (*it).y /= (maxy - miny);
      }
    }

    float xa = 0., ya = 0., c = 0.;

    for(i = 0; i < my_vectors.size(); i++){
      vector<vec3d> * j = &my_vectors[i];
      vector<vec3d>::iterator it;
      for(it = j->begin(); it != j->end(); it++){
        xa += (*it).x;
        ya += (*it).y;
        c += 1.;
      }
    }
    xa /= (c);
    ya /= (c);

    for(i = 0; i < my_vectors.size(); i++){
      vector<vec3d> * j = &my_vectors[i];
      vector<vec3d>::iterator it;
      for(it = j->begin(); it != j->end(); it++){
        (*it).x -= xa;
        (*it).y -= ya;
        (*it).x *= 5.;
        (*it).y *= 5.;
      }
    }
  }

  pick = _pick;
  renderflag = false;
  a1 = a2 = a3 = 1;
  console_position = fullscreen = 0;

  /* Initialise olLUT and create a window */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(STARTX,STARTY);
  glutCreateWindow("");
  zprInit();

  /* Configure GLUT callback functions */
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);

  /* glutKeyboardUpFunc(keyboardup); */
  glutIdleFunc(idle);
  glScalef(0.25, 0.25, 0.25);

  /* Configure ZPR module */
  zprSelectionFunc(drawAxes); /* Selection mode draw function */
  zprPickFunc(pick); /* Pick event client callback */

  /* Enter GLUT event loop */
  glutMainLoop();
  return 0;
}

int parse_JSON(string fn){
  long int c = 0;

  cout << KYEL << "parse(" << KMAG << fn << KYEL << ") (JSON)" << endl;

  long int nclass = 0; // number of features in this class

  bool DEBUG = false;
  vector<string> ORC_PRIMRY; /* primary ORC */
  vector<string> PROT_NAME; /* park name */
  vector<string> COORDS; /* GIS coordinate string (WKT format) */

  long int fs = getFileSize(fn);
  char * fd = (char *)(void *)malloc(fs);
  memset(fd, '\0', fs);
  if(DEBUG) printf("load file\n");
  FILE * f = fopen(fn.c_str(), "rb");
  long int br = fread(fd, 1, fs, f);
  if(br != fs){
    printf("Error: br (%ld)!= fs (%ld)\n", (long int)br, (long int)fs);
    exit(1);
  }
  long int i;
  long int f2 = fs;
  for(i = 0; i < fs; i++){
    if(fd[i] == '\n'){
      f2 -= 1;
    }
  }
  if(DEBUG) cout << fs << endl << f2 << endl;

  char * fd2 = (char *)(void *)malloc(f2);
  memset(fd2, '\0', f2);

  if(DEBUG) printf("remove newline\n");
  long int ci = 0;
  for(i = 0; i < fs; i++){
    if(fd[i] != '\n'){
      fd2[ci++] = fd[i];
    }
  }

  if(DEBUG) printf("parsing json...\n");
  Document document;
  document.Parse((const char *)(void *)fd2);
  assert(document.IsObject());

  static const char* kTypeNames[] = {
    "Null",
    "False",
    "True",
    "Object",
    "Array",
    "String",
    "Number"
  };

  /* for all doc members */
  for(Value::ConstMemberIterator itr = document.MemberBegin();
  itr != document.MemberEnd();
  ++itr){

    if(DEBUG) printf("%s%s%s%s (%s%s%s)\n", KGRN, KRED, itr->name.GetString(),
    KGRN, KYEL, kTypeNames[itr->value.GetType()], KGRN);

    /* assert(itr->IsArray()); */
    if(!strncmp("Array\0", kTypeNames[itr->value.GetType()], 5)){

      const Value& a = itr->value;

      /* for all members in a */
      for(Value::ConstValueIterator itr2 = a.Begin();
      itr2 != a.End();
      ++itr2){

        /* THIS IS THE LOOP FOR ITERATING OVER FEATURES...

        E.G., need an array for points declared here..
        */
        vector<vec3d> my_points; // from test.cpp
        std::string feature_name("");
        std::string feature_id("");
        double x, y;

        /* temporary: only show first 2 parks */
        if(c > 1) break;

        if(false && c > 1){
          if(ORC_PRIMRY.size() != PROT_NAME.size() ||
          ORC_PRIMRY.size() != COORDS.size() ||
          PROT_NAME.size() != COORDS.size()){
            printf("error:\n\tlen(1)=%ld len(2)=%ld len(3)=%ld\n", ORC_PRIMRY.size(), PROT_NAME.size(), COORDS.size());
          }
          else{
            printf("done\n");
          }
          exit(1);
        }

        long int i = 0;

        /* to hold the coordinates */
        std::string my_coord("POLYGON((");

        /* feature(poly) counter */
        if(DEBUG){
          printf("feature(%ld) %s\n", (long int)c, itr2->IsObject()?"true":"false");
        }

        itr2->MemberBegin();

        /* for all the members in iter2 */
        for(Value::ConstMemberIterator itr3 = itr2->MemberBegin();
        itr3 != itr2->MemberEnd();
        ++itr3){

          if(DEBUG) printf("\t%s%s%s%s (%s%s%s)\n", KGRN, KRED, itr3->name.GetString(),
          KGRN, KYEL, kTypeNames[itr3->value.GetType()], KGRN);
          /* Type of member geometry: Object */

          if(!strncmp("String\0", kTypeNames[itr3->value.GetType()], 5)){
            if(DEBUG) printf("\t\t\t%s\n", itr3->value.GetString());
          }

          if(!strncmp("Object\0", kTypeNames[itr3->value.GetType()], 5)){
            if(!strncmp("properties\0", itr3->name.GetString(), 10)){
              for(Value::ConstMemberIterator itr4 = itr3->value.MemberBegin();
              itr4 != itr3->value.MemberEnd();
              ++itr4){

                /* don't show unused fields */
                if(false) printf("\t\t%s%s%s%s (%s%s%s)\n", KGRN, KRED, itr4->name.GetString(),
                KGRN, KYEL, kTypeNames[itr3->value.GetType()], KGRN);

                if(!strncmp("ORC_PRIMRY\0", itr4->name.GetString(), 10)){
                  if(DEBUG) printf("\t\t\tORC_PRIMRY=%s\n", itr4->value.GetString());
                  feature_id = itr4->value.GetString();
                  ORC_PRIMRY.push_back(itr4->value.GetString());
                }

                if(!strncmp("PROT_NAME\0", itr4->name.GetString(), 9)){
                  if(DEBUG) printf("\t\t\tPROT_NAME=%s\n", itr4->value.GetString());
                  feature_name = itr4->value.GetString();
                  PROT_NAME.push_back(itr4->value.GetString());
                  cout << KYEL << "PROT_NAME " << KGRN << feature_name << endl;
                }
              }

            }
            if(!strncmp("geometry\0", itr3->name.GetString(), 8)){

              for(Value::ConstMemberIterator itr4 = itr3->value.MemberBegin();
              itr4 != itr3->value.MemberEnd();
              ++itr4){

                if(DEBUG) printf("\t\t%s%s%s%s (%s%s%s)\n", KGRN, KRED, itr4->name.GetString(),
                KGRN, KYEL, kTypeNames[itr4->value.GetType()], KGRN);

                /* Type of member "type": Object */
                /* Type of member "coordinates": Object */

                if(!strncmp("coordinates\0", itr4->name.GetString(), 10)){

                  /* assert(itr->IsArray()); */
                  if(!strncmp("Array\0", kTypeNames[itr4->value.GetType()], 5)){

                    /* iterate the coordinate array */
                    for (Value::ConstValueIterator itr5 = itr4->value.Begin();
                    itr5 != itr4->value.End();
                    ++itr5){
                      itr5->GetType();

                      if(!strncmp("Array\0", kTypeNames[itr5->GetType()], 5)){

                        for(Value::ConstValueIterator itr6 = itr5->Begin();
                        itr6 != itr5->End();
                        ++itr6){

                          if(!strncmp("Array\0", kTypeNames[itr6->GetType()], 5)){

                            long int number_index = 0;

                            for(Value::ConstValueIterator itr7 = itr6->Begin();
                            itr7 != itr6->End();
                            ++itr7){
                              i++;

                              itr7->GetType();
                              if(!itr7->IsDouble()){
                                for(Value::ConstValueIterator itr8 = itr7->Begin();
                                itr8 != itr7->End();
                                ++itr8){
                                  itr8->GetType();
                                  if(!itr8->IsDouble()){
                                    printf("%sError: !IsDouble()\n", KGRN);
                                    exit(1);
                                  }
                                  else{

                                    ++number_index;

                                    if(number_index %3 != 0){

                                      if((number_index % 3 == 1) && (i > 3)){
                                        my_coord += std::string(",");
                                      }

                                      // extra
                                      if(number_index % 3 == 1){
                                        x = itr8->GetDouble();
                                      }

                                      if(number_index % 3 == 2){
                                        y = itr8->GetDouble();
                                        my_points.push_back(vec3d(x, y, 0.));
                                        my_coord += std::string(" ");
                                      }

                                      my_coord += dtos(itr8->GetDouble());
                                    }
                                  }
                                }
                              }
                              else{

                                ++number_index;

                                if(number_index %3 != 0){

                                  //same
                                  if((number_index % 3 == 1) && (i > 3)){
                                    my_coord += std::string(",");
                                  }

                                  // extra
                                  if(number_index % 3 == 1){
                                    x = itr7->GetDouble();
                                  }

                                  //different
                                  if(number_index % 3 == 2){
                                    y = itr7->GetDouble();
                                    my_points.push_back(vec3d(x, y, 0.));
                                    my_coord += std::string(" ");
                                  }

                                  //same
                                  my_coord += dtos(itr7->GetDouble());
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            //geometry
          }
          //object
        }

        my_coord += std::string("))");
        if(DEBUG) cout << KMAG << "\t\t" << my_coord << KGRN << endl;
        COORDS.push_back(my_coord);

        /* end of feature-- save data */
        cout << "class " << c << endl;
        within_class_index.push_back(c++);
        my_vectors.push_back(my_points);
        my_names.push_back(feature_name);
        my_id.push_back(feature_id);
        my_class.push_back(next_class);
        nclass++;

      }
    }
  }
  long int ii = 0;
  for(ii = 0; ii < nclass; ii++){
    n_my_class.push_back(nclass);
  }

  if(ORC_PRIMRY.size() != PROT_NAME.size() ||
  ORC_PRIMRY.size() != COORDS.size() ||
  PROT_NAME.size() != COORDS.size()){
    printf("error:\n\tlen(1)=%ld len(2)=%ld len(3)=%ld\n",
    ORC_PRIMRY.size(), PROT_NAME.size(), COORDS.size());
  }
  else{
    printf("%sdone\n", KGRN);
  }
  // go to next class
  next_class++;

  return nclass;
}

int v(std:: string fn){

  cout << KYEL << "parse(" << KMAG << fn << KYEL << ")" << endl;
  ifstream in(fn.c_str());

  if (!in.is_open()){
    printf("error\n"); exit(1);
  }

  string line;
  getline(in, line);

  cout << KGRN << "\t" << line << KNRM << endl;

  int nclass = 0, di = -1; // di -- feature index

  while(getline(in, line)){
    di ++;

    // array of points for each feature...
    vector<vec3d> my_points;
    vector<string> a(split(line.c_str(),','));

    //FEATURE_NAME, FEATURE_ID, N_POINTS, POINTS..
    string feature_name(a[0]);
    string feature_id(a[1].c_str());

    int n_points = atoi(a[2].c_str());
    int i, ci;
    ci = 3;
    for(i=0; i<n_points; i++){
      float x = atof((a[ci++]).c_str()), y = atof((a[ci++]).c_str());
      my_points.push_back(vec3d(x,y,0.));
    }
    within_class_index.push_back(di);
    my_vectors.push_back(my_points);
    my_names.push_back(feature_name);
    my_id.push_back(feature_id);
    my_class.push_back(next_class);
    nclass++;
  }
  int i;
  for(i = 0; i < nclass; i++){
    n_my_class.push_back(nclass);
  }
  next_class ++;
  return nclass; // number of points in this file (associate a class with each file)
}
