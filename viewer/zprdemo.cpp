#include"ansicolor.h"
#include <iostream>
#include <deque>
#include<fstream>
#include<sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include"ansicolor.h"
#include"rapidjson/document.h"

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

/*
http://www.boost.org/doc/libs/1_47_0/libs/geometry/doc/html/geometry/reference/algorithms/append.html
http://www.boost.org/doc/libs/1_47_0/libs/geometry/doc/html/geometry/reference/algorithms/intersection.html
http://www.boost.org/doc/libs/1_65_0/libs/geometry/doc/html/geometry/reference/algorithms/intersection/intersection_3.html
*/
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
vector< string > my_names;
vector<int> my_id;
vector<int> my_class;
vector<int> n_my_class;
vector<int> within_class_index;
vector<int> urx;

vector<vec3d> max_p;
double max_f;

int next_class;

int cur_fire_ind, cur_park_ind;
int n_fire, n_park;

std::string * orc_to_name;

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
  if(!(i.is_open())){
    printf("Error: was unable to open file: %s\n", (fn.c_str()));
    return -1;
  }
  i.seekg(0, ios::end);
  long int len = i.tellg();
  return(len);
}

void _pick(GLint name){
  if(myPickNames.size() < 1){
    return;
  }
  cout << "PickSet:" << endl;
  std::set<GLint>::iterator it;
  for(it=myPickNames.begin(); it!=myPickNames.end(); it++){
    int my_ind = *it;
    if(my_class[my_ind] ==1){
      my_ind -= n_fire;
    }
    cout << KYEL << "\t" << my_names[my_ind] << KMAG << "--> " << KYEL << "(" << KGRN << my_ind << KYEL << ")" << KGRN << ",";

    if(my_class[my_ind] == 1){
      cout << endl << "\t\t" << my_names[my_ind] << " my_id:" << my_id[my_ind] << endl;
    }
    cout << endl;
  }
  cout << KNRM << endl;
  fflush(stdout);
}

void renderBitmapString(float x, float y, void *font, char *string){
  char *c;
  glRasterPos2f(x,y);
  for (c=string; *c != '\0'; c++){
    glutBitmapCharacter(font, *c);
  }
}

//http://www.codeproject.com/Articles/80923/The-OpenGL-and-GLUT-A-Powerful-Graphics-Library-an
void setOrthographicProjection() {
  int h = WINDOWY;
  int w = WINDOWX;
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  glScalef(1, -1, 1);
  glTranslatef(0, -h, 0);
  glMatrixMode(GL_MODELVIEW);
}

void resetPerspectiveProjection() {
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void drawText(){
  glColor3f(0.0f,1.0f,0.0f);
  setOrthographicProjection();
  glPushMatrix();
  glLoadIdentity();
  int lightingState = glIsEnabled(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  renderBitmapString(3,WINDOWY-3,(void *)MYFONT,console_string);
  if(lightingState) glEnable(GL_LIGHTING);
  glPopMatrix();
  resetPerspectiveProjection();
}

/* convert float to string.. from gift meta4::gift */
string ftos( float i){
  std::string number("");
  std::stringstream strstream;
  strstream << i;
  strstream >> number;
  return number;
}

float a1, a2, a3;

void drawAxes(void){
  glPushMatrix();
  float r, g, b;
  r =0.; g = b = 1.;
  glColor3f(r,g,b);
  glPointSize(1.);
  int picki = -1;
  if( myPickNames.size() ==1){
    picki =*( myPickNames.begin());
    cout << picki << "," << my_names[picki] << ",myclass= " << my_class[picki] << "n_my_class" << n_my_class[picki] << endl;
    cout << "within_class_index" << within_class_index[picki]<<endl;
  }
  int i;
  int ci=0;
  int n_labels = my_vectors.size();
  for(i=0; i< n_labels; i++){
    if(my_class[i] ==0){
      if(cur_fire_ind>=0 && cur_fire_ind < n_fire){
        if(cur_fire_ind != within_class_index[i]){
          continue;
        }
      }
    }
    if(my_class[i] ==1){
      if(cur_park_ind>=0 && cur_park_ind < n_park){
        if(cur_park_ind != within_class_index[i]){
          continue;
        }
      }
    }
    float myf = ((float)((within_class_index[i]+1))) / ((float) n_my_class[i]);
    hsv2rgb(&r, &g, &b, 360. * myf, my_class[i]==1?1.:0.5, my_class[i]==1?1.:0.15);
    vector<vec3d> * j = &my_vectors[i];
    vector<vec3d>::iterator it;
    ci = 0;
    if(picki==i){
      glColor3f(1.-r, 1.-g, 1.-b);
    }
    else{
      glColor3f(r,g,b);
    }
    glPushName(i);
    glBegin(GL_POLYGON);
    for(it=j->begin(); it!=j->end(); it+=1){
      if(true){
        //ci % 2 ==0 ?
        if(picki==i && ci==0){
          zprReferencePoint[0] =(*it).x;
          zprReferencePoint[1] =(*it).y;
        }
        glVertex3f((*it).x, (*it).y, (*it).z);
      }
      ci ++;
    }
    glEnd();
    glPopName();
  }
  if(cur_fire_ind>=0 && cur_fire_ind < n_fire && cur_park_ind>=0 && cur_park_ind < n_park){
    int k;

    using boost::assign::tuple_list_of;
    using boost::make_tuple;
    using boost::geometry::append;
    typedef boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double> > polygon;

    polygon f_poly; float a, b, c, d; a=b=c=d=0.;
    int i = cur_fire_ind;
    long int slen = my_vectors[i].size(); long int sskip = slen / 256;
    vector<vec3d> * v = &my_vectors[i];
    printf("sskip %ld\n", sskip);
    string wkt_f("POLYGON((");
    int add_s = 0;
    for(k=0; k< slen; k++){
      vec3d x(v->at(k));

      if(k % sskip == 0){
        add_s +=1;
        if(k>0){
          wkt_f += ",";
        }
        wkt_f += ftos(x.x);
        wkt_f += " ";
        wkt_f += ftos(x.y);
        //append(f_poly, make_tuple(x.x, x.y));
        //append wktstring command
      }
      if(k<10){
        printf("%e %e\n", x.x, x.y);
        if(k==0){
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
    if(wkt_f.length() < 999){
      cout << wkt_f << endl;
    }

    printf("%sread_wkt%s()%s n%s=%s(%s%ld%s)%s from %ld\n",
    KYEL, KBLU, KGRN, KYEL, KRED, KMAG, add_s, KRED, KNRM, slen);

    boost::geometry::read_wkt(wkt_f, f_poly);
    printf("%scorrect%s()%s\n", KYEL, KBLU, KNRM);
    boost::geometry::correct(f_poly);
    printf("%sf_poly %si(%d) x(%f, %f) y(%f, %f)\n", KMAG, KNRM, i, a, b, c, d);
    //add first point to end?

    glColor3f(1., 0., 0.);
    glBegin(GL_LINES);
    glVertex3f(a,d,0); glVertex3f(b,d,0);
    glVertex3f(b,d,0); glVertex3f(b,c,0);
    glVertex3f(b,c,0); glVertex3f(a,c,0);
    glVertex3f(a,c,0); glVertex3f(a,d,0);
    glEnd();

    polygon p_poly; a = b = c = d = 0.;
    int j = cur_park_ind + n_fire; // this is the object index: remember, everything's lumped together in one array (fire centres, first)
    long int clen = my_vectors[j].size(); long int cskip = clen / 256;
    v = &my_vectors[j];
    printf("cskip %ld\n", cskip);
    string wkt_p("POLYGON((");
    int add_c = 0;
    for(k=0; k< clen; k++){
      vec3d x(v->at(k));

      if(k % cskip == 0){
        add_c += 1;
        if(k>0){
          wkt_p += ",";
        }
        wkt_p += ftos(x.x);
        wkt_p += " ";
        wkt_p += ftos(x.y);
      }
      if(k<10){
        printf("%e %e\n", x.x, x.y);
        if(k==0){
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
    if(wkt_p.length() < 999){
      cout << wkt_p << endl;
    }

    printf("%sread_wkt%s()%s n%s=%s(%s%ld%s)%s from %ld\n",
    KYEL, KBLU, KGRN, KYEL, KRED, KMAG, add_c, KRED, KNRM, clen);

    boost::geometry::read_wkt(wkt_p, p_poly);
    printf("%scorrect%s()%s\n", KYEL, KBLU, KNRM);
    boost::geometry::correct(p_poly);
    printf("%sp_poly %si(%d) x(%f, %f) y(%f, %f)\n", KMAG, KNRM, i, a, b, c, d);
    //add first point to the end?

    glColor3f(0., 0., 1.);
    glBegin(GL_LINES);
    glVertex3f(a,d,0); glVertex3f(b,d,0);
    glVertex3f(b,d,0); glVertex3f(b,c,0);
    glVertex3f(b,c,0); glVertex3f(a,c,0);
    glVertex3f(a,c,0); glVertex3f(a,d,0);
    glEnd();

    std::deque<polygon> p_result;

    printf("%sintersection%s()%s\n", KYEL, KBLU, KNRM);
    boost::geometry::intersection(f_poly, p_poly, p_result);
    printf("%sintersection%s(%ld)%s\n", KYEL, KBLU, p_result.size(), KNRM);
    std::deque<polygon>::iterator it;
    printf("%sarea%s()%s\n", KYEL, KBLU, KNRM);
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

  glPopMatrix();
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
  while (0 != *str++);
  return result;
}

void setup();

/* Keyboard functions */
void special(int key, int x, int y){
  switch(key){
    case GLUT_KEY_LEFT:{
      if(cur_fire_ind >=0){
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
  display();

}

/* Keyboard functions */
void keyboard(unsigned char key, int x, int y){
  switch(key){

    case 8 :
    case 127:
    if(console_position>0){
      console_position --;
      console_string[console_position]='\0';
      display();
    }
    break;

    // Enter
    case 13 :{
      printf( "%d Pressed RETURN\n",(char)key);

      string mys(&console_string[0]);

      vector<string> words(split(mys.c_str()));
      if(words.size()==2){
        cout << "good "<<endl;
        if(words[0][0]=='f'){
          int iii = atoi(words[1].c_str());
          if(iii >=0 && iii < n_fire){
            cur_fire_ind = iii;
          }
          else{
            cur_fire_ind = -1;
          }
        }
        if(words[0][0]=='p'){
          int iii=atoi(words[1].c_str());
          if(iii>=0 && iii< n_park){
            cur_park_ind = iii;
          }
          else{
            cur_park_ind = -1;
          }
        }
        cout <<"cur_fire_ind " << cur_fire_ind<< " cur_park_ind " << cur_park_ind <<endl;
      }

      console_string[0]='\0';
      console_position=0;
      display();
    }
    break;

    // Escape
    case 27 :
    quitme();
    exit(0);
    break;

    default:
    //printf( "Pressed key %c AKA %d at position %d % d\n",(char)key, key, x, y);
    console_string[console_position++] = (char)key;
    console_string[console_position]='\0';
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
  int nclass = 0;
  int di = -1;
  while(getline(in, line)){
    di ++;
    vector<vec3d> my_points;
    vector<string> a(split(line.c_str(),','));
    //FEATURE_NAME, FEATURE_ID, N_POINTS, POINTS..
    string feature_name(a[0]);
    int feature_id = atoi(a[1].c_str());
    int n_points = atoi(a[2].c_str());
    int i, ci;
    ci = 3;
    for(i=0; i<n_points; i++){
      float x = atof((a[ci++]).c_str());
      float y = atof((a[ci++]).c_str());
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
  for(i=0; i<nclass; i++){
    n_my_class.push_back(nclass);
  }
  next_class ++;
  return nclass; // number of points in this file (associate a class with each file)
}

void loadJSON(vector<string> & ORC_PRIMRY, vector<string> & PROT_NAME, vector<string> & COORDS){

}

int main(int argc, char *argv[]){
  orc_to_name = NULL;

  setup();
  cout << KYEL << "BOOST INFO"<<endl;
  cout << KGRN << "\tBOOST_VERSION " << KMAG << BOOST_VERSION << endl;
  cout << KGRN << "\tBOOST_PATCH_LEVEL " << KMAG << BOOST_VERSION % 100 << endl;
  cout << KGRN << "\tBOOST_MINOR_VERSION " << KMAG << BOOST_VERSION / 100 << endl;
  cout << KGRN << "\tBOOST_MAJOR_VERSION " << KMAG << BOOST_VERSION / 100000 << endl;
  cout << KGRN << "\tBOOST_LIB_VERSION " << KMAG << BOOST_LIB_VERSION << KNRM << endl;

  max_f = 0.;
  cur_fire_ind = -1;
  cur_park_ind = -1;

  FILE * fff = fopen("ind.txt", "wb");
  FILE * ggg = fopen("mat.txt", "ab");
  if(!ggg){
    cout << "Error: could not open file mat.dat" <<endl;
  }
  my_vectors.clear();
  int nc0 = parse(string("firec.dat"));
  int nc1 = parse(string("parks.dat"));
  n_fire = nc0;
  n_park = nc1;
  cout << KGRN << "number of park shp entries " << KRED << nc1 << endl;
  cout << KGRN << "number of fire centre shp entries " << KRED << nc0 << endl << KNRM;
  fclose(fff);
  fclose(ggg);

  /* load from JSON */
  vector<string> ORC_PRIMRY; /* primary ORC */
  vector<string> PROT_NAME; /* park name */
  vector<string> COORDS; /* GIS coordinate string (WKT format) */
  loadJSON(ORC_PRIMRY, PROT_NAME, COORDS);

  if(true){
    int i;
    next_class = 0;
    float minx, maxx, miny, maxy;
    minx = maxx = miny = maxy = 0.;
    for(i=0; i< my_vectors.size(); i++){
      vector<vec3d> * j = &my_vectors[i];
      vector<vec3d>::iterator it;
      for(it = j->begin(); it != j->end(); it++){
        float x = (*it).x; float y = (*it).y;
        if(x > maxx) maxx=x;
        if(x < minx) minx=x;
        if(y > maxy) maxy=y;
        if(y < miny) miny=y;
      }
    }

    for(i=0; i< my_vectors.size(); i++){
      vector<vec3d> * j = &my_vectors[i];
      vector<vec3d>::iterator it;
      for(it=j->begin(); it!=j->end(); it++){
        (*it).x -= minx; (*it).x /= (maxx - minx);
        (*it).y -= miny; (*it).y /= (maxy - miny);
      }
    }

    float xa =0.; float ya=0.; float c = 0.;
    for(i=0; i< my_vectors.size(); i++){
      vector<vec3d> * j = &my_vectors[i];
      vector<vec3d>::iterator it;
      for(it=j->begin(); it!=j->end(); it++){
        xa += (*it).x;
        ya += (*it).y;
        c += 1.;
      }
    }

    xa /= (c);
    ya /= (c);

    for(i=0; i< my_vectors.size(); i++){
      vector<vec3d> * j = &my_vectors[i];
      vector<vec3d>::iterator it;
      for(it=j->begin(); it!=j->end(); it++){
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
  console_position = 0;
  fullscreen=0;

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
  // glutKeyboardUpFunc(keyboardup);
  glutIdleFunc(idle);
  glScalef(0.25,0.25,0.25);

  /* Configure ZPR module */
  zprSelectionFunc(drawAxes); /* Selection mode draw function */
  zprPickFunc(pick); /* Pick event client callback */

  /* Enter GLUT event loop */
  glutMainLoop();
  return 0;
}

void setup(){

  long int mk = 9954;
  long int nb = sizeof(std::string *) * (mk + 1);
  //std::string * orc_to_name
  orc_to_name = (std::string *)(void *)malloc(nb);
  memset(orc_to_name, '\0', nb);
  orc_to_name[1] = std::string("Strathcona Megin/Talbot");
  orc_to_name[2] = std::string("Mount Robson Provincial Park");
  orc_to_name[3] = std::string("John Dean Provincial Park");
  orc_to_name[4] = std::string("Kokanee Glacier Provincial Park");
  orc_to_name[5] = std::string("Mount Assiniboine Provincial Park");
  orc_to_name[6] = std::string("Sir Alexander Mackenzie Provincial Park");
  orc_to_name[7] = std::string("Garibaldi Provincial Park");
  orc_to_name[8] = std::string("Golden Ears Provincial Park");
  orc_to_name[9] = std::string("Sooke Mountain Provincial Park");
  orc_to_name[10] = std::string("Inonoaklin Provincial Park");
  orc_to_name[11] = std::string("Keremeos Columns Provincial Park");
  orc_to_name[12] = std::string("Lockhart Beach Provincial Park");
  orc_to_name[13] = std::string("Dead Man&#8217;s Island Provincial Park");
  orc_to_name[14] = std::string("Beatton Provincial Park");
  orc_to_name[15] = std::string("Mount Seymour Provincial Park");
  orc_to_name[16] = std::string("Swan Lake Provincial Park");
  orc_to_name[17] = std::string("King George VI Provincial Park");
  orc_to_name[18] = std::string("Tweedsmuir North Provincial Park");
  orc_to_name[19] = std::string("Tweedsmuir South Provincial Park");
  orc_to_name[20] = std::string("Mara Provincial Park");
  orc_to_name[21] = std::string("Mount Maxwell Provincial Park");
  orc_to_name[22] = std::string("Nickel Plate Provincial Park");
  orc_to_name[23] = std::string("Peace Arch Provincial Park");
  orc_to_name[24] = std::string("Wells Gray Provincial Park: Mahood Lake");
  orc_to_name[25] = std::string("Premier Lake Provincial Park");
  orc_to_name[26] = std::string("Chasm Provincial Park");
  orc_to_name[27] = std::string("Silver Star Provincial Park");
  orc_to_name[28] = std::string("Elk Falls Provincial Park and Protected Area");
  orc_to_name[29] = std::string("Englishman River Falls Provincial Park");
  orc_to_name[30] = std::string("Little Qualicum Falls Provincial Park");
  orc_to_name[31] = std::string("Stamp River Provincial Park");
  orc_to_name[32] = std::string("Wendle Provincial Park");
  orc_to_name[33] = std::string("E.C. Manning Provincial Park");
  orc_to_name[34] = std::string("Hamber Provincial Park");
  orc_to_name[35] = std::string("Darke Lake Provincial Park");
  orc_to_name[36] = std::string("Kitty Coleman Beach Provincial Park");
  orc_to_name[37] = std::string("Memory Island Provincial Park");
  orc_to_name[38] = std::string("Kitsumkalum Provincial Park");
  orc_to_name[39] = std::string("MacMillan Provincial Park");
  orc_to_name[40] = std::string("Roberts Creek Provincial Park");
  orc_to_name[41] = std::string("Cultus Lake Provincial Park");
  orc_to_name[43] = std::string("Petroglyph Provincial Park");
  orc_to_name[44] = std::string("Beaver Point Provincial Park");
  orc_to_name[45] = std::string("Miracle Beach Provincial Park");
  orc_to_name[46] = std::string("Monck Provincial Park");
  orc_to_name[47] = std::string("Ethel F. Wilson Memorial Provincial Park");
  orc_to_name[48] = std::string("Fillongley Provincial Park");
  orc_to_name[49] = std::string("Apodaca Provincial Park");
  orc_to_name[50] = std::string("Maquinna Marine Provincial Park");
  orc_to_name[51] = std::string("Champion Lakes Provincial Park");
  orc_to_name[52] = std::string("Kokanee Creek Provincial Park");
  orc_to_name[53] = std::string("Wasa Lake Provincial Park");
  orc_to_name[54] = std::string("Okanagan Lake Provincial Park");
  orc_to_name[55] = std::string("Bijoux Falls Provincial Park");
  orc_to_name[56] = std::string("Boundary Creek Provincial Park");
  orc_to_name[57] = std::string("Bridge Lake Provincial Park");
  orc_to_name[58] = std::string("Bromley Rock Provincial Park");
  orc_to_name[59] = std::string("Canim Beach Provincial Park");
  orc_to_name[60] = std::string("Cottonwood River Provincial Park");
  orc_to_name[61] = std::string("Dry Gulch Provincial Park");
  orc_to_name[62] = std::string("Exchamsiks River Provincial Park");
  orc_to_name[63] = std::string("Goldpan Provincial Park");
  orc_to_name[64] = std::string("Inkaneep Provincial Park");
  orc_to_name[65] = std::string("Jimsmith Lake Provincial Park");
  orc_to_name[66] = std::string("Johnstone Creek Provincial Park");
  orc_to_name[67] = std::string("Kleanza Creek Provincial Park");
  orc_to_name[68] = std::string("Lac La Hache Provincial Park");
  orc_to_name[69] = std::string("Lac Le Jeune Provincial Park");
  orc_to_name[70] = std::string("Lakelse Lake Provincial Park");
  orc_to_name[71] = std::string("Monte Lake Provincial Park");
  orc_to_name[72] = std::string("Nicolum River Provincial Park");
  orc_to_name[73] = std::string("sx&#780;&#695;&#601;x&#780;&#695;nitk&#695; Provincial Park");
  orc_to_name[74] = std::string("Seeley Lake Provincial Park");
  orc_to_name[75] = std::string("Skihist Provincial Park");
  orc_to_name[76] = std::string("Stemwinder Provincial Park");
  orc_to_name[77] = std::string("Vaseux Lake Provincial Park");
  orc_to_name[78] = std::string("Whiskers Point Provincial Park");
  orc_to_name[79] = std::string("Yahk Provincial Park");
  orc_to_name[80] = std::string("Yard Creek Provincial Park");
  orc_to_name[81] = std::string("Emory Creek Provincial Park");
  orc_to_name[82] = std::string("Loon Lake Provincial Park");
  orc_to_name[84] = std::string("Tyhee Lake Provincial Park");
  orc_to_name[85] = std::string("Cinnemousun Narrows Provincial Park");
  orc_to_name[86] = std::string("Echo Lake Provincial Park");
  orc_to_name[87] = std::string("Rosewall Creek Provincial Park");
  orc_to_name[89] = std::string("Shuswap Lake Provincial Park");
  orc_to_name[90] = std::string("Alice Lake Provincial Park");
  orc_to_name[92] = std::string("Liard River Hot Springs Provincial Park");
  orc_to_name[93] = std::string("Muncho Lake Provincial Park");
  orc_to_name[94] = std::string("Stone Mountain Provincial Park");
  orc_to_name[95] = std::string("Skookumchuck Narrows Provincial Park");
  orc_to_name[96] = std::string("Goldstream Provincial Park");
  orc_to_name[98] = std::string("Norbury Lake Provincial Park");
  orc_to_name[102] = std::string("Elko Provincial Park");
  orc_to_name[104] = std::string("Montague Harbour Marine Provincial Park");
  orc_to_name[105] = std::string("Mount Fernie Provincial Park");
  orc_to_name[106] = std::string("Koksilah River Provincial Park");
  orc_to_name[108] = std::string("Moyie Lake Provincial Park");
  orc_to_name[109] = std::string("Rebecca Spit Marine Provincial Park");
  orc_to_name[110] = std::string("Rosebery Provincial Park");
  orc_to_name[111] = std::string("Ruth Lake Provincial Park");
  orc_to_name[112] = std::string("Ryan Provincial Park");
  orc_to_name[113] = std::string("Chemainus River Provincial Park");
  orc_to_name[114] = std::string("Thunder Hill Provincial Park");
  orc_to_name[115] = std::string("Beaumont Provincial Park");
  orc_to_name[116] = std::string("Plumper Cove Marine Provincial Park");
  orc_to_name[117] = std::string("Bamberton Provincial Park");
  orc_to_name[118] = std::string("Gabriola Sands Provincial Park");
  orc_to_name[119] = std::string("Allison Lake Provincial Park");
  orc_to_name[120] = std::string("Crowsnest Provincial Park");
  orc_to_name[121] = std::string("Elk Valley Provincial Park");
  orc_to_name[122] = std::string("Rolley Lake Provincial Park");
  orc_to_name[123] = std::string("Victor Lake Provincial Park");
  orc_to_name[124] = std::string("Chilliwack River Provincial Park");
  orc_to_name[127] = std::string("Paul Lake Provincial Park");
  orc_to_name[129] = std::string("Bowron Lake Provincial Park");
  orc_to_name[130] = std::string("Marl Creek Provincial Park");
  orc_to_name[131] = std::string("Mitlenatch Island Nature Provincial Park");
  orc_to_name[133] = std::string("Newcastle Island Marine Provincial Park");
  orc_to_name[135] = std::string("Cedar Point Provincial Park");
  orc_to_name[136] = std::string("Ten Mile Lake Provincial Park");
  orc_to_name[137] = std::string("Eves Provincial Park");
  orc_to_name[139] = std::string("Ellison Provincial Park");
  orc_to_name[140] = std::string("Kiskatinaw Provincial Park");
  orc_to_name[141] = std::string("Murrin Provincial Park");
  orc_to_name[142] = std::string("s&#7809;i&#7809;s Provincial Park");
  orc_to_name[143] = std::string("Monashee Provincial Park");
  orc_to_name[144] = std::string("Morrissey Provincial Park");
  orc_to_name[145] = std::string("Saltery Bay Provincial Park");
  orc_to_name[146] = std::string("Otter Lake Provincial Park");
  orc_to_name[150] = std::string("Davis Lake Provincial Park");
  orc_to_name[151] = std::string("Ferry Island Provincial Park");
  orc_to_name[152] = std::string("Birkenhead Lake Provincial Park");
  orc_to_name[153] = std::string("Rock Creek Provincial Park");
  orc_to_name[154] = std::string("Spectacle Lake Provincial Park");
  orc_to_name[155] = std::string("Ballingall Islets Ecological Reserve");
  orc_to_name[156] = std::string("Summit Lake Provincial Park");
  orc_to_name[158] = std::string("Silver Lake Provincial Park");
  orc_to_name[159] = std::string("Hyland River Provincial Park");
  orc_to_name[160] = std::string("Topley Landing Provincial Park");
  orc_to_name[161] = std::string("Charlie Lake Provincial Park");
  orc_to_name[162] = std::string("Prudhomme Lake Provincial Park");
  orc_to_name[163] = std::string("Pilot Bay Provincial Park");
  orc_to_name[164] = std::string("Stagleap Provincial Park");
  orc_to_name[165] = std::string("Bellhouse Provincial Park");
  orc_to_name[166] = std::string("Bridal Veil Falls Provincial Park");
  orc_to_name[167] = std::string("White Lake Provincial Park");
  orc_to_name[169] = std::string("Beaver Creek Provincial Park");
  orc_to_name[170] = std::string("Cariboo Nature Provincial Park");
  orc_to_name[172] = std::string("Burges James Gadsen Provincial Park");
  orc_to_name[173] = std::string("Princess Louisa Marine Provincial Park");
  orc_to_name[174] = std::string("Erie Creek Provincial Park");
  orc_to_name[177] = std::string("Crooked River Provincial Park");
  orc_to_name[178] = std::string("Boya Lake Provincial Park");
  orc_to_name[179] = std::string("Nairn Falls Provincial Park");
  orc_to_name[180] = std::string("Wood Mountain Ski Provincial Park");
  orc_to_name[181] = std::string("Moberly Lake Provincial Park");
  orc_to_name[182] = std::string("Sproat Lake Provincial Park");
  orc_to_name[183] = std::string("Marble Canyon Provincial Park");
  orc_to_name[185] = std::string("Cody Caves Provincial Park");
  orc_to_name[186] = std::string("Arbutus Grove Provincial Park");
  orc_to_name[187] = std::string("Helliwell Provincial Park");
  orc_to_name[188] = std::string("Kin Beach Provincial Park");
  orc_to_name[189] = std::string("Loveland Bay Provincial Park");
  orc_to_name[190] = std::string("Morton Lake Provincial Park");
  orc_to_name[192] = std::string("Driftwood Canyon Provincial Park");
  orc_to_name[193] = std::string("Rathtrevor Beach Provincial Park");
  orc_to_name[195] = std::string("North Thompson River Provincial Park");
  orc_to_name[196] = std::string("Gibson Marine Provincial Park");
  orc_to_name[198] = std::string("Pirates Cove Marine Provincial Park");
  orc_to_name[199] = std::string("Cathedral Provincial Park");
  orc_to_name[200] = std::string("Sasquatch Provincial Park");
  orc_to_name[201] = std::string("Eneas Lakes Provincial Park");
  orc_to_name[202] = std::string("Syringa Provincial Park");
  orc_to_name[203] = std::string("Garden Bay Marine Provincial Park");
  orc_to_name[204] = std::string("Sun-Oka Beach Provincial Park");
  orc_to_name[206] = std::string("Bugaboo Provincial Park");
  orc_to_name[210] = std::string("Gordon Bay Provincial Park");
  orc_to_name[211] = std::string("Pinnacles Provincial Park");
  orc_to_name[212] = std::string("Silver Beach Provincial Park");
  orc_to_name[213] = std::string("Big Bar Lake Provincial Park");
  orc_to_name[214] = std::string("Buckinghorse River Wayside Provincial Park");
  orc_to_name[215] = std::string("Thurston Bay Marine Provincial Park");
  orc_to_name[216] = std::string("Drewry Point Provincial Park");
  orc_to_name[217] = std::string("Downing Provincial Park");
  orc_to_name[218] = std::string("Kickininee Provincial Park");
  orc_to_name[220] = std::string("Horne Lake Caves Provincial Park");
  orc_to_name[221] = std::string("Porpoise Bay Provincial Park");
  orc_to_name[222] = std::string("Gwillim Lake Provincial Park");
  orc_to_name[223] = std::string("Echo Bay Marine Provincial Park");
  orc_to_name[224] = std::string("White Pelican Provincial Park");
  orc_to_name[225] = std::string("Christina Lake Provincial Park");
  orc_to_name[226] = std::string("Drumbeg Provincial Park");
  orc_to_name[227] = std::string("Smuggler Cove Marine Provincial Park");
  orc_to_name[228] = std::string("Copeland Islands Marine Provincial Park");
  orc_to_name[229] = std::string("Purden Lake Provincial Park");
  orc_to_name[230] = std::string("Stuart Lake Provincial Park");
  orc_to_name[231] = std::string("Morden Colliery Historic Provincial Park");
  orc_to_name[232] = std::string("Nancy Greene Provincial Park");
  orc_to_name[233] = std::string("Seton Portage Historic Provincial Park");
  orc_to_name[234] = std::string("Paarens Beach Provincial Park");
  orc_to_name[235] = std::string("Kikomun Creek Provincial Park");
  orc_to_name[236] = std::string("Kettle River Recreation Area");
  orc_to_name[237] = std::string("Discovery Island Marine Provincial Park");
  orc_to_name[238] = std::string("Mount Edziza Provincial Park");
  orc_to_name[240] = std::string("Sooke Potholes Provincial Park");
  orc_to_name[241] = std::string("Mabel Lake Provincial Park");
  orc_to_name[242] = std::string("Brandywine Falls Provincial Park");
  orc_to_name[243] = std::string("Smelt Bay Provincial Park");
  orc_to_name[244] = std::string("Conkle Lake Provincial Park");
  orc_to_name[245] = std::string("Kilby Provincial Park");
  orc_to_name[246] = std::string("Atlin/T&eacute;ix&rsquo;gi Aan Tlein Provincial Park");
  orc_to_name[247] = std::string("Top of the World Provincial Park");
  orc_to_name[250] = std::string("Cape Scott Provincial Park");
  orc_to_name[251] = std::string("Carp Lake Provincial Park");
  orc_to_name[252] = std::string("Desolation Sound Marine Provincial Park");
  orc_to_name[253] = std::string("Elk Lakes Provincial Park");
  orc_to_name[254] = std::string("Kwadacha Wilderness Provincial Park");
  orc_to_name[255] = std::string("Naikoon Provincial Park");
  orc_to_name[256] = std::string("St Mary&#8217;s Alpine Provincial Park");
  orc_to_name[257] = std::string("Tatlatui Provincial Park");
  orc_to_name[258] = std::string("Chilliwack Lake Provincial Park");
  orc_to_name[259] = std::string("Okanagan Mountain Provincial Park");
  orc_to_name[261] = std::string("Skagit Valley Provincial Park");
  orc_to_name[262] = std::string("French Beach Provincial Park");
  orc_to_name[263] = std::string("Ross Lake Provincial Park");
  orc_to_name[264] = std::string("Mansons Landing Provincial Park");
  orc_to_name[265] = std::string("Octopus Islands Provincial Park");
  orc_to_name[266] = std::string("Tarahne Provincial Park");
  orc_to_name[267] = std::string("Ruckle Provincial Park");
  orc_to_name[268] = std::string("Horsefly Lake Provincial Park");
  orc_to_name[269] = std::string("Fossli Provincial Park");
  orc_to_name[272] = std::string("Pennask Lake Provincial Park");
  orc_to_name[273] = std::string("Green Lake Provincial Park");
  orc_to_name[275] = std::string("Niskonlith Lake Provincial Park");
  orc_to_name[276] = std::string("Herald Provincial Park");
  orc_to_name[277] = std::string("Kalamalka Lake Provincial Park");
  orc_to_name[278] = std::string("Cypress Provincial Park");
  orc_to_name[279] = std::string("Spatsizi Plateau Wilderness Provincial Park");
  orc_to_name[280] = std::string("Prophet River Wayside Provincial Park");
  orc_to_name[281] = std::string("Roderick Haig-Brown Provincial Park");
  orc_to_name[282] = std::string("Wardner Provincial Park");
  orc_to_name[283] = std::string("Schoen Lake Provincial Park");
  orc_to_name[286] = std::string("Taylor Landing Provincial Park");
  orc_to_name[287] = std::string("Whiteswan Lake Provincial Park");
  orc_to_name[288] = std::string("Red Bluff Provincial Park");
  orc_to_name[289] = std::string("One Island Lake Provincial Park");
  orc_to_name[290] = std::string("F.H. Barber Provincial Park");
  orc_to_name[292] = std::string("Tribune Bay Provincial Park");
  orc_to_name[293] = std::string("James Chabot Provincial Park");
  orc_to_name[294] = std::string("Okeover Arm Provincial Park");
  orc_to_name[295] = std::string("West Shawnigan Lake Provincial Park");
  orc_to_name[296] = std::string("Taylor Arm Provincial Park");
  orc_to_name[299] = std::string("Diana Lake Provincial Park");
  orc_to_name[300] = std::string("Shuswap Lake Marine Provincial Park");
  orc_to_name[301] = std::string("Roberts Memorial Provincial Park");
  orc_to_name[302] = std::string("Puntchesakut Lake Provincial Park");
  orc_to_name[303] = std::string("Sechelt Inlets Marine Provincial Park");
  orc_to_name[305] = std::string("West Lake Provincial Park");
  orc_to_name[306] = std::string("Kentucky-Alleyne Provincial Park");
  orc_to_name[307] = std::string("Bear Creek Provincial Park");
  orc_to_name[308] = std::string("Arrow Lakes Provincial Park");
  orc_to_name[310] = std::string("Hemer Provincial Park");
  orc_to_name[311] = std::string("Grohman Narrows Provincial Park");
  orc_to_name[313] = std::string("Spider Lake Provincial Park");
  orc_to_name[314] = std::string("Porteau Cove Provincial Park");
  orc_to_name[315] = std::string("Monkman Provincial Park");
  orc_to_name[316] = std::string("Sukunka Falls Provincial Park");
  orc_to_name[317] = std::string("Tudyah Lake Provincial Park");
  orc_to_name[318] = std::string("Dahl Lake Provincial Park");
  orc_to_name[319] = std::string("Jewel Lake Provincial Park");
  orc_to_name[320] = std::string("Wistaria Provincial Park");
  orc_to_name[321] = std::string("Pure Lake Provincial Park");
  orc_to_name[322] = std::string("Whaleboat Island Marine Provincial Park");
  orc_to_name[323] = std::string("Blanket Creek Provincial Park");
  orc_to_name[324] = std::string("McDonald Creek Provincial Park");
  orc_to_name[325] = std::string("Mount Terry Fox Provincial Park");
  orc_to_name[326] = std::string("East Pine Provincial Park");
  orc_to_name[327] = std::string("Valhalla Provincial Park");
  orc_to_name[328] = std::string("Maxhamish Lake Provincial Park");
  orc_to_name[329] = std::string("Babine Mountains Provincial Park");
  orc_to_name[330] = std::string("Alexandra Bridge Provincial Park");
  orc_to_name[331] = std::string("Shannon Falls Provincial Park");
  orc_to_name[333] = std::string("Simson Provincial Park");
  orc_to_name[334] = std::string("Coldwater River Provincial Park");
  orc_to_name[335] = std::string("Coquihalla Canyon Provincial Park");
  orc_to_name[336] = std::string("Coquihalla River Provincial Park");
  orc_to_name[338] = std::string("Akamina-Kishinena Provincial Park");
  orc_to_name[339] = std::string("Brooks Peninsula Park (a.k.a. M<sup>q</sup>uq&#x1D42;in Park)");
  orc_to_name[340] = std::string("Gitnadoiks River Provincial Park");
  orc_to_name[341] = std::string("Northern Rocky Mountains Provincial Park");
  orc_to_name[343] = std::string("Fiordland Conservancy");
  orc_to_name[344] = std::string("Hakai Conservation Study Area");
  orc_to_name[345] = std::string("Kakwa Provincial Park and Protected Area");
  orc_to_name[347] = std::string("Stikine River Provincial Park");
  orc_to_name[348] = std::string("Strathcona-Westmin Provincial Park");
  orc_to_name[351] = std::string("Coquihalla Summit Recreation Area");
  orc_to_name[353] = std::string("Walloper Lake Provincial Park");
  orc_to_name[355] = std::string("Eskers Provincial Park");
  orc_to_name[356] = std::string("Kinaskan Lake Provincial Park");
  orc_to_name[357] = std::string("Kootenay Lake Provincial Park");
  orc_to_name[358] = std::string("Meziadin Lake Provincial Park");
  orc_to_name[361] = std::string("Adams Lake Provincial Park - Bush Creek Site");
  orc_to_name[362] = std::string("Columbia Lake Provincial Park");
  orc_to_name[363] = std::string("Joffre Lakes Provincial Park");
  orc_to_name[365] = std::string("Halkett Bay Provincial Park");
  orc_to_name[366] = std::string("Sandwell Provincial Park");
  orc_to_name[367] = std::string("Squitty Bay Provincial Park");
  orc_to_name[369] = std::string("Juniper Beach Provincial Park");
  orc_to_name[370] = std::string("Sowchea Bay Provincial Park");
  orc_to_name[371] = std::string("Boyle Point Provincial Park");
  orc_to_name[372] = std::string("Buccaneer Bay Provincial Park");
  orc_to_name[373] = std::string("Roscoe Bay Provincial Park");
  orc_to_name[374] = std::string("Rugged Point Marine Provincial Park");
  orc_to_name[375] = std::string("Teakerne Arm Provincial Park");
  orc_to_name[376] = std::string("Walsh Cove Provincial Park");
  orc_to_name[377] = std::string("Raft Cove Provincial Park");
  orc_to_name[378] = std::string("Kekuli Bay Provincial Park");
  orc_to_name[379] = std::string("Sargeant Bay Provincial Park");
  orc_to_name[380] = std::string("Little Andrews Bay Marine Provincial Park");
  orc_to_name[381] = std::string("Blackcomb Glacier Provincial Park");
  orc_to_name[382] = std::string("Wallace Island Marine Provincial Park");
  orc_to_name[383] = std::string("Carmanah Walbran Provincial Park");
  orc_to_name[384] = std::string("Dionisio Point Provincial Park");
  orc_to_name[385] = std::string("Rearguard Falls Provincial Park");
  orc_to_name[386] = std::string("Anhluut&#8217;ukwsim Laxmihl Angwinga&#8217;Asanskwhl Nisga&#8217;a Provincial Park (a.k.a. Nisga&#8217;a Memorial Lava Bed Park)");
  orc_to_name[388] = std::string("Hardy Island Marine Provincial Park");
  orc_to_name[389] = std::string("Penrose Island Marine Provincial Park");
  orc_to_name[390] = std::string("Cormorant Channel Marine Provincial Park");
  orc_to_name[391] = std::string("Broughton Archipelago Marine Provincial Park");
  orc_to_name[392] = std::string("Harmony Islands Marine Provincial Park");
  orc_to_name[393] = std::string("Jackson Narrows Marine Provincial Park");
  orc_to_name[394] = std::string("Green Inlet Marine Provincial Park");
  orc_to_name[395] = std::string("Oliver Cove Marine Provincial Park");
  orc_to_name[396] = std::string("Codville Lagoon Marine Provincial Park");
  orc_to_name[397] = std::string("Khutzeymateen Provincial Park (a.k.a. Khutzeymateen/K&#8217;tzim-a-deen Grizzly Sanctuary)");
  orc_to_name[398] = std::string("Bull Canyon Provincial Park");
  orc_to_name[400] = std::string("Babine Lake Marine Provincial Park: Smithers Landing Site");
  orc_to_name[401] = std::string("Kitson Island Marine Provincial Park");
  orc_to_name[402] = std::string("Duffey Lake Provincial Park");
  orc_to_name[403] = std::string("Klewnuggit Inlet Marine Provincial Park");
  orc_to_name[404] = std::string("Martha Creek Provincial Park");
  orc_to_name[405] = std::string("Lowe Inlet Marine Provincial Park");
  orc_to_name[406] = std::string("Takla Lake Marine Provincial Park");
  orc_to_name[407] = std::string("Union Passage Marine Provincial Park");
  orc_to_name[408] = std::string("Steelhead Provincial Park");
  orc_to_name[409] = std::string("Ts&#8217;il&#660;os Provincial Park");
  orc_to_name[410] = std::string("Tatshenshini-Alsek Provincial Park");
  orc_to_name[411] = std::string("H&#225;thayim Marine Provincial Park");
  orc_to_name[412] = std::string("J&amp;aacute;ji7em and Kw&amp;rsquo;ulh Marine Provincial Park (a.k.a. Sandy Island)");
  orc_to_name[414] = std::string("Brackendale Eagles Provincial Park");
  orc_to_name[415] = std::string("Close-To-The-Edge Provincial Park");
  orc_to_name[416] = std::string("Erg Mountain Provincial Park");
  orc_to_name[421] = std::string("Klin-se-za Provincial Park");
  orc_to_name[422] = std::string("Three Sisters Lakes Provincial Park");
  orc_to_name[425] = std::string("Small River Caves Provincial Park");
  orc_to_name[426] = std::string("Dune Za Keyih Park (a.k.a. Frog-Gataga Park)");
  orc_to_name[427] = std::string("Border Lake Provincial Park");
  orc_to_name[428] = std::string("Choquette Hot Springs Provincial Park");
  orc_to_name[429] = std::string("Chukachida Protected Area");
  orc_to_name[430] = std::string("Craig Headwaters Protected Area");
  orc_to_name[431] = std::string("Great Glacier Provincial Park");
  orc_to_name[432] = std::string("Iskut River Hot Springs Provincial Park");
  orc_to_name[433] = std::string("Ningunsaw Provincial Park");
  orc_to_name[434] = std::string("Todagin South Slope Provincial Park");
  orc_to_name[435] = std::string("Chase Provincial Park");
  orc_to_name[436] = std::string("Finlay-Russel Provincial Park");
  orc_to_name[437] = std::string("Heather-Dina Lakes Provincial Park");
  orc_to_name[438] = std::string("Tuya Mountains Provincial Park");
  orc_to_name[440] = std::string("Anarchist Protected Area");
  orc_to_name[442] = std::string("Graystokes Provincial Park");
  orc_to_name[445] = std::string("Mount Griffin Provincial Park");
  orc_to_name[446] = std::string("Myra-Bellevue Provincial Park");
  orc_to_name[447] = std::string("Pukeashun Provincial Park");
  orc_to_name[448] = std::string("Snowy Protected Area");
  orc_to_name[449] = std::string("Upper Seymour River Provincial Park");
  orc_to_name[450] = std::string("Lanz and Cox Islands Provincial Park");
  orc_to_name[453] = std::string("Kingfisher Creek Provincial Park");
  orc_to_name[456] = std::string("Browne Lake Provincial Park");
  orc_to_name[457] = std::string("English Lake Provincial Park");
  orc_to_name[462] = std::string("Skaha Bluffs Provincial Park");
  orc_to_name[463] = std::string("Skookumchuck Rapids Provincial Park");
  orc_to_name[464] = std::string("South Okanagan Grasslands Protected Area");
  orc_to_name[467] = std::string("Upper Violet Creek Provincial Park");
  orc_to_name[468] = std::string("Mara Meadows Provincial Park");
  orc_to_name[469] = std::string("Francis Point Provincial Park");
  orc_to_name[470] = std::string("Spatsizi Headwaters Provincial Park");
  orc_to_name[471] = std::string("Vaseux Protected Area");
  orc_to_name[472] = std::string("Lower Tsitika River Provincial Park");
  orc_to_name[474] = std::string("Pennask Creek Provincial Park");
  orc_to_name[475] = std::string("Francis Point Ecological Reserve");
  orc_to_name[477] = std::string("Det San Ecological Reserve");
  orc_to_name[481] = std::string("Brim River Hot Springs Protected Area");
  orc_to_name[482] = std::string("Coste Rocks Provincial Park");
  orc_to_name[483] = std::string("Dala-Kildala Rivers Estuaries Provincial Park");
  orc_to_name[484] = std::string("Eagle Bay Provincial Park");
  orc_to_name[485] = std::string("Foch-Gilttoyees Provincial Park");
  orc_to_name[486] = std::string("Lower Skeena River Provincial Park");
  orc_to_name[487] = std::string("Lundmark Bog Protected Area");
  orc_to_name[488] = std::string("Nalbeelah Creek Wetlands Provincial Park");
  orc_to_name[489] = std::string("Owyacumish River Provincial Park");
  orc_to_name[490] = std::string("Sleeping Beauty Mountain Provincial Park");
  orc_to_name[491] = std::string("Kitsumkalum Lake North Protected Area");
  orc_to_name[492] = std::string("Swan Creek Protected Area");
  orc_to_name[509] = std::string("Mount Robson Corridor Protected Area");
  orc_to_name[511] = std::string("Power River Watershed Protected Area");
  orc_to_name[517] = std::string("Thunderbird&rsquo;s Nest (T&rsquo;iitsk&rsquo;in Paawats) Protected Area");
  orc_to_name[518] = std::string("Nation Lakes Provincial Park");
  orc_to_name[520] = std::string("Allison Harbour Marine Provincial Park");
  orc_to_name[523] = std::string("Nakina &ndash; Inklin Rivers/Y&aacute;wu Yaa Conservancy");
  orc_to_name[524] = std::string("Becher&rsquo;s Prairie Provincial Park");
  orc_to_name[525] = std::string("Nakina &ndash; Inklin Rivers (Kuthai Area)/Y&aacute;wu Yaa Conservancy");
  orc_to_name[527] = std::string("Boothman&#8217;s Oxbow Provincial Park");
  orc_to_name[528] = std::string("Gilpin Grasslands Provincial Park");
  orc_to_name[529] = std::string("Mount Erskine Provincial Park");
  orc_to_name[530] = std::string("Anderson Flats Provincial Park");
  orc_to_name[531] = std::string("Atna River Provincial Park");
  orc_to_name[532] = std::string("Burnie-Shea Provincial Park");
  orc_to_name[533] = std::string("Burnie River Protected Area");
  orc_to_name[534] = std::string("Nanika-Kidprice Lake Provincial Park");
  orc_to_name[535] = std::string("Morice Lake Provincial Park");
  orc_to_name[536] = std::string("Nadina Mountain Provincial Park");
  orc_to_name[537] = std::string("Old Man Lake Provincial Park");
  orc_to_name[538] = std::string("Sand Point Conservancy");
  orc_to_name[539] = std::string("Port Arthur Conservancy");
  orc_to_name[540] = std::string("Wilkinson-Wright Bay Conservancy");
  orc_to_name[541] = std::string("Long Island Conservancy");
  orc_to_name[542] = std::string("Sanctuary Bay Conservancy");
  orc_to_name[543] = std::string("Bear Island Conservancy");
  orc_to_name[544] = std::string("North Spit Conservancy");
  orc_to_name[545] = std::string("Callaghan Conservancy");
  orc_to_name[546] = std::string("Nlh&#225;xten/Cerise Creek Conservancy");
  orc_to_name[547] = std::string("Hakai L&#250;xvb&#225;l&#237;s Conservancy");
  orc_to_name[549] = std::string("Namu Corridor Conservancy");
  orc_to_name[551] = std::string("Eagle River Provincial Park");
  orc_to_name[552] = std::string("K&#8217;zuz&#225;lt/Twin Two Conservancy");
  orc_to_name[553] = std::string("Qwal&#237;mak/Upper Birkenhead Conservancy");
  orc_to_name[554] = std::string("Upper Elaho Valley Conservancy");
  orc_to_name[555] = std::string("Upper Rogers k&oacute;lii7 Conservancy");
  orc_to_name[556] = std::string("Est&#233;-tiwilh/Sigurd Creek Conservancy");
  orc_to_name[557] = std::string("Mkwal&rsquo;ts Conservancy");
  orc_to_name[558] = std::string("Upper Soo Conservancy");
  orc_to_name[559] = std::string("Duu Guusd Heritage Site/Conservancy");
  orc_to_name[560] = std::string("Yaaguun Suu Conservancy");
  orc_to_name[561] = std::string("I7loqaw7/100 Lakes Plateau Conservancy");
  orc_to_name[562] = std::string("Daawuuxusda Conservancy");
  orc_to_name[563] = std::string("Kamdis Consevancy");
  orc_to_name[564] = std::string("Kun&#60;u&#62;x&#60;/u&#62;alas Heritage Site/Conservancy");
  orc_to_name[566] = std::string("&#7733;&#8217;uuna Gwaay Heritage Site/Conservancy");
  orc_to_name[567] = std::string("Nang Xaldangaas Heritage Site/Conservancy");
  orc_to_name[568] = std::string("Indian Lake &ndash; Hitchcock Creek/&Aacute;t Ch'&icirc;ni Sh&agrave; Conservancy");
  orc_to_name[569] = std::string("Damaxyaa Heritage Site/Conservancy");
  orc_to_name[570] = std::string("SGaay Taw Siiwaay K&#8217;adjuu Heritage Site/Conservancy");
  orc_to_name[571] = std::string("Tlall Conservancy");
  orc_to_name[572] = std::string("Yaaguun Gandlaay Conservancy");
  orc_to_name[577] = std::string("Nisga&#8217;a Memorial Lava Bed Corridor Protected Area");
  orc_to_name[582] = std::string("Beaver Valley Provincial Park");
  orc_to_name[583] = std::string("Big Basin Provincial Park");
  orc_to_name[584] = std::string("Rainbow/Q&rsquo;iwentem Provincial Park");
  orc_to_name[585] = std::string("Dante&rsquo;s Inferno Provincial Park");
  orc_to_name[586] = std::string("Donnely Lake Provincial Park");
  orc_to_name[587] = std::string("Dragon Mountain Provincial Park");
  orc_to_name[588] = std::string("Eleven Sisters Provincial Park");
  orc_to_name[589] = std::string("Long Creek Provincial Park");
  orc_to_name[590] = std::string("Punti Island Provincial Park");
  orc_to_name[591] = std::string("Quesnel Lake Provincial Park");
  orc_to_name[592] = std::string("Titetown Provincial Park");
  orc_to_name[593] = std::string("Ne&rsquo;&#257;h&rsquo; Conservancy");
  orc_to_name[596] = std::string("Hanna-Tintina Conservancy");
  orc_to_name[597] = std::string("Small Inlet Protected Area");
  orc_to_name[598] = std::string("Otter Lake Protected Area");
  orc_to_name[616] = std::string("Stawamus Chief Protected Area");
  orc_to_name[623] = std::string("Bridge River Delta Provincial Park");
  orc_to_name[624] = std::string("Fred Antoine Provincial Park");
  orc_to_name[625] = std::string("French Bar Creek Provincial Park");
  orc_to_name[626] = std::string("Gwyneth Lake Provincial Park");
  orc_to_name[627] = std::string("Yalakom Provincial Park");
  orc_to_name[628] = std::string("Copper Johnny Provincial Park");
  orc_to_name[629] = std::string("Crater Lake Provincial Park");
  orc_to_name[631] = std::string("Elk Falls Protected Area");
  orc_to_name[632] = std::string("Boyle Point Protected Area");
  orc_to_name[633] = std::string("Denman Island Park");
  orc_to_name[634] = std::string("Denman Island Protected Area");
  orc_to_name[635] = std::string("Gerald Island Park");
  orc_to_name[1000] = std::string("Hunwadi/Ahnuhati-Bald Conservancy");
  orc_to_name[1001] = std::string("K&#8217;nabiyaaxl/Ashdown Conservancy");
  orc_to_name[1002] = std::string("Banks Nii &#321;uutiksm Conservancy");
  orc_to_name[1003] = std::string("Bishop Bay - Monkey Beach Conservancy");
  orc_to_name[1004] = std::string("Lax Kul Nii Luutiksm/Bonilla Conservancy");
  orc_to_name[1005] = std::string("Calvert Island Conservancy");
  orc_to_name[1006] = std::string("Lax ka&#8217;Gass/Campania Conservancy");
  orc_to_name[1007] = std::string("Crab Lake Conservancy");
  orc_to_name[1008] = std::string("Mahpahkum-Ahkwuna Deserters-Walker Conservancy");
  orc_to_name[1009] = std::string("Lax Kwil Dziidz/Fin Conservancy");
  orc_to_name[1010] = std::string("Moksgm&#8217;ol/Chapple-Cornwall Conservancy");
  orc_to_name[1011] = std::string("Gitxaala Nii Luutiksm Kitkatla Conservancy");
  orc_to_name[1012] = std::string("K&#8217;ootz/Khutze Conservancy");
  orc_to_name[1013] = std::string("Kitasoo Spirit Bear Conservancy");
  orc_to_name[1014] = std::string("K&#8217;lgaan/Klekane Conservancy");
  orc_to_name[1015] = std::string("Q&#8217;altanaas/Aaltanhas Conservancy");
  orc_to_name[1016] = std::string("Koeye Conservancy");
  orc_to_name[1017] = std::string("K&#8217;mooda/Lowe-Gamble Conservancy");
  orc_to_name[1018] = std::string("Monckton Nii Luutiksm Conservancy");
  orc_to_name[1019] = std::string("Pooley Island Conservancy");
  orc_to_name[1020] = std::string("Kt&#8217;ii/Racey Conservancy");
  orc_to_name[1021] = std::string("Tsa-latl/Smokehouse Conservancy");
  orc_to_name[1022] = std::string("Ksi xts&#8217;at&#8217;kw/Stagoo Conservancy");
  orc_to_name[1023] = std::string("Ugwiwey/Cape Caution-Blunden Bay Conservancy");
  orc_to_name[1024] = std::string("Qwiquallaaq/Boat Bay Conservancy");
  orc_to_name[1025] = std::string("Broughton Archipelago Conservancy");
  orc_to_name[1026] = std::string("Burdwood Group Conservancy");
  orc_to_name[1027] = std::string("Burnt Bridge Creek Conservancy");
  orc_to_name[1028] = std::string("Ugwiwey/Cape Caution Conservancy");
  orc_to_name[1029] = std::string("Carter Bay Conservancy");
  orc_to_name[1030] = std::string("Cascade-Sutslem Conservancy");
  orc_to_name[1031] = std::string("Catto Creek Conservancy");
  orc_to_name[1032] = std::string("Clayton Falls Conservancy");
  orc_to_name[1033] = std::string("Thorsen Creek Conservancy");
  orc_to_name[1034] = std::string("Clyak Estuary Conservancy");
  orc_to_name[1035] = std::string("Cranstown Point Conservancy");
  orc_to_name[1036] = std::string("Ellerslie-Roscoe Conservancy");
  orc_to_name[1037] = std::string("Emily Lake Conservancy");
  orc_to_name[1038] = std::string("Pa&#322;&#601;min/Estero Basin Conservancy");
  orc_to_name[1039] = std::string("Forward Harbour/&#411;&#601;x&#780;&#601;&#7490;&#601;y&#601;m Conservancy");
  orc_to_name[1040] = std::string("Jump Across Conservancy");
  orc_to_name[1041] = std::string("Qud&#477;s/Gillard-Jimmy Judd Island Conservancy");
  orc_to_name[1042] = std::string("Goat Cove Conservancy");
  orc_to_name[1043] = std::string("Goose Bay Conservancy");
  orc_to_name[1044] = std::string("Dean River Conservancy");
  orc_to_name[1045] = std::string("Kilbella Estuary Conservancy");
  orc_to_name[1046] = std::string("Kimsquit Estuary Conservancy");
  orc_to_name[1048] = std::string("Smithers Island Conservancy");
  orc_to_name[1049] = std::string("Dzawadi/Klinaklini Estuary Conservancy");
  orc_to_name[1050] = std::string("Kwatna Estuary Conservancy");
  orc_to_name[1051] = std::string("Lady Douglas-Don Penninsula Conservancy");
  orc_to_name[1052] = std::string("Machmell Conservancy");
  orc_to_name[1053] = std::string("Namu Conservancy");
  orc_to_name[1054] = std::string("Nooseneck Conservancy");
  orc_to_name[1055] = std::string("Outer Central Coast Islands Conservancy");
  orc_to_name[1056] = std::string("Lockhart-Gordon Conservancy");
  orc_to_name[1057] = std::string("Owikeno Conservancy");
  orc_to_name[1058] = std::string("Phillips Estuary/&#5448;Nacinux&#7490; Conservancy");
  orc_to_name[1059] = std::string("Polkinghorne Islands Conservancy");
  orc_to_name[1060] = std::string("Rescue Bay Conservancy");
  orc_to_name[1061] = std::string("Restoration Bay Conservancy");
  orc_to_name[1062] = std::string("&#7808;a&#7809;a&#411;/Seymour Estuary Conservancy");
  orc_to_name[1063] = std::string("Sheemahant Conservancy");
  orc_to_name[1064] = std::string("X&#7490;ak&#7490;&#601;&#5448;naxd&#601;&#5448;ma/Stafford Estuary Conservancy");
  orc_to_name[1065] = std::string("Cetan/Thurston Bay Conservancy");
  orc_to_name[1066] = std::string("Troup Passage Conservancy");
  orc_to_name[1067] = std::string("Upper Kimsquit River Conservancy");
  orc_to_name[1068] = std::string("Dzawadi/Upper Klinaklini River Conservancy");
  orc_to_name[1069] = std::string("Wahkash Point Conservancy");
  orc_to_name[1070] = std::string("Wakeman Estuary Conservancy");
  orc_to_name[1071] = std::string("Penrose-Ripon Conservancy");
  orc_to_name[1072] = std::string("H&#601;n&#654;&#601;md&#7611;i M&#601;kola/Yorke Island Conservancy");
  orc_to_name[1073] = std::string("Codville Lagoon Conservancy");
  orc_to_name[1074] = std::string("Hot Springs-No Name Creek Conservancy");
  orc_to_name[1075] = std::string("Neg&#780;i&#411;/Nekite Estuary Conservancy");
  orc_to_name[1077] = std::string("Upper Gladys River/Wats&iacute;x Deiyi Conservancy");
  orc_to_name[1078] = std::string("Khutzeymateen Inlet West Conservancy");
  orc_to_name[1079] = std::string("Mount Minto/K&rsquo;iy&aacute;n Conservancy");
  orc_to_name[1080] = std::string("Golden Gate/&lt;u&gt;X&lt;/u&gt;&aacute;at Y&aacute;di Aani Conservancy");
  orc_to_name[1081] = std::string("Monarch Mountain/A X&eacute;eg Deiyi Conservancy");
  orc_to_name[1083] = std::string("Willison Creek &ndash; Nelson Lake/S&iacute;t&rsquo; H&eacute;eni Conservancy");
  orc_to_name[1084] = std::string("Patterson Lake Provincial Park");
  orc_to_name[1086] = std::string("Upper Klinaklini Protected Area");
  orc_to_name[1087] = std::string("Redbrush Provincial Park");
  orc_to_name[1088] = std::string("Nisga&#8217;a Memorial Lava Bed Protected Area");
  orc_to_name[1089] = std::string("Ecstall-Spokskuut Conservancy");
  orc_to_name[1090] = std::string("Lax Kwaxl / Dundas-Melville Islands Conservancy");
  orc_to_name[1091] = std::string("Ethelda Bay - Tennant Island Conservany");
  orc_to_name[1092] = std::string("Europa Lake Conservancy");
  orc_to_name[1093] = std::string("Klewnuggit Conservancy");
  orc_to_name[1094] = std::string("Alty Conservancy");
  orc_to_name[1095] = std::string("Ecstall-Sparkling Conservancy");
  orc_to_name[1096] = std::string("Gunboat Harbour Conservancy");
  orc_to_name[1097] = std::string("Kennedy Island Conservancy");
  orc_to_name[1098] = std::string("Khtada Lake Conservancy");
  orc_to_name[1099] = std::string("Khyex Conservancy");
  orc_to_name[1100] = std::string("Khutzeymateen Inlet Conservancy");
  orc_to_name[1101] = std::string("Ksi X&#8217;anmaas Conservancy");
  orc_to_name[1102] = std::string("Kts&#8217;mkta&#8217;ani/Union Lake Conservancy");
  orc_to_name[1103] = std::string("Larcom Lagoon Conservancy");
  orc_to_name[1104] = std::string("Lucy Islands Conservancy");
  orc_to_name[1105] = std::string("Ktisgaidz/MacDonald Bay Conservancy");
  orc_to_name[1106] = std::string("Manzanita Cove Conservancy");
  orc_to_name[1108] = std::string("Dean River Corridor Conservancy");
  orc_to_name[1109] = std::string("Pa-aat Conservancy");
  orc_to_name[1110] = std::string("Shearwater Hot Springs Conservancy");
  orc_to_name[1111] = std::string("Simpson Lake East Conservancy");
  orc_to_name[1112] = std::string("Skeena Bank Conservancy");
  orc_to_name[1113] = std::string("Stair Creek Conservancy");
  orc_to_name[1114] = std::string("Ksgaxl/Stephens Island Conservancy");
  orc_to_name[1115] = std::string("Thulme Falls Conservancy");
  orc_to_name[1116] = std::string("Maxtaktsm&#8217;aa/Union Passage Conservancy");
  orc_to_name[1117] = std::string("Woodworth Lake Conservancy");
  orc_to_name[1118] = std::string("K&#8217;distsausk/Turtle Point Conservancy");
  orc_to_name[1119] = std::string("Wales Harbour Conservancy");
  orc_to_name[1120] = std::string("Winter Inlet Conservancy");
  orc_to_name[1121] = std::string("Zumtela Bay Conservancy");
  orc_to_name[1122] = std::string("Ecstall Headwaters Conservancy");
  orc_to_name[1123] = std::string("K&#8217;waal Conservancy");
  orc_to_name[1124] = std::string("Bella Coola Estuary Conservancy");
  orc_to_name[1125] = std::string("Bishop Bay - Monkey Beach Corridor Conservancy");
  orc_to_name[3001] = std::string("Cleland Island Ecological Reserve");
  orc_to_name[3002] = std::string("East Redonda Island Ecological Reserve");
  orc_to_name[3003] = std::string("Soap Lake Ecological Reserve");
  orc_to_name[3004] = std::string("Lasqueti Island Ecological Reserve");
  orc_to_name[3005] = std::string("Lily Pad Lake Ecological Reserve");
  orc_to_name[3006] = std::string("Buck Hills Road Ecological Reserve");
  orc_to_name[3007] = std::string("Trout Creek Ecological Reserve");
  orc_to_name[3008] = std::string("Clayhurst Ecological Reserve");
  orc_to_name[3009] = std::string("Tow Hill Ecological Reserve");
  orc_to_name[3010] = std::string("Rose Spit Ecological Reserve");
  orc_to_name[3011] = std::string("Sartine Islands Ecological Reserve");
  orc_to_name[3012] = std::string("Beresford Island Ecological Reserve");
  orc_to_name[3013] = std::string("Anne Vallee (Triangle Island) Ecological Reserve");
  orc_to_name[3014] = std::string("Solander Island Ecological Reserve");
  orc_to_name[3016] = std::string("Mount Tuam Ecological Reserve");
  orc_to_name[3017] = std::string("Canoe Islets Ecological Reserve");
  orc_to_name[3018] = std::string("Rose Islets Ecological Reserve");
  orc_to_name[3019] = std::string("Mount Sabine Ecological Reserve");
  orc_to_name[3020] = std::string("Columbia Lake Ecological Reserve");
  orc_to_name[3021] = std::string("Skagit River Forest Ecological Reserve");
  orc_to_name[3022] = std::string("Ross Lake Ecological Reserve");
  orc_to_name[3023] = std::string("Moore/McKenny/Whitmore Islands Ecological Reserve");
  orc_to_name[3024] = std::string("Baeria Rocks Ecological Reserve");
  orc_to_name[3025] = std::string("Dewdney and Glide Islands Ecological Reserve");
  orc_to_name[3026] = std::string("Ram Creek Ecological Reserve");
  orc_to_name[3027] = std::string("Whipsaw Creek Ecological Reserve");
  orc_to_name[3028] = std::string("Ambrose Lake Ecological Reserve");
  orc_to_name[3029] = std::string("Tranquille Ecological Reserve");
  orc_to_name[3030] = std::string("Vance Creek Ecological Reserve");
  orc_to_name[3031] = std::string("Lew Creek Ecological Reserve");
  orc_to_name[3032] = std::string("Evans Lake Ecological Reserve");
  orc_to_name[3033] = std::string("Field&#8217;s Lease Ecological Reserve");
  orc_to_name[3034] = std::string("Big White Mountain Ecological Reserve");
  orc_to_name[3035] = std::string("Westwick Lakes Ecological Reserve");
  orc_to_name[3036] = std::string("Mackinnon Esker Ecological Reserve");
  orc_to_name[3037] = std::string("Mount Maxwell Ecological Reserve");
  orc_to_name[3038] = std::string("Takla Lake Ecological Reserve");
  orc_to_name[3039] = std::string("Sunbeam Creek Ecological Reserve");
  orc_to_name[3040] = std::string("Kingcome River / Atlatzi River Ecological Reserve");
  orc_to_name[3041] = std::string("Tacheeda Lakes Ecological Reserve");
  orc_to_name[3042] = std::string("Mara Meadows Ecological Reserve");
  orc_to_name[3043] = std::string("Mount Griffin Ecological Reserve");
  orc_to_name[3045] = std::string("Vladimir J. Krajina (Port Chanal) Ecological Reserve");
  orc_to_name[3046] = std::string("Sikanni Chief River Ecological Reserve");
  orc_to_name[3047] = std::string("Parker Lake Ecological Reserve");
  orc_to_name[3048] = std::string("Bowen Island Ecological Reserve");
  orc_to_name[3049] = std::string("Kingfisher Creek Ecological Reserve");
  orc_to_name[3050] = std::string("Cecil Lake Ecological Reserve");
  orc_to_name[3051] = std::string("Browne Lake Ecological Reserve");
  orc_to_name[3052] = std::string("Drizzle Lake Ecological Reserve");
  orc_to_name[3053] = std::string("Narcosli Lake Ecological Reserve");
  orc_to_name[3054] = std::string("Nitinat Lake Ecological Reserve");
  orc_to_name[3055] = std::string("Cardiff Mountain Ecological Reserve");
  orc_to_name[3056] = std::string("Goosegrass Creek Ecological Reserve");
  orc_to_name[3057] = std::string("Chickens Neck Mountain Ecological Reserve");
  orc_to_name[3058] = std::string("Blue/Dease Rivers Ecological Reserve");
  orc_to_name[3059] = std::string("Ningunsaw River Ecological Reserve");
  orc_to_name[3060] = std::string("Drywilliam Lake Ecological Reserve");
  orc_to_name[3061] = std::string("Upper Shuswap River Ecological Reserve");
  orc_to_name[3062] = std::string("Fort Nelson River Ecological Reserve");
  orc_to_name[3063] = std::string("Skeena River Ecological Reserve");
  orc_to_name[3064] = std::string("Ilgachuz Range Ecological Reserve");
  orc_to_name[3065] = std::string("Chasm Ecological Reserve");
  orc_to_name[3066] = std::string("Ten Mile Point Ecological Reserve");
  orc_to_name[3067] = std::string("Satellite Channel Ecological Reserve");
  orc_to_name[3068] = std::string("Gladys Lake Ecological Reserve");
  orc_to_name[3069] = std::string("Baynes Island Ecological Reserve");
  orc_to_name[3070] = std::string("Mount Tinsdale Ecological Reserve");
  orc_to_name[3071] = std::string("Blackwater Creek Ecological Reserve");
  orc_to_name[3072] = std::string("Nechako River Ecological Reserve");
  orc_to_name[3073] = std::string("Torkelsen Lake Ecological Reserve");
  orc_to_name[3075] = std::string("Clanninick Creek Ecological Reserve");
  orc_to_name[3076] = std::string("Fraser River Ecological Reserve");
  orc_to_name[3077] = std::string("Campbell Brown (Kalamalka Lake) Ecological Reserve");
  orc_to_name[3078] = std::string("Meridian Road (Vanderhoof) Ecological Reserve");
  orc_to_name[3079] = std::string("Chilako River Ecological Reserve");
  orc_to_name[3080] = std::string("Smith River Ecological Reserve");
  orc_to_name[3081] = std::string("Morice River Ecological Reserve");
  orc_to_name[3082] = std::string("Cinema Bog Ecological Reserve");
  orc_to_name[3083] = std::string("San Juan Ridge Ecological Reserve");
  orc_to_name[3084] = std::string("Aleza Lake Ecological Reserve");
  orc_to_name[3085] = std::string("Patsuk Creek Ecological Reserve");
  orc_to_name[3086] = std::string("Bednesti Lake Ecological Reserve");
  orc_to_name[3087] = std::string("Heather Lake Ecological Reserve");
  orc_to_name[3088] = std::string("Skwaha Lake Ecological Reserve");
  orc_to_name[3089] = std::string("Skagit River Cottonwoods Ecological Reserve");
  orc_to_name[3090] = std::string("Sutton Pass Ecological Reserve");
  orc_to_name[3091] = std::string("Raspberry Harbour Ecological Reserve");
  orc_to_name[3092] = std::string("Skihist Ecological Reserve");
  orc_to_name[3093] = std::string("Lepas Bay Ecological Reserve");
  orc_to_name[3094] = std::string("Oak Bay Islands Ecological Reserve");
  orc_to_name[3097] = std::string("Race Rocks Ecological Reserve");
  orc_to_name[3098] = std::string("Chilliwack River Ecological Reserve");
  orc_to_name[3099] = std::string("Pitt Polder Ecological Reserve");
  orc_to_name[3100] = std::string("Hayne&#8217;s Lease Ecological Reserve");
  orc_to_name[3101] = std::string("Doc English Bluff Ecological Reserve");
  orc_to_name[3102] = std::string("Charlie Cole Creek Ecological Reserve");
  orc_to_name[3103] = std::string("Byers/Conroy/Harvey/Sinnett Islands Ecological Reserve");
  orc_to_name[3104] = std::string("Gilnockie Creek Ecological Reserve");
  orc_to_name[3105] = std::string("Megin River Ecological Reserve");
  orc_to_name[3106] = std::string("Skagit River Rhododendron Ecological Reserve");
  orc_to_name[3107] = std::string("Chunamon Creek Ecological Reserve");
  orc_to_name[3108] = std::string("Cougar Canyon Ecological Reserve");
  orc_to_name[3109] = std::string("Checleset Bay Ecological Reserve");
  orc_to_name[3110] = std::string("McQueen Creek Ecological Reserve");
  orc_to_name[3111] = std::string("Robson Bight (Michael Bigg) Ecologcial Reserve");
  orc_to_name[3112] = std::string("Mount Tzuhalem Ecological Reserve");
  orc_to_name[3113] = std::string("Honeymoon Bay Ecological Reserve");
  orc_to_name[3114] = std::string("Williams Creek Ecological Reserve");
  orc_to_name[3115] = std::string("Gingietl Creek Ecological Reserve");
  orc_to_name[3116] = std::string("Katherine Tye (Vedder Crossing) Ecological Reserve");
  orc_to_name[3117] = std::string("Haley Lake Ecological Reserve");
  orc_to_name[3118] = std::string("Nimpkish River Ecological Reserve");
  orc_to_name[3119] = std::string("Tahsish River Ecological Reserve");
  orc_to_name[3120] = std::string("Duke of Edinburgh (Pine/Storm/Tree Islands) Ecological Reserve");
  orc_to_name[3122] = std::string("Tsitika Mountain Ecological Reserve");
  orc_to_name[3123] = std::string("Mount Derby Ecological Reserve");
  orc_to_name[3124] = std::string("Tsitika River Ecological Reserve");
  orc_to_name[3125] = std::string("Mount Elliot Ecological Reserve");
  orc_to_name[3126] = std::string("Claud Elliot Creek Ecological Reserve");
  orc_to_name[3127] = std::string("Big Creek Ecological Reserve");
  orc_to_name[3128] = std::string("Galiano Island Ecological Reserve");
  orc_to_name[3129] = std::string("Klaskish River Ecological Reserve");
  orc_to_name[3130] = std::string("Mahoney Lake Ecological Reserve");
  orc_to_name[3131] = std::string("Stoyoma Creek Ecological Reserve");
  orc_to_name[3132] = std::string("Trial Islands Ecological Reserve");
  orc_to_name[3133] = std::string("Gamble Creek Ecological Reserve");
  orc_to_name[3134] = std::string("Ellis Island Ecological Reserve");
  orc_to_name[3875] = std::string("Finn Creek Protected Area");
  orc_to_name[3931] = std::string("Ancient Forest/Chun T'oh Whudujut Provincial Park");
  orc_to_name[3932] = std::string("Nisga'a Memorial Lava Bed Corridor Protected Area (No. 2)");
  orc_to_name[4041] = std::string("Goguka Creek Protected Area");
  orc_to_name[4104] = std::string("Denison-Bonneau Provincial Park");
  orc_to_name[4214] = std::string("Slim Creek Provincial Park");
  orc_to_name[4232] = std::string("Portage Brule Rapids Ecological Reserve");
  orc_to_name[4276] = std::string("Fraser River Breaks Provincial Park");
  orc_to_name[4337] = std::string("Bowser Ecological Reserve");
  orc_to_name[4351] = std::string("Pine River Breaks Provincial Park");
  orc_to_name[4361] = std::string("San Juan River Estuary Ecological Reserve");
  orc_to_name[4382] = std::string("Kitimat River Provincial Park");
  orc_to_name[4433] = std::string("Lac du Bois Grasslands Protected Area");
  orc_to_name[4448] = std::string("Catherine Creek Ecological Reserve");
  orc_to_name[4455] = std::string("Woodley Range Ecological Reserve");
  orc_to_name[4460] = std::string("Klanawa River Ecological Reserve");
  orc_to_name[4471] = std::string("Yellowpoint Bog Ecological Reserve");
  orc_to_name[4981] = std::string("Bearhole Lake Protected Area");
  orc_to_name[4982] = std::string("Cathedral Protected Area");
  orc_to_name[4983] = std::string("Close-To-The-Edge Protected Area");
  orc_to_name[4984] = std::string("Cummins River Protected Area");
  orc_to_name[4985] = std::string("Denetiah Corridor Protected Area");
  orc_to_name[5015] = std::string("Dune Za Keyih Protected Area [a.k.a. Frog-Gataga Protected Area]");
  orc_to_name[5018] = std::string("Finlay Russel Protected Area");
  orc_to_name[5019] = std::string("Fintry Protected Area");
  orc_to_name[5020] = std::string("Francois Lake Protected Area");
  orc_to_name[5022] = std::string("Liard River Corridor Protected Area");
  orc_to_name[5023] = std::string("Maxhamish Lake Protected Area");
  orc_to_name[5024] = std::string("Myra-Bellevue Protected Area");
  orc_to_name[5025] = std::string("Nahatlatch Protected Area");
  orc_to_name[5026] = std::string("Northern Rocky Mountains Protected Area");
  orc_to_name[5027] = std::string("Omineca Protected Area");
  orc_to_name[5028] = std::string("Portage Brule Rapids Protected Area");
  orc_to_name[5029] = std::string("Ptarmigan Creek Protected Area");
  orc_to_name[5030] = std::string("Seven Sisters Protected Area");
  orc_to_name[5031] = std::string("Sugarbowl-Grizzly Den Protected Area");
  orc_to_name[5032] = std::string("Sustut Protected Area");
  orc_to_name[5033] = std::string("Sutherland River Protected Area");
  orc_to_name[5034] = std::string("Liard River West Corridor Provincial Park");
  orc_to_name[5035] = std::string("Thinahtea South Protected Area");
  orc_to_name[5036] = std::string("Tweedsmuir Corridor Protected Area");
  orc_to_name[5037] = std::string("West Twin Protected Area");
  orc_to_name[5039] = std::string("Exchamsiks River Protected Area");
  orc_to_name[5040] = std::string("Foch-Gilttoyees Protected Area");
  orc_to_name[5041] = std::string("Gitnadoiks River Protected Area");
  orc_to_name[5042] = std::string("Maquinna Protected Area");
  orc_to_name[5043] = std::string("Mount Robson Protected Area");
  orc_to_name[5044] = std::string("Kakwa Protected Area");
  orc_to_name[6081] = std::string("Comox Lake Bluffs Ecological Reserve");
  orc_to_name[6093] = std::string("Main Lake Provincial Park");
  orc_to_name[6111] = std::string("Santa Getrudis-Boca del Infierno Provincial Park");
  orc_to_name[6161] = std::string("Cowichan River Provincial Park");
  orc_to_name[6197] = std::string("Inland Lake Provincial Park");
  orc_to_name[6268] = std::string("Malaspina Provincial Park");
  orc_to_name[6301] = std::string("Anderson Bay Provincial Park");
  orc_to_name[6328] = std::string("Stawamus Chief Provincial Park");
  orc_to_name[6547] = std::string("Trepanier Provincial Park");
  orc_to_name[6610] = std::string("Wap Creek Provincial Park");
  orc_to_name[6624] = std::string("White Lake Grasslands Protected Area");
  orc_to_name[6648] = std::string("Adams Lake Marine Provincial Park: Spillman Beach Site");
  orc_to_name[6818] = std::string("Finn Creek Provincial Park");
  orc_to_name[6860] = std::string("Chu Chua Cottonwood Provincial Park");
  orc_to_name[6865] = std::string("McConnell Lake Provincial Park");
  orc_to_name[6878] = std::string("Tunkwa Provincial Park");
  orc_to_name[6892] = std::string("Roche Lake Provincial Park");
  orc_to_name[6900] = std::string("Blue Earth Lake Provincial Park");
  orc_to_name[6987] = std::string("Epsom Provincial Park");
  orc_to_name[6998] = std::string("Nahatlatch Provincial Park");
  orc_to_name[7211] = std::string("Windermere Lake Provincial Park");
  orc_to_name[7458] = std::string("Kluskoil Lake Provincial Park");
  orc_to_name[7668] = std::string("Nuntsi Provincial Park");
  orc_to_name[8053] = std::string("Fort George Canyon Provincial Park");
  orc_to_name[8094] = std::string("Beatton River Provincial Park");
  orc_to_name[8097] = std::string("Kiskatinaw River Provincial Park");
  orc_to_name[8109] = std::string("Wapiti Lake Provincial Park");
  orc_to_name[8123] = std::string("Rolla Canyon Ecological Reserve");
  orc_to_name[8277] = std::string("Smith River Falls - Fort Halkett Provincial Park");
  orc_to_name[8284] = std::string("Toad River Hot Springs Provincial Park");
  orc_to_name[8288] = std::string("Prophet River Hotsprings Provincial Park");
  orc_to_name[8291] = std::string("Ospika Cones Ecological Reserve");
  orc_to_name[8297] = std::string("Denetiah Provincial Park");
  orc_to_name[8299] = std::string("Redfern-Keily Provincial Park");
  orc_to_name[8306] = std::string("Sikanni Chief Falls Protected Area");
  orc_to_name[8312] = std::string("Grayling River Hot Springs Ecological Reserve");
  orc_to_name[8325] = std::string("Kotcho Lake Ecological Reserve");
  orc_to_name[8330] = std::string("Klua Lakes Protected Area");
  orc_to_name[8345] = std::string("Jesse Falls Protected Area");
  orc_to_name[8350] = std::string("Hai Lake - Mount Herman Provincial Park");
  orc_to_name[8379] = std::string("Weewanie Hot Springs Provincial Park");
  orc_to_name[8509] = std::string("Nilkitkwa Lake Provincial Park");
  orc_to_name[8555] = std::string("Burns Lake Provincial Park");
  orc_to_name[8642] = std::string("Lava Forks Provincial Park");
  orc_to_name[8645] = std::string("Pitman River Protected Area");
  orc_to_name[8697] = std::string("Enderby Cliffs Provincial Park");
  orc_to_name[8741] = std::string("Seven Sisters Provincial Park");
  orc_to_name[8774] = std::string("God&#8217;s Pocket Marine Provincial Park");
  orc_to_name[8778] = std::string("Catala Island Marine Provincial Park");
  orc_to_name[8779] = std::string("Big Bunsby Marine Provincial Park");
  orc_to_name[8782] = std::string("White Ridge Provincial Park");
  orc_to_name[8796] = std::string("Stuart River Provincial Park");
  orc_to_name[8814] = std::string("Bear Glacier Provincial Park");
  orc_to_name[8966] = std::string("Lakelse Lake Wetlands Provincial Park");
  orc_to_name[8969] = std::string("Liard River Corridor Provincial Park");
  orc_to_name[9034] = std::string("Ptarmigan Creek Provincial Park");
  orc_to_name[9066] = std::string("South Chilcotin Mountains Provincial Park");
  orc_to_name[9077] = std::string("Swan Lake Kispiox River Provincial Park");
  orc_to_name[9118] = std::string("Trembleur Lake Provincial Park");
  orc_to_name[9123] = std::string("Tutshi Lake/T&rsquo;ooch&rsquo; &Aacute;ayi Conservancy");
  orc_to_name[9147] = std::string("Dixie Cove Marine Provincial Park");
  orc_to_name[9185] = std::string("Height of the Rockies Provincial Park");
  orc_to_name[9209] = std::string("Bligh Island Marine Provincial Park");
  orc_to_name[9213] = std::string("Fintry Provincial Park");
  orc_to_name[9218] = std::string("Pritchard Park");
  orc_to_name[9229] = std::string("Gowlland Tod Provincial Park");
  orc_to_name[9335] = std::string("Shuswap River Islands Provincial Park");
  orc_to_name[9383] = std::string("Pillar Provincial Park");
  orc_to_name[9398] = std::string("Juan De Fuca Provincial Park");
  orc_to_name[9435] = std::string("Purcell Wilderness Conservancy Provincial Park");
  orc_to_name[9451] = std::string("Callaghan Lake Provincial Park");
  orc_to_name[9453] = std::string("Arctic Pacific Lakes Provincial Park");
  orc_to_name[9456] = std::string("Itcha Ilgachuz Provincial Park");
  orc_to_name[9457] = std::string("Huchsduwachsdu Nuyem Jees / Kitlope Heritage Conservancy");
  orc_to_name[9458] = std::string("Stein Valley Nlaka&#8217;pamux Heritage Provincial Park");
  orc_to_name[9459] = std::string("Tahsish-Kwois Provincial Park");
  orc_to_name[9460] = std::string("Tetrahedron Provincial Park");
  orc_to_name[9461] = std::string("West Twin Provincial Park");
  orc_to_name[9464] = std::string("Quatsino Provincial Park");
  orc_to_name[9465] = std::string("Marble River Provincial Park");
  orc_to_name[9466] = std::string("Lower Nimpkish Provincial Park");
  orc_to_name[9469] = std::string("Claud Elliot Lake Provincial Park");
  orc_to_name[9471] = std::string("Woss Lake Provincial Park");
  orc_to_name[9474] = std::string("Hitchie Creek Provincial Park");
  orc_to_name[9476] = std::string("Rock Bay Marine Provincial Park");
  orc_to_name[9480] = std::string("Moose Valley Provincial Park");
  orc_to_name[9481] = std::string("Homathko River-Tatlayoko Protected Area");
  orc_to_name[9482] = std::string("Junction Sheep Range Provincial Park");
  orc_to_name[9483] = std::string("Schoolhouse Lake Provincial Park");
  orc_to_name[9485] = std::string("Marble Range Provincial Park");
  orc_to_name[9489] = std::string("Nazko Lake Provincial Park");
  orc_to_name[9493] = std::string("Hesquiat Lake Provincial Park");
  orc_to_name[9494] = std::string("Hesquiat Peninsula Provincial Park");
  orc_to_name[9495] = std::string("Sydney Inlet Provincial Park");
  orc_to_name[9497] = std::string("Flores Island Provincial Park");
  orc_to_name[9498] = std::string("Vargas Island Provincial Park");
  orc_to_name[9499] = std::string("Epper Passage Provincial Park");
  orc_to_name[9500] = std::string("Dawley Passage Provincial Park");
  orc_to_name[9501] = std::string("Tranquil Creek Provincial Park");
  orc_to_name[9502] = std::string("Clayoquot Arm Provincial Park");
  orc_to_name[9503] = std::string("Kennedy River Bog Provincial Park");
  orc_to_name[9504] = std::string("Kennedy Lake Provincial Park");
  orc_to_name[9506] = std::string("Sabine Channel Marine Provincial Park");
  orc_to_name[9507] = std::string("Clayoquot Plateau Provincial Park");
  orc_to_name[9508] = std::string("Pinecone Burke Provincial Park");
  orc_to_name[9509] = std::string("Indian Arm Provincial Park");
  orc_to_name[9510] = std::string("Pine Le Moray Provincial Park");
  orc_to_name[9512] = std::string("Jedediah Island Marine Provincial Park");
  orc_to_name[9518] = std::string("Greenbush Lake Protected Area");
  orc_to_name[9522] = std::string("Taku River/T&amp;rsquo;ak&amp;uacute; T&amp;eacute;ix&amp;rsquo; Conservancy");
  orc_to_name[9532] = std::string("Nimpkish Lake Provincial Park");
  orc_to_name[9540] = std::string("Sulphur Passage Provincial Park");
  orc_to_name[9544] = std::string("Spipiyus Provincial Park");
  orc_to_name[9548] = std::string("Granby Provincial Park");
  orc_to_name[9549] = std::string("Gladstone Provincial Park");
  orc_to_name[9550] = std::string("Lockhart Creek Provincial Park");
  orc_to_name[9551] = std::string("Kianuko Provincial Park");
  orc_to_name[9552] = std::string("West Arm Provincial Park");
  orc_to_name[9553] = std::string("Goat Range Provincial Park");
  orc_to_name[9554] = std::string("Bodega Ridge Provincial Park");
  orc_to_name[9556] = std::string("Upper Lillooet Provincial Park");
  orc_to_name[9557] = std::string("Edge Hills Provincial Park");
  orc_to_name[9563] = std::string("Big Creek Provincial Park");
  orc_to_name[9565] = std::string("Bishop River Provincial Park");
  orc_to_name[9567] = std::string("Bonaparte Provincial Park");
  orc_to_name[9571] = std::string("Bulkley Junction Provincial Park");
  orc_to_name[9582] = std::string("Anstey Hunakwa Provincial Park");
  orc_to_name[9584] = std::string("Babine River Corridor Provincial Park");
  orc_to_name[9587] = std::string("Brent Mountain Protected Area");
  orc_to_name[9590] = std::string("Churn Creek Protected Area");
  orc_to_name[9596] = std::string("Dunn Peak Provincial Park");
  orc_to_name[9597] = std::string("Ed Bird-Estella Lakes Provincial Park");
  orc_to_name[9601] = std::string("Sue Channel Provincial Park");
  orc_to_name[9604] = std::string("Damdochax Protected Area");
  orc_to_name[9622] = std::string("Cariboo Mountains Provincial Park");
  orc_to_name[9633] = std::string("Peace River Corridor Provincial Park");
  orc_to_name[9658] = std::string("Stuart Lake Marine Provincial Park");
  orc_to_name[9679] = std::string("Cariboo River Provincial Park");
  orc_to_name[9680] = std::string("Gilnockie Provincial Park");
  orc_to_name[9681] = std::string("Cummins Lakes Provincial Park");
  orc_to_name[9682] = std::string("Flat Lake Provincial Park");
  orc_to_name[9687] = std::string("High Lakes Basin Provincial Park");
  orc_to_name[9688] = std::string("Cornwall Hills Provincial Park");
  orc_to_name[9689] = std::string("Arrowstone Provincial Park");
  orc_to_name[9690] = std::string("Elephant Hill Provincial Park");
  orc_to_name[9691] = std::string("Emar Lakes Provincial Park");
  orc_to_name[9693] = std::string("Momich Lakes Provincial Park");
  orc_to_name[9694] = std::string("Oregon Jack Provincial Park");
  orc_to_name[9695] = std::string("Porcupine Meadows Provincial Park");
  orc_to_name[9696] = std::string("Taweel Provincial Park");
  orc_to_name[9698] = std::string("Upper Adams River Provincial Park");
  orc_to_name[9711] = std::string("Wrinkly Face Provincial Park");
  orc_to_name[9712] = std::string("Bedard Aspen Provincial Park");
  orc_to_name[9713] = std::string("Blue River Black Spruce Provincial Park");
  orc_to_name[9714] = std::string("Blue River Pine Provincial Park");
  orc_to_name[9715] = std::string("Buse Lake Protected Area");
  orc_to_name[9716] = std::string("Caligata Lake Provincial Park");
  orc_to_name[9717] = std::string("Castle Rock Hoodoos Provincial Park");
  orc_to_name[9719] = std::string("Eakin Creek Canyon Provincial Park");
  orc_to_name[9720] = std::string("Eakin Creek Floodplain Provincial Park");
  orc_to_name[9721] = std::string("Greenstone Mountain Provincial Park");
  orc_to_name[9722] = std::string("Harbour-Dudgeon Lakes Provincial Park");
  orc_to_name[9723] = std::string("Harry Lake Aspen Provincial Park");
  orc_to_name[9726] = std::string("Monte Creek Provincial Park");
  orc_to_name[9727] = std::string("Mount Savona Provincial Park");
  orc_to_name[9728] = std::string("Mud Lake Delta Provincial Park");
  orc_to_name[9729] = std::string("North Thompson Islands Provincial Park");
  orc_to_name[9730] = std::string("North Thompson Oxbows Jensen Island Provincial Park");
  orc_to_name[9731] = std::string("North Thompson Oxbows East Provincial Park");
  orc_to_name[9732] = std::string("North Thompson Oxbows Manteau Provincial Park");
  orc_to_name[9733] = std::string("Oregana Creek Provincial Park");
  orc_to_name[9734] = std::string("Painted Bluffs Provincial Park");
  orc_to_name[9735] = std::string("Pyramid Creek Falls Provincial Park");
  orc_to_name[9736] = std::string("Six Mile Hill Protected Area");
  orc_to_name[9738] = std::string("Tsintsunko Lake Provincial Park");
  orc_to_name[9739] = std::string("Walhachin Oxbows Provincial Park");
  orc_to_name[9740] = std::string("Wire Cache Provincial Park");
  orc_to_name[9743] = std::string("Hudson Rocks Ecological Reserve");
  orc_to_name[9744] = std::string("Misty Lake Ecological Reserve");
  orc_to_name[9745] = std::string("Artlish Caves Provincial Park");
  orc_to_name[9746] = std::string("Gold Muchalat Provincial Park");
  orc_to_name[9747] = std::string("Lawn Point Provincial Park");
  orc_to_name[9748] = std::string("Nitinat River Provincial Park");
  orc_to_name[9749] = std::string("Nuchatlitz Provincial Park");
  orc_to_name[9750] = std::string("Read Island Provincial Park");
  orc_to_name[9751] = std::string("Surge Narrows Provincial Park");
  orc_to_name[9752] = std::string("Weymer Creek Provincial Park");
  orc_to_name[9753] = std::string("White River Provincial Park");
  orc_to_name[9754] = std::string("Small Inlet Marine Provincial Park");
  orc_to_name[9755] = std::string("Banana Island Provincial Park");
  orc_to_name[9761] = std::string("Mount Elphinstone Provincial Park");
  orc_to_name[9762] = std::string("Duck Lake Protected Area");
  orc_to_name[9763] = std::string("South Texada Island Provincial Park");
  orc_to_name[9764] = std::string("Tantalus Provincial Park");
  orc_to_name[9765] = std::string("Mount Richardson Provincial Park");
  orc_to_name[9767] = std::string("Rendezvous Island South Provincial Park");
  orc_to_name[9768] = std::string("Clendinning Provincial Park");
  orc_to_name[9769] = std::string("Yale Garry Oak Ecological Reserve");
  orc_to_name[9773] = std::string("Purcell Wilderness Conservancy Corridor Protected Area");
  orc_to_name[9777] = std::string("Sutherland River Provincial Park");
  orc_to_name[9778] = std::string("Francois Lake Provincial Park");
  orc_to_name[9779] = std::string("Nechako Canyon Protected Area");
  orc_to_name[9780] = std::string("Finger-Tatuk Provincial Park");
  orc_to_name[9781] = std::string("Entiako Provincial Park");
  orc_to_name[9782] = std::string("Kitwanga Mountain Provincial Park");
  orc_to_name[9783] = std::string("Bearhole Lake Provincial Park");
  orc_to_name[9785] = std::string("Bocock Peak Provincial Park");
  orc_to_name[9786] = std::string("Butler Ridge Provincial Park");
  orc_to_name[9790] = std::string("Ekwan Lake Protected Area");
  orc_to_name[9792] = std::string("Evanoff Provincial Park");
  orc_to_name[9793] = std::string("Rubyrock Lake Provincial Park");
  orc_to_name[9794] = std::string("Foster Arm Protected Area");
  orc_to_name[9795] = std::string("Fraser River Provincial Park");
  orc_to_name[9796] = std::string("Giscome Portage Trail Protected Area");
  orc_to_name[9797] = std::string("Graham-Laurier Provincial Park");
  orc_to_name[9799] = std::string("Hay River Protected Area");
  orc_to_name[9800] = std::string("Hole-in-the-Wall Provincial Park");
  orc_to_name[9801] = std::string("Holliday Creek Arch Protected Area");
  orc_to_name[9802] = std::string("Jackman Flats Provincial Park");
  orc_to_name[9803] = std::string("Jackpine Remnant Protected Area");
  orc_to_name[9805] = std::string("Lower Raush Protected Area");
  orc_to_name[9806] = std::string("Milligan Hills Provincial Park");
  orc_to_name[9808] = std::string("Mount Blanchet Provincial Park");
  orc_to_name[9809] = std::string("Mount Pope Provincial Park");
  orc_to_name[9810] = std::string("Mudzenchoot Provincial Park");
  orc_to_name[9812] = std::string("Omineca Provincial Park");
  orc_to_name[9813] = std::string("Pink Mountain Provincial Park");
  orc_to_name[9815] = std::string("Sugarbowl-Grizzly Den Provincial Park");
  orc_to_name[9819] = std::string("Thinahtea North Protected Area");
  orc_to_name[9820] = std::string("Kotcho Lake Village Site Provincial Park");
  orc_to_name[9821] = std::string("Upper Raush Protected Area");
  orc_to_name[9822] = std::string("Sustut Provincial Park");
  orc_to_name[9824] = std::string("Mehatl Creek Provincial Park");
  orc_to_name[9825] = std::string("Homathko Estuary Provincial Park");
  orc_to_name[9828] = std::string("Dall River Old Growth Provincial Park");
  orc_to_name[9829] = std::string("Horneline Creek Provincial Park");
  orc_to_name[9830] = std::string("Scatter River Old Growth Provincial Park");
  orc_to_name[9841] = std::string("Liumchen Ecological Reserve");
  orc_to_name[9842] = std::string("Sikanni Chief Canyon Provincial Park");
  orc_to_name[9843] = std::string("Sikanni Old Growth Provincial Park");
  orc_to_name[9846] = std::string("Burnt Cabin Bog Ecological Reserve");
  orc_to_name[9847] = std::string("Call Lake Provincial Park");
  orc_to_name[9848] = std::string("Netalzul Meadows Provincial Park");
  orc_to_name[9849] = std::string("Rainbow Alley Provincial Park");
  orc_to_name[9851] = std::string("Boulder Creek Provincial Park");
  orc_to_name[9855] = std::string("Bobtail Mountain Provincial Park");
  orc_to_name[9864] = std::string("Muscovite Lakes Provincial Park");
  orc_to_name[9866] = std::string("Uncha Mountains Red Hills Provincial Park");
  orc_to_name[9867] = std::string("Wakes Cove Provincial Park");
  orc_to_name[9868] = std::string("Burgoyne Bay Provincial Park");
  orc_to_name[9869] = std::string("Collinson Point Provincial Park");
  orc_to_name[9870] = std::string("Mount Geoffery Escarpment Provincial Park");
  orc_to_name[9953] = std::string("Ancient Forest/Chun T'oh Whudujut Protected Area");
}