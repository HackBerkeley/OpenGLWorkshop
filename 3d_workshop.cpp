// Simple OpenGL example for CS184 F06 by Nuttapong Chentanez, modified from sample code for CS184 on Sp06
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <list>
#include <vector>
#include "Eigen/Core"
#include "Eigen/Geometry"
#include "Eigen/LU"
#include "Eigen/Householder"
#include "Eigen/Eigenvalues"
#include "Eigen/Dense"
#include "Eigen/Eigen"
//#include "FreeImage.h"

using namespace Eigen;
using namespace std;

#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define PI 3.14159265

/****************
 * 3 - 2 - 1 Euler Angles
 * **************/
 

float psi = 1.435f;
float theta = 0.0f;
float phi = 0.0f;

float psiDot = 0.0f;
float thetaDot = 0.0f;
float phiDot = 0.0f;

float omegaOne = 0.0f;
float omegaTwo = 0.0f;
float omegaThree = 0.0f;

float e1speed = 0.0f;
float e2speed = 0.0f;
float e3speed = 0.0f;

float omegaSpeed = 0.35f;
float rotationTimestep = 0.1f;
float translateSpeed = 0.2f;

bool isTranslatinge1 = false;
bool isTranslatinge2 = false;
bool isTranslatinge3 = false;

bool isRotatinge1 = false;
bool isRotatinge2 = false;
bool isRotatinge3 = false;

Vector3f e1;
Vector3f e2;
Vector3f e3;

Vector3f translationVector(0,-8,0);

/***************
GLOBALS: global variables
***************/

float scaleAmount = 2.75f;
float stepSize = 0.02f;

float rotateDelta = 5.1f;
float transDelta = 0.1f;

bool isSmoothShading = false;
bool isWireframe = false;
bool isAdaptive = false;
bool isHiddenWire = false;
bool showGrid = false;
bool paused = true;
bool collisionsOn = true;
bool stickyCollisions = false;
bool elasticCollisions = true;
bool trailsOn = true;

/****************
* GUI interface global variables
*
****************/


/****************
* Prototype Functions
* (defined here so they can be hidden down below)
****************/
						
//(yes I know these should be in a header but not everyone is a c++ expert)

Vector3f getXZplanePoint(int x, int y);
void handleKeyUp(unsigned char key, int x, int y);
void handleArrowUp(int key, int x, int y);
void handleKeypress(unsigned char key, int x, int y);
void handleArrowPress(int key, int x, int y);
void drawScene(void);
void handleResize(int w, int h);
void initRendering(void);
void calculateEulerAngleDots(void);
void deleteStuff(void);
void drawAxes(void);
void calculateLittleEs(void);
void deleteStuff(void);
void handleSmoothKeys(void);

/************* Constants for Configuration ****************/
float particleSize = 0.08f;
float particleDensity = 488.28f;
float trailSizeInit = particleSize / 3.0f;
float trailDecayFactor = 1/60.0f;
float massRandInit = 40.0f;
float positionRandInit = 20.0f;
float velocityRandInit = 6.0f;
float deltaT = 0.001f;
float timeScaleFactor = 102.0f;
int numParticles = 10;
int trailLength = 60;

/************** Workshop Globals ***********/

Vector3f teapotLocation;

/****************
* Math Functions
*
****************/

//helpful random function between 0 and 1
float myRand()
{
	return rand()*1.0f/RAND_MAX;
}


//rayPlaneIntersect:
	//intersect a ray with a plane and return the value of t
	//that corresponds to the intersection point
	//
	// (-1 returned for no intersection)
float rayPlaneIntersect(Vector3f origin, Vector3f direction, Vector3f planePoint, Vector3f planeNormal)
{
	//first get the origin - line origin
	Vector3f diff = planePoint - origin;
	//dot with normal
	float numerator = diff.dot(planeNormal);

	if(abs(numerator) <= 0.00001f)
	{
		//no intersection
		cout << "numerator is 0!!!";
		return -1.0f;
	}

	float denom = direction.dot(planeNormal);
	if(abs(denom) <= 0.00001f)
	{
		//parallel to plane
		cout << "Parallel??";
		return -1.0f;
	}

    //not parallel and there is a intersection, so just return the value!
	return numerator / denom;

}

//planePoint
	//get a point on the plane with the given normal that intersects the origin
	//
	// (just a shortcut for not having to specify the origin point every time
	// 		and the line math)
Vector3f planePoint(Vector3f lineOrigin, Vector3f direction, Vector3f planeNormal)
{
	Vector3f origin(0,0,0);
	float deltaT = rayPlaneIntersect(lineOrigin,direction,origin,planeNormal);
	Vector3f point = lineOrigin + deltaT * direction;
	return point;
}


/***************CLASSES************
**********************************/
class Particle;
class Snowman;

list <Particle> allParticles;
list <Snowman> allSnowmen;


class Particle
{
private:
	Vector3f pos, vel, accel;
	list<Vector3f> trailList;
	float radius;
	int myId;
public:

	/********
	* Boring initializers and equivalence checkers
	*
	*********/

	Particle()
	{
		//default values
		this->pos = Vector3f(0,0,0);
		this->accel = Vector3f(0,0,0);
		this->vel = Vector3f(0,0,0);
		this->radius = 0.08f;
	}
	Particle(Vector3f pos, Vector3f vel, Vector3f accel, int id,float radius)
	{
		this->pos = pos; this->vel = vel; this->accel = accel; this->myId = id;
		this->radius = radius;
	}
	
	int operator==(Particle rhs)
	{
		if(myId == rhs.getId()) 
		{return 1;}
		return 0;
	}

	void print()
	{
		cout << "Printing particle ID: " << myId << "\n";
		cout << "Position:\n" << pos;
		cout << "Vel:\n" << vel;
		cout << "Accel:\n" << accel << "\n";
	}
	
	
	/*********
	* Actual functionality!
	*
	*********/
	
	void collisionCheck()
	{
		//TODO_collision
		
		//Check for a collision with the ground plane
		
			//if so, reverse your velocity y component!
		
		
    }
    
	void updatePosition(float deltaT)
	{
		trailList.push_back(pos);

		//standard kinematic equation
		pos = pos + vel * deltaT + deltaT * deltaT * 0.5f * accel;

		//collision detection
		collisionCheck();
	}
	
	void drawMyself()
	{
		glPushMatrix();
		glTranslatef(pos(0),pos(1),pos(2));
		glutSolidSphere(radius,8,8);

		glPopMatrix();
		
		drawTrails();
	}
	
	
	
	
	
	
	
	
	
	
	
	
	void drawTrails()
	{
		//////////////////////////////
		// Trail drawing below, don't worry about this
		/////////////////////////////
		
		//variable inits
		bool first = true;
		//our current position in the list (we go backwards)
		int expCounter = trailList.size();
		//initial size of trail particles
		float tSizeInit = radius / 1.5f;
		//how many sides of the spheres
		int sides = 4;
		//our current size
		float currentSize = tSizeInit * exp(-expCounter*trailDecayFactor);
		
		//Loop over trail positions, drawing each one
		//and decaying size
		for(list<Vector3f>::iterator trailPos=trailList.begin();trailPos!=trailList.end();++trailPos)
		{
			//if we are the first and our trail is too long,
			if(first && trailList.size() > trailLength)
			{
				//delete
				trailPos = trailList.erase(trailPos);
				first = false;
			}
			
			if(trailsOn)
			{
					//draw like normal
					glPushMatrix();
					glTranslatef(trailPos->coeff(0),trailPos->coeff(1),trailPos->coeff(2));
					glutSolidSphere(currentSize,sides,sides);
					glPopMatrix();
			}
			expCounter--;
			currentSize = tSizeInit * exp(-expCounter*trailDecayFactor);
			//currentSize = trailSizeInit;
		}

	}

	
	/**********
	* Boring Getters and Setters
	***********/
	
	
    //for particle picking
    void setRadius(float theRad)
    {
        this->radius = theRad;
    }
	
	Vector3f getPos()
	{
		return pos;
	}

	Vector3f getVel()
	{
		return vel;
	}

	Vector3f getAccel()
	{
		return accel;
	}

	void setVel(Vector3f velocity)
	{
		vel = velocity;
	}

	void setPos(Vector3f poss)
	{
		pos = poss;
	}

	float getRadius()
	{
		return radius;
	}
	
	int getId()
	{
		return myId;
	}

};

class Snowman
{
private:
    Vector3f pos, vel;
    float width;
    float height;
    int myId;
public:
    
    //Initializers

    Snowman()
    {
        this->pos = Vector3f(0,0,0);
        this->vel = Vector3f(1,1,0);
        this->width = 1.0f;
        this->height = 1.0f;
        this->myId = 1;
    }
    Snowman(Vector3f pos, Vector3f vel, float width, float height, int id)
    {
        this->pos = pos; this->vel = vel; this->width = width; this->height = height;
        this->myId = id;
    }

    int operator==(Snowman otherSnowman)
    {
        if(this->myId == otherSnowman.getId())
        {
            return 1;
        }
        return 0;
    }

    int getId()
    {
        return this->myId;
    }



    void drawMyself()
    {




    }

    void collisionCheck()
    {



    }

    void updatePosition(float deltaT)
    {




    }



};


float rayParticleIntersectTest(Vector3f rayStart, Vector3f direction, Particle * testPart)
{
    Vector3f center = testPart->getPos();
    float radius = testPart->getRadius();

    float A = direction.dot(direction);
    float B = 2 * (direction.dot((rayStart - center)));
    float C = (rayStart - center).dot((rayStart - center)) - radius*radius;

    float underSquare = B * B - 4 * A * C;

    if(underSquare < 0)
    {
        //imaginary
        return -1.0f;
    }

    float underSquared = sqrt(underSquared);

    float t1 = (-B + underSquared) / (2 * A);
    float t2 = (-B - underSquared) / (2* A);

    if(t1 < 0 && t2 < 0)
    {
        return -1.0f;
    }
    if(t1 > 0 && t2 > 0)
    {
        return min(t1,t2);
    }
    if(t1 < 0 && t2 > 0)
    {
        return t2;
    }
    else
    {
        return t1;
    }
    return 0.0f;
}

int selectFromAllParticles(Vector3f rayStart, Vector3f direction)
{
    float tRecord = 100000000.0f;
    int idRecord = -1;

    //the idea is to loop through all the particles in the current list
    //and then intersect rays with them
    for(list<Particle>::iterator currParticle=allParticles.begin();currParticle!=allParticles.end();++currParticle)
    {
        //do an intersect test and get the t
        Particle p = *currParticle;
        float thisT = rayParticleIntersectTest(rayStart,direction,&p);

        if(thisT > 0.0f)
        {
            //it actually intersected!
            if(thisT < tRecord)
            {
                //it's closer! update the stuff
                tRecord = thisT;
                idRecord = currParticle->getId();
            }
        }
    }

    return idRecord;
}

/****************************
*
*
*			3D accessory functions (ray picking, etc)
*
*
*****************************/

//get the ray corresponding to a mouse click on the screen
Vector3f rayPick(int x, int y, float * origin)
{
	
	//openGl stuff
	
    GLdouble modelMatrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
	GLdouble projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);

	GLdouble nearPosition[3];
	GLdouble farPosition[3];

	int height = viewport[3];
	
	
	//unproject once on the near clipping plane
	gluUnProject(x,
		height - y,
		0.0,
		modelMatrix,
		projMatrix,
		viewport,
		&nearPosition[0],
		&nearPosition[1],
		&nearPosition[2]);
	//unproject again, but onto the far clipping plane
	gluUnProject(x,
		height - y,
		1.0,
		modelMatrix,
		projMatrix,
		viewport,
		&farPosition[0],
		&farPosition[1],
		&farPosition[2]);
	
    Vector3f near(    nearPosition[0],
                        nearPosition[1],
                        nearPosition[2] );
    Vector3f far(       farPosition[0],
                        farPosition[1],
                        farPosition[2] );
	//doing far - near didn't work for whatever reason...
    Vector3f direction(farPosition[0] - nearPosition[0],
						farPosition[1] - nearPosition[1],
						farPosition[2] - nearPosition[2]);
	
    origin[0] = nearPosition[0];
    origin[1] = nearPosition[1];
    origin[2] = nearPosition[2];
	
	//return the direction of the ray and also
	//store the origin of the array
    return direction;
    
}

/**********************************/


void updateTeapotPosition(int x, int y)
{
   //to pass into function
   float originArray[3];
   
   //get the ray direction and origin
   Vector3f direction = rayPick(x,y,originArray);
   Vector3f origin(originArray[0],originArray[1],originArray[2]);
	
	//get the direction and origin
	//find the point with which it intersects the x y plane
	Vector3f planeNormal(0,1,0);
	teapotLocation = planePoint(origin,direction,planeNormal);
	
}


//the function for mouse click and dragging
void handleMotion(int x, int y)
{
    cout << "Mouse down motion: To (" << x << "," << y << ")" << endl;
   	updateTeapotPosition(x,y);
}


//the function for mouse clicks
void handleClick(int button, int state, int x, int y)
{
	cout << "Mouse click: Button " << button << " state " << state << " // here (" << x << "," << y << ")" << endl;

	//OpenGL enumeration quick reference below:
		//GLUT_LEFT_BUTTON
		//GLUT_MIDDLE_BUTTON
		//GLUT_RIGHT_BUTTON
		//GLUT_DOWN -> pushed down
		//GLUT_UP -> released
	
    float origin[3];
    Vector3f direction = rayPick(x, y, origin);
    Vector3f originPos(origin[0],origin[1],origin[2]);

    updateTeapotPosition(x,y);
	
}


void handleDrawing()
{

	//draw the teapot
	glPushMatrix();
	glTranslatef(teapotLocation(0),teapotLocation(1),teapotLocation(2));
	glutSolidTeapot(0.25f);
	glPopMatrix();

	
	
	//draw the origin lines
	drawAxes();


	for(list<Particle>::iterator currParticle=allParticles.begin();currParticle!=allParticles.end();++currParticle)
	{
		currParticle->drawMyself();
	}

}




void update(int value) {
	
	handleSmoothKeys();

	if(!paused)
	{
        //update particles
		for(list<Particle>::iterator currParticle=allParticles.begin();currParticle!=allParticles.end();++currParticle)
		{
			currParticle->updatePosition(deltaT*timeScaleFactor);
		}
	}
	
	glutPostRedisplay();
	//reset our timer
	glutTimerFunc(25, update, 0);
}


int main(int argc, char** argv) {
	
	//seed random number generator
	srand(time(NULL));
	
	//Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	
	//window size
	glutInitWindowSize(600, 400);
	
	//Create the window
	glutCreateWindow("Hackers @ Berkeley - Intro to 3D workshop");
	initRendering();
	
	//Set handler functions
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutMotionFunc(handleMotion);
	glutMouseFunc(handleClick);
	glutKeyboardUpFunc(handleKeyUp);
	glutSpecialFunc(handleArrowPress);
	glutSpecialUpFunc(handleArrowUp);
	glutReshapeFunc(handleResize);
	
	glutTimerFunc(25, update, 0); //Add a timer
	
	//update euler angles
	calculateLittleEs();

	//do the main loop!
	glutMainLoop();
	return 0;
}






























//lookout below!!




















/********
*
*				Standard OpenGL stuff
*
*********/

//Initializes 3D rendering
void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING); //Enable lighting
	glEnable(GL_LIGHT0); //Enable light #0
	glEnable(GL_LIGHT1); //Enable light #1
	glEnable(GL_NORMALIZE); //Automatically normalize normals
	glShadeModel(GL_SMOOTH); //Enable smooth shading
}

//Called when the window is resized
void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}


//Draws the 3D scene
void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glScalef(scaleAmount,scaleAmount,scaleAmount);

	//point to look at
	Vector3f pointToLookAt = translationVector + e1;

	gluLookAt(	translationVector(0), translationVector(1), translationVector(2),
			pointToLookAt(0),pointToLookAt(1),pointToLookAt(2),
			e3(0),e3(1),e3(2));

	//we loaded identity, so we are in Cartesian here
	glRotatef(psi*PI/180.0f,		0.0f,0.0f,1.0f);
	//after first rotation
	glRotatef(theta*PI/180.0f,	0.0f,1.0f,0.0f);
	//glRotatef(phi,		e3(0),e3(1),e3(2));
	glRotatef(phi*PI/180.0f,		0.0f,0.0f,1.0f);
	
	//Add ambient light
	GLfloat ambientColor[] = {0.1f, 0.1f, 0.1f, 1.0f}; //Color 
	GLfloat specularColor[] = {1.0f,1.0f,1.0f,1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	
	//Add positioned light
	GLfloat lightColor0[] = {0.0f, 0.0f, 0.9f, 1.0f}; //Color 
	GLfloat lightPos0[] = {-10.0f, 10.5f, 10.5f, 1.0f}; //Positioned at 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
	
	//Add directed light
	GLfloat lightColor1[] = {0.9f, 0.9f, 0.9f, 1.0f};
	GLfloat lightPos1[] = {-1.0f, -1.5f, 0.0f, 1.0f};
	GLfloat ambientLight[] = {0.3f,0.3f,0.3f,1.0f};
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularColor);

	
	if(!isHiddenWire)
	{
		//draw normally
		glEnable(GL_DEPTH_TEST);
		if(!isWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

		glColor3f(1.0f,1.0f,1.0f);
		handleDrawing();
	}
	else
	{
		//do hidden wire
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glColor3f(1.0f,1.0f,1.0f);
		handleDrawing();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
		glColor3f(0.0f,0.0f,0.0f);
		handleDrawing();
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	//swap buffers onto screen
	glutSwapBuffers();
}

void drawAxes()
{
	float axisLength = 20.0f;
	float axisSize = 0.03f;

	glPushMatrix();
	glTranslatef(axisLength/2.0f,0,0);
	glScalef(axisLength,axisSize,axisSize);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0,axisLength/2.0f,0);
	glScalef(axisSize,axisLength,axisSize);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0,0,axisLength/2.0f);
	glScalef(axisSize,axisSize,axisLength);
	glutSolidCube(1.0f);
	glPopMatrix();
}

/*********
*
*				Keyboard and Mouse stuff... ignore most of this
* 
***********/

void calculateEulerAngleDots()
{
	psiDot =   0 * omegaOne + (sin(phi)/cos(theta))*omegaTwo + (cos(phi)/cos(theta))*omegaThree;
	thetaDot = 0 * omegaOne + cos(phi) * omegaTwo		 + -sin(phi) * omegaThree;
	phiDot =   1 * omegaOne + sin(phi) * tan(theta)*omegaTwo + cos(phi)*tan(theta)*omegaThree;
}

void calculateLittleEs()
{
	//we have to make a bunch of matrices here :-/
	Matrix3f left;
	Matrix3f middle;
	Matrix3f right;
	left << 	cos(psi), 	-sin(psi), 	0,
		 	sin(psi), 	cos(psi),  	0,
			0,		0,		1;

	middle << 	cos(theta),	0,	sin(theta),
	       		0,		1,		0,
			-sin(theta),	0,	cos(theta);
	
	right <<	1,		0,		0,
	      		0,		cos(phi),	-sin(phi),
			0,		sin(phi),	cos(phi);

	//multiply all of them
	Matrix3f Q = left * middle * right;
	Vector3f E1(1,0,0);
	Vector3f E2(0,1,0);
	Vector3f E3(0,0,1);

	e3 = Q(0,2)*E1 + Q(1,2) * E2 + Q(2,2) * E3;
	e2 = Q(0,1)*E1 + Q(1,1) * E2 + Q(2,1) * E3;
	e1 = Q(0,0)*E1 + Q(1,0) * E2 + Q(2,0) * E3;

	e1.normalize();
	e2.normalize();
	e3.normalize();

	//cout << "\nUpdating e's\ne1\n" << e1 << "\ne2\n" << e2 << "\ne3\n" << e3;

}


void handleKeyUp(unsigned char key, int x, int y)
{
	//cout << "\nThis was released:" << int(key);
	switch (key) {
		case 119: //w
			isTranslatinge1 = false;
			break;
		case 115: //s
			isTranslatinge1 = false;
			break;
		case 97: //a
			isTranslatinge2 = false;
			break;
		case 100: //d
			isTranslatinge2 = false;
			break;
		case 32: //space
			isTranslatinge3 = false;
			break;
		case 113: //q
			isTranslatinge3 = false;
			break;
		case 61: //+
			isRotatinge1 = false;
			omegaOne = 0.0f;
			break;
		case 45: //-
			isRotatinge1 = false;
			omegaOne = 0.0f;
			break;
	}
}


void handleArrowUp(int key, int x, int y)
{
	int mod = glutGetModifiers();

	switch(key)
	{
			case GLUT_KEY_LEFT:
				isRotatinge3 = false;
				omegaThree = 0.0f;
				break;
			case GLUT_KEY_RIGHT:
				isRotatinge3 = false;
				omegaThree = 0.0f;
				break;
			case GLUT_KEY_UP:
				isRotatinge2 = false;
				omegaTwo = 0.0f;
				break;
			case GLUT_KEY_DOWN:
				isRotatinge2 = false;
				omegaTwo = 0.0f;
				break;
	}
}


void handleKeypress(unsigned char key, int x, int y) {
	//cout << "\nThis was pressed:" << int(key);
	int mod = glutGetModifiers();
	if(mod)
	{
		return;
	}

	switch (key) {
		case 27: //Escape key
			deleteStuff();
			exit(0);
			break;
       case 109: //m
			cout << "\nSwitching shading model";
			if(isSmoothShading)
				glShadeModel(GL_FLAT);
			else
				glShadeModel(GL_SMOOTH);
			isSmoothShading = !isSmoothShading;
			break;
		case 104: //h  hidden wire frame thing
			cout << "\nSwitching to hidden wire";
			if(isHiddenWire)
			{
				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			}
			isHiddenWire = !isHiddenWire;
			trailLength++;
			break;
		case 116: //t
			cout << "\nSwitching trails";
			trailsOn = !trailsOn;
			break;
		case 120: //x
			cout << "\nSwitching wireframe choice";
			if(!isWireframe)
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			else
				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			isWireframe = !isWireframe;
			break;
		case 112: // p
			cout << "\nPausing / Playing";
			paused = !paused;
			break;

		//MOVEMENT keys!!! for translation
		case 119: //w
			isTranslatinge1 = true;
			e1speed = translateSpeed;
			break;
		case 115: //s
			isTranslatinge1 = true;
			e1speed = -translateSpeed;
			break;
		case 97: //a
			isTranslatinge2 = true;
			e2speed = translateSpeed;
			break;
		case 91: //slow down
			timeScaleFactor -= 10.5;
			break;
		case 93: //speed up
			timeScaleFactor += 10.5;
			break;
		case 100: //d
			isTranslatinge2 = true;
			e2speed = -translateSpeed;
			break;
		case 32: //space
			isTranslatinge3 = true;
			e3speed = translateSpeed;
			break;
		case 113: //b?
			isTranslatinge3 = true;
			e3speed = -translateSpeed;
			break;
		case 61: //+
			isRotatinge1 = true;
			omegaOne = omegaSpeed;
			break;
		case 45: //-
			isRotatinge1 = true;
			omegaOne = -omegaSpeed;
			break;
	}
}

void handleArrowPress(int key, int x, int y) {
	//cout << "\nThis was pressed:" << key;
	int mod = glutGetModifiers();

	switch(mod)
	{
	case GLUT_ACTIVE_CTRL :
		cout << "Ctrl Held" << endl; break;
	case GLUT_ACTIVE_SHIFT :
		cout << "Shift Held" << endl; break;
	case GLUT_ACTIVE_ALT :
		cout << "Alt Held" << endl; break;
	}

	if(!mod)
	{
		//use arrow keys to pitch and everything
		switch(key)
		{
			case GLUT_KEY_LEFT:
				//want yaw left
				omegaThree = omegaSpeed;
				isRotatinge3 = true;
				break;
			case GLUT_KEY_RIGHT:
				omegaThree = -omegaSpeed;
				isRotatinge3 = true;
				break;
			case GLUT_KEY_UP:
				//pitch
				//this is where you would set inverted or not :D
				omegaTwo = omegaSpeed;
				isRotatinge2 = true;
				break;
			case GLUT_KEY_DOWN:
				omegaTwo = -omegaSpeed;
				isRotatinge2 = true;
				break;
		}
	}
	else
	{
		switch(key)
		{
			case GLUT_KEY_LEFT:
				break;
			case GLUT_KEY_RIGHT:
				break;
			case GLUT_KEY_UP:
				break;
			case GLUT_KEY_DOWN:
				break;
		}
	}

}


void handleSmoothKeys()
{
	//rotation
	if(isRotatinge1 || isRotatinge2 || isRotatinge3)
	{
		calculateLittleEs();
		calculateEulerAngleDots();

		psi += psiDot * rotationTimestep;
		if(psi > 2 * PI)
		{
			psi -= 2*PI;
		}
		else if (psi < -2 * PI)
		{
			psi += 2*PI;
		}
		theta += thetaDot * rotationTimestep;
		phi += phiDot * rotationTimestep;
	}

	//translation
	if(isTranslatinge1 || isTranslatinge2 || isTranslatinge3)
	{
		calculateLittleEs();
		if(isTranslatinge1) { translationVector += e1speed * e1;}
		if(isTranslatinge2) { translationVector += e2speed * e2;}
		if(isTranslatinge3) { translationVector += e3speed * e3;}
	}


}

void deleteStuff(void)
{
	allParticles.clear();
}

