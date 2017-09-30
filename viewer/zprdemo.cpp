#include <iostream>
#include <deque>

#include <boost/assign.hpp>
#include <boost/version.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp> //#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/adapted/boost_range/filtered.hpp>

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

/* 
http://www.boost.org/doc/libs/1_47_0/libs/geometry/doc/html/geometry/reference/algorithms/append.html
http://www.boost.org/doc/libs/1_47_0/libs/geometry/doc/html/geometry/reference/algorithms/intersection.html
*/


#define STR_MAX 10000
#include "hsv.h"
#include "newzpr.h"
#include "pthread.h"
#include"memory.h"
#include "time.h"
#include "vec3d.h"
#define MYFONT  GLUT_BITMAP_HELVETICA_12
#include <stdio.h>
#include <stdlib.h>
#include<vector>

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

int cur_fire_ind;
int cur_park_ind;
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
/* 
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

int cur_fire_ind;
int cur_park_ind;
int n_fire, n_park;
*/


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
        } // for(it=j->begin()
        glEnd();
        glPopName();
    }
    if(cur_fire_ind>=0 && cur_fire_ind < n_fire && cur_park_ind>=0 && cur_park_ind < n_park){
      int k;

      using boost::assign::tuple_list_of;
      using boost::make_tuple;
      using boost::geometry::append;
      typedef boost::geometry::model::polygon<boost::tuple<float, float> > polygon;

// Append a range
  ///  append(poly, tuple_list_of(0, 0)(0, 10)(11, 11)(10, 0));
    // Append a point (in this case the closing point
 //   append(poly, make_tuple(0, 0));

        polygon f_poly;
        int i = cur_fire_ind;  
        long int slen = my_vectors[i].size();
        vector<vec3d> * v = &my_vectors[i];
        for(k=0; k< slen; k++){
          vec3d x(v->at(k));
          append(f_poly, make_tuple(x.x, x.y));
        }
        {
          vec3d x(v->at(0));
          append(f_poly, make_tuple(x.x, x.y));
        }
  // add the last point in????        //s[slen-1].x = s[0].x;          //s[slen-1].y = s[0].y;

        polygon p_poly;
        int j = cur_park_ind + n_fire;
        long int clen = my_vectors[j].size();
        v = &my_vectors[j];
        for(k=0; k< clen; k++){
          vec3d x(v->at(k));
          append(f_poly, make_tuple(x.x, x.y));
        }


    }
    
glPopMatrix();
/*
	glPushMatrix();
	glPushName(0);
	glTranslatef( a1, a2, a3);
	glColor3f(1,0,0);
	if(myPickNames.count(0)){ glColor3f(0,1,1);}
	glutSolidSphere(0.5, 8,8  );
	glPopName();
	glPopMatrix();

	glPushMatrix();
	glPushName(4);
	glColor3f(1,0,0);    
	if(myPickNames.count(4)){ glColor3f( 0,1,1);}// 1- 0.3*a1,1-0.3*a2,1-1);}//;}
 	glutSolidSphere(0.7, 20, 20);
	glPopName();
	glPopMatrix();

	glPushMatrix();
  glPushName(1);      
  glColor3f(0,0,1);
	if(myPickNames.count(1)){ glColor3f(1,1,0);}
        glRotatef(90.*a1,0,1,0);
        glutSolidCone(0.6, 4.0, 20, 20);
      	glPopName();
      	glPopMatrix();

      	glPushMatrix ();
      	glPushName(2);  
       	glColor3f(0,0,1);
	if(myPickNames.count(2)){ glColor3f(1,1,0);}
        glRotatef(-90.*a1,1,0,0);
        glutSolidCone(0.6, 4.0, 20, 20);
      	glPopName();
      	glPopMatrix();

	glPushMatrix();
      	glColor3f(0,0,1);  
	if(myPickNames.count(3)){ glColor3f(1,1,0);}
      	glPushName(3);
        glutSolidCone(0.6, 4.0, 20, 20);
      	glPopName();
   	glPopMatrix();

*/
}

/* Callback function for drawing */
void display(void)
{
   GLERROR;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   drawAxes();
   //glutSwapBuffers();//Flush();
   drawText();
   //glutPostRedisplay();
   //glFlush();
   glutSwapBuffers();

   GLERROR;
	renderflag = false;
}

/* Callback function for pick-event handling from ZPR */





void quitme(){
	exit(0);

}


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
void keyboard(unsigned char key, int x, int y){
  switch(key){
	// Backspace

	/*case GLUT_F1:
		//if( stop_time > clock())
		//	break;
		if(!fullscreen){
			fullscreen=1;
			glutFullScreen();

		}else{
			fullscreen=0;
			glutReshapeWindow(STARTX, STARTY);
			glutPositionWindow(0, 0);

		}
		glutPostRedisplay();
		//display();
  		//start_time = clock();
		//stop_time = start_time + CLOCKS_PER_SEC;
		//while(clock() < stop_time){}

		break;
	*/
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
        }
      }
      if(words[0][0]=='p'){
         int iii=atoi(words[1].c_str());
        if(iii>=0 && iii< n_park){
          cur_park_ind = iii;
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
		//printf( "%d Pressed Esc\n",(char)key);
		break;

	// Delete
/*	case 127 :
		printf( "%d Pressed Del\n",(char)key);
		break;
*/
	default: 
		//printf( "Pressed key %c AKA %d at position %d % d\n",(char)key, key, x, y);
		console_string[console_position++] = (char)key;
		console_string[console_position]='\0';
		printf("STRING: %s\n", &console_string[0]);
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

/* Entry point */

//https://computing.llnl.gov/tutorials/pthreads/
/*void * threadfun(void *arg){

	while(true){
		printf("Sleeping...\n");

		usleep(200000);
		a1 = a1+0.5;
		a2 = a2-0.5;
		a3 = a3*1.5;
		renderflag = true;
		printf("Sleeping\n");

		usleep(200000);
		a1 = a1-0.5;
                a2 = a2+0.5;
                a3 = a3/1.5;
		renderflag = true;

	}

}
*/

void idle(){
    if(renderflag){
		glFlush();
        glutPostRedisplay();
	}	
}
/* 
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
*/


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

//vector<vec3d> max_p;
//double max_f;
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

    if(false){ // intersection calculation
        int i, j, k;
        // vector< vector<vec3d> > my_vectors; 
        int npts = my_vectors.size();

        for(i=0; i< nc0; i++){ // iterate over all the fire centres
            //vec_t s[] = {{50,150},{200,50},...} 
            long int slen = my_vectors[i].size();
            //vec_t * s = (vec_t *)(void *)malloc(sizeof(vec_t) * slen);
            //vector<vec3d> * v = &my_vectors[i];
            //for(k=0; k< v->size(); k++){
                //vec3d x(v->at(k));
                //s[k].x = x.x; s[k].y = x.y;
            //}
            //poly_t subject = {slen, 0, s};
            for(j=nc0; j< nc0+nc1; j++){ // iterate over all the parks
                ///fprintf(fff, "i %d/%d j %d/%d\n", i+1, nc0, j-nc0+1, nc1);
                //long int clen = my_vectors[j].size();
                //vec_t * c = (vec_t *)(void *)malloc(sizeof(vec_t) * clen);
                //v = &my_vectors[j];
                //for(k=0; k< clen; k++){
                  //  vec3d x(v->at(k));
                   // c[k].x = x.x;  c[k].y = x.y;
                //}
                //poly_t clipper = {clen, 0, c};
                // the polygon calculation
                {
                        //printf("%e AREA of subject\n", area(&subject));
                        //printf("%e AREA of clipper\n", area(&clipper));
                        //poly res = poly_clip(&subject, &clipper); //subject, &clipper);
                        //for (k = 0; k < res->len; k++)
                        //printf("%g %g\n", res->v[k].x, res->v[k].y);
                        //double ar = area(res);
                        //if(a>0.){
                            //printf("i %d j %d %e AREA of Intersection\n", i+1, j-nc0+1, ar);
                            //fprintf(ggg, "%d,%d,%e\n", i+1, j-nc0+1, ar);  
    
                        /*if(ar > max_f){
                           max_f = ar;
                           max_p.clear();
                            int kk;
                            for (kk = 0; kk < res->len; kk++){
                              max_p.push_back( vec3d(res->v[k].x, res->v[k].y, 0.));
                            }
                        }        */
            
                      //}//area(res));
                }
                //free(c);
             } // iterate over all the parks

            //free(s);
        }// iterate over all the fire centres
    }
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
        for(it=j->begin(); it!=j->end(); it++){
            float x = (*it).x;  float y = (*it).y;
            if (x>maxx) maxx=x; if (x<minx) minx=x;
            if (y>maxy) maxy=y; if (y<miny) miny=y;
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
            xa += (*it).x;  ya += (*it).y;
            c += 1.;
        }    
      }

    xa /= (c);  ya /= (c); 

    for(i=0; i< my_vectors.size(); i++){
        vector<vec3d> * j = &my_vectors[i];
        vector<vec3d>::iterator it;
        for(it=j->begin(); it!=j->end(); it++){
            (*it).x -= xa; (*it).y -= ya;
            (*it).x *= 5.; (*it).y *= 5.;
        }
      }
    }

    pick = _pick;
	renderflag = false;
	a1=a2=a3=1;
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



