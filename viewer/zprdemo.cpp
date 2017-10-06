#include"ansicolor.h"
#include <iostream>
#include <deque>
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
#define STR_MAX 10000
#include "hsv.h"
#include "newzpr.h"
#include "pthread.h"
#include "memory.h"
#include "time.h"
#include "vec3d.h"
#define MYFONT  GLUT_BITMAP_HELVETICA_12
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// point data from shape files
vector< vector<vec3d> > my_vectors;
vector< string > my_names;
vector< int > my_id;
vector< int > my_class;
vector<int > n_my_class;
vector<int> within_class_index;
vector<int> urx;

vector<vec3d> max_p;
double max_f;

int next_class;

int cur_fire_ind, cur_park_ind;
int n_fire, n_park;

/* Macro for checking OpenGL error state */
#define GLERROR                                                    \
    {                                                              \
        GLenum code = glGetError();                                \
        while (code!=GL_NO_ERROR)                                  \
        {                                                          \
            printf("%s\n",(char *) gluErrorString(code));          \
                code = glGetError();                               \
        }                                                          \
    }
/* Draw axes */
#define STARTX 500
#define STARTY 500
int fullscreen;
clock_t 	start_time;
clock_t		stop_time;
#define SECONDS_PAUSE 0.4
char console_string[STR_MAX];
int console_position;
int renderflag;

void _pick(GLint name){
    cout << "PickSet:";
    std::set<GLint>::iterator it;
    for(it=myPickNames.begin(); it!=myPickNames.end(); it++){
	    int my_ind = *it;
      if(my_class[my_ind] ==1){
        my_ind -= n_fire;
      }
      cout << my_ind << "," ;
    }
    cout << " (check indexing method) " << endl;
   fflush(stdout);
}

void renderBitmapString(float x, float y, void *font, char *string){
  char *c;
  glRasterPos2f(x,y);
  for (c=string; *c != '\0'; c++)
  {
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


float a1, a2, a3;

class point{
  public:
    point(){
    }
};

/*convert float to string.. from gift meta4/gift/*/
string ftos( float i){
  std::string number("");
  std::stringstream strstream;
  strstream << i;
  strstream >> number;
  return number;
}


void drawAxes(void){

// need to calculate the area of intersection of the park with the fire centre, 
//   divided by the area of the park


    /* Name-stack manipulation for the purpose of
       selection hit processing when mouse button
       is pressed.  Names are ignored in normal
       OpenGL rendering mode.                    */

//   vector< vector<vec3d> > my_vectors;

//hsv2rgb( float *r, float *g, float *b, float h, float s, float v)


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
        hsv2rgb(&r, &g, &b, 360. * myf,  my_class[i]==1?1.:0.5, my_class[i]==1?1.:0.15); 
        vector<vec3d> * j = &my_vectors[i];
        vector<vec3d>::iterator it;
        ci = 0;
        if(picki==i){
            glColor3f(1.-r, 1.-g, 1.-b);
        }else{
            glColor3f(r,g,b);
        }
        glPushName(i);
        glBegin(GL_POLYGON);
        for(it=j->begin(); it!=j->end(); it+=1){
          if(true){ //ci % 2  ==0){
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
            if(k==0){ a = b = x.x; c = d = x.y;}
          }
          if(x.x < a) a = x.x; if(x.x > b) b = x.x;
          if(x.y < c) c = x.y; if(x.y > d) d = x.y;
        }
        wkt_f+= "))";
        if(wkt_f.length() < 999){
          cout << wkt_f << endl;
        }
        printf("%sread_wkt%s()%s n%s=%s(%s%ld%s)%s from %ld\n", KYEL, KBLU, KGRN, KYEL, KRED, KMAG, add_s, KRED, KNRM, slen);
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
            //append(p_poly, make_tuple(x.x, x.y));
            //append wktstring command
          } 
          if(k<10){
            printf("%e %e\n", x.x, x.y);
            if(k==0){ a = b = x.x; c = d = x.y;}
          }
          if(x.x < a) a = x.x; if(x.x > b) b = x.x;
          if(x.y < c) c = x.y; if(x.y > d) d = x.y;
        }
        wkt_p+= "))";     
        if(wkt_p.length() < 999){
          cout << wkt_p << endl;
        }
        printf("%sread_wkt%s()%s n%s=%s(%s%ld%s)%s from %ld\n", KYEL, KBLU, KGRN, KYEL, KRED, KMAG, add_c, KRED, KNRM, clen );
        boost::geometry::read_wkt(wkt_p, p_poly);
        printf("%scorrect%s()%s\n", KYEL, KBLU, KNRM);
        boost::geometry::correct(p_poly);
        printf("%sp_poly %si(%d) x(%f, %f) y(%f, %f)\n", KMAG, KNRM, i, a, b, c, d);
        //add first point to the end

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
       //cout << "park " << p_poly << endl;
        printf("%sintersection%s(%ld)%s\n", KYEL, KBLU, p_result.size(), KNRM);
        std::deque<polygon>::iterator it;
        // = p_result.begin();
        printf("%sarea%s()%s\n", KYEL, KBLU, KNRM);
        double my_area = 0.;
        int ci=0;
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
   GLERROR;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   drawAxes();
   drawText();
   glutSwapBuffers();
   GLERROR;
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
    }while (0 != *str++);
    return result;
}

/* Keyboard functions */
void special(int key, int x, int y){
  switch(key){
/*
  int cur_fire_ind, cur_park_ind;
  int n_fire, n_park; 
*/

/*
    case GLUT_KEY_UP:
      {
        if(cur_park_ind >=0){
          cur_park_ind ++;
          if(cur_park_ind >= n_park){
            cur_park_ind = 0;
          }
        }else{
          cur_park_ind = 0;
        }
      }
    break;  
    case GLUT_KEY_DOWN:
       {
        if(cur_park_ind >=0){
          cur_park_ind --;
          if(cur_park_ind <0){
            cur_park_ind = n_park-1;
          }
        }else{
          cur_park_ind = 0;
        }
      }
    break;
*/
    case GLUT_KEY_LEFT:
      {
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
    case GLUT_KEY_RIGHT:
      {
        if(cur_fire_ind >=0){
          cur_fire_ind ++;
          if(cur_fire_ind >= n_fire){
            cur_fire_ind = 0;
          }
        }else{
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
	case 13 :
  {
		printf( "%d Pressed RETURN\n",(char)key);

    string mys(&console_string[0]);

    vector<string> words(split(mys.c_str()));
    if(words.size()==2){
      cout << "good "<<endl;
      if(words[0][0]=='f'){
        int iii = atoi(words[1].c_str());
        if(iii >=0 && iii < n_fire){
          cur_fire_ind = iii; 
        }else{
          cur_fire_ind = -1;
        }
      }
      if(words[0][0]=='p'){
         int iii=atoi(words[1].c_str());
        if(iii>=0 && iii< n_park){
          cur_park_ind = iii;
        }else{
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

static GLfloat light_ambient[]  = { 0.0, 0.0, 0.0, 1.0 };
static GLfloat light_diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

static GLfloat mat_ambient[]    = { 0.7, 0.7, 0.7, 1.0 };
static GLfloat mat_diffuse[]    = { 0.8, 0.8, 0.8, 1.0 };
static GLfloat mat_specular[]   = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat high_shininess[] = { 100.0 };


void idle(){
    if(renderflag){
		glFlush();
        glutPostRedisplay();
	}	
}

int parse(string fn){
    ifstream in(fn.c_str());
    if (!in.is_open()){
        printf("error\n"); exit(1);
    }
    string line;

    getline(in, line);
    cout << line << endl;
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

int main(int argc, char *argv[]){

cout << "BOOST_VERSION\t\t" << BOOST_VERSION<< endl;
cout << "BOOST_PATCH_LEVEL\t\t" << BOOST_VERSION % 100 << endl;
cout << "BOOST_MINOR_VERSION\t\t" << BOOST_VERSION / 100 << endl;
cout << "BOOST_MAJOR_VERSION\t\t" << BOOST_VERSION / 100000 << endl;
cout << "Boost version:\t\t" << BOOST_LIB_VERSION << endl;

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
    cout << "number of fire centres " << nc0 <<endl;
    cout << "number of parks        " << nc1 <<endl;
    fclose(fff);
    fclose(ggg);

    if(true){
    int i;
    next_class = 0;
    float minx, maxx, miny, maxy;
    minx = maxx = miny = maxy = 0.;
    for(i=0; i< my_vectors.size(); i++){
        vector<vec3d> * j = &my_vectors[i];
        vector<vec3d>::iterator it;
        for(it = j->begin(); it != j->end(); it++){
            float x = (*it).x;  float y = (*it).y;
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

    xa /= (c);  ya /= (c); 

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
	//Py_Initialize();
	//printf("Py_init()\n");
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
	   // zprInit();
    zprSelectionFunc(drawAxes);     /* Selection mode draw function */
    zprPickFunc(pick);              /* Pick event client callback   */

     /* Initialise OpenGL */

    /* Enter GLUT event loop */
    glutMainLoop();
    return 0;
}



