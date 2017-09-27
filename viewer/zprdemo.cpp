#define STR_MAX 10000
#include "hsv.h"
#include "newzpr.h"
#include "pthread.h"
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

int next_class;

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
	cout << *it << "," ;
    }
    cout << endl;
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
          if(ci % 13 ==0){
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

/* Keyboard functions */
void keyboard(unsigned char key, int x, int y)
{
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
			printf("STRING: %s\n", &console_string[0]);
			//printf( "%d Pressed Backspace\n",(char)key);
			display();
		}
		break;
		

	// Enter
	case 13 :
		//printf( "%d Pressed RETURN\n",(char)key);
		console_string[0]='\0';
		console_position=0;
		display();
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


void  idle(){
	if( renderflag ){
		glFlush();
    		glutPostRedisplay();
	}	
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



void parse(string fn){
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
        //cout << a[0] << endl;
        //vector< string > my_names;
        //vector< int > my_id;
        my_names.push_back(feature_name);
        my_id.push_back(feature_id);
        my_class.push_back(next_class);
        nclass++;
    }
    int i; for(i=0; i<nclass; i++){
    n_my_class.push_back(nclass);
    }
    next_class ++;
}

int main(int argc, char *argv[]){
    my_vectors.clear();
    parse(string("firec.dat"));
    parse(string("parks.dat"));

    if(true){
    int i;
    next_class = 0;
    float minx, maxx, miny, maxy;
    minx=maxx=miny=maxy = 0.;
    for(i=0; i< my_vectors.size(); i++){
        vector<vec3d> * j = &my_vectors[i];
        vector<vec3d>::iterator it;
        for(it=j->begin(); it!=j->end(); it++){
            float x = (*it).x;
            float y = (*it).y;
            if (x>maxx) maxx=x;
            if (x<minx) minx=x;
            if (y>maxy) maxy=y;
            if (y<miny) miny=y;
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
            xa += (*it).x; // -= minx; (*it).x /= (maxx - minx);
            ya += (*it).y;// -= miny; (*it).y /= (maxy - miny);
            c += 1.;
        }    
      }

    xa /= (c);
    ya /= (c);

    for(i=0; i< my_vectors.size(); i++){
        vector<vec3d> * j = &my_vectors[i];
        vector<vec3d>::iterator it;
        for(it=j->begin(); it!=j->end(); it++){
            (*it).x -= xa; // -= minx; (*it).x /= (maxx - minx);
            (*it).y -= ya;// -= miny; (*it).y /= (maxy - miny);
            (*it).x *= 5.;//* ((maxx - minx)/(maxy - miny)) ;
            (*it).y *= 5.;// * ((maxy - miny)/(maxx - minx));
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

    GLERROR;
/* 
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

     glEnable(GL_LIGHTING);
     glEnable(GL_LIGHT0);
     glDepthFunc(GL_LESS);
     glEnable(GL_DEPTH_TEST);
     glEnable(GL_NORMALIZE);
     glEnable(GL_COLOR_MATERIAL);
*/
     GLERROR;

//	pthread_t thread;
//	pthread_create(&thread, NULL, &threadfun, NULL);
    /* Enter GLUT event loop */
    glutMainLoop();
    return 0;
}



