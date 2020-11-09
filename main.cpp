// Original Code Base: Dr. Kwangtaek kim
// Comet Code Reference: 
// https://cs.lmu.edu/~ray/notes/openglexamples/
// Credit to Alexandri Zavodny for arcball code
// Modified By: MarcoDotIO
// Program to render out animation of a robot either walking in a straight or circular pattern

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <fstream>
#include <mmsystem.h>

// Body Definitions
#define BODY_WIDTH 2
#define BODY_HEIGHT 4
#define BODY_DEPTH 1
#define PI 3.141592

// Global Variables
char title[] = "POLISH ROBOT"; // Window border name

static size_t windowWidth = 800;
static size_t windowHeight = 600;
static float aspectRatio;

// Limb manipulation variables
static float rightShoulderAngle = 0.0, rightElbowAngle = 0.0, rightUpperLegAngle = 0.0, rightLowerLegAngle = 0.0;
static float leftShoulderAngle = 0.0, leftElbowAngle = 0.0, leftUpperLegAngle = 0.0, leftLowerLegAngle = 0.0;
static float robotPositionX = 0, robotPositionY = 0, robotPositionZ = 0;
static float robotRotationX = 0, robotRotationY = 0, robotRotationZ = 0;

// Walking animation variables
float angle = 0.0;
bool up = true; bool down = false;
GLfloat forwards = 2.0; GLfloat backwards = 1.0;
static float robotRotate = 270.0;

// Toggle variables
bool baxis = true;
bool path = false;
bool walking = false;
bool music = true;

static int u = 0;                 // curve parameter for comet pos

enum walkPattern {circular, straight, polishCow}; // Enumeration to determine pattern walked

walkPattern currentPattern = straight; // Variable for enumator

GLint leftMouseButton, rightMouseButton; //status of the mouse buttons
int mouseX = 0, mouseY = 0; //last known X and Y of the mouse
float cameraTheta, cameraPhi, cameraRadius; //camera position in spherical coordinates
float x, y, z; //camera position in cartesian coordinates

// solidBox(w, h, d) makes a box with width w, height h and
// depth d centered at the origin. It uses the GLUT solid cube function.
// The calls to glPushMatrix and glPopMatrix are essential here; they enable
// this function to be called from just about anywhere and guarantee that
// the glScalef call does not pollute code that follows a call to mySolidBox.
// (Note: Function based on original wireBox function) 
void solidBox(GLdouble width, GLdouble height, GLdouble depth) {
	glPushMatrix();
	glColor3f(1, 1, 1);
	glScalef(width, height, depth);
	glutSolidCube(1.0);
	glPopMatrix();
}

// Same as solidBox, but with a color option
void solidBoxColor(GLdouble width, GLdouble height, GLdouble depth, GLdouble red, GLdouble green, GLdouble blue) {
	glPushMatrix();
	glColor3f(red, green, blue);
	glScalef(width, height, depth);
	glutSolidCube(1.0);
	glPopMatrix();
}

// Plays music using Window's PlaySound() function
void playSomeMusic() {
	if (music)     PlaySound(TEXT("polishcow.wav"), NULL, SND_ASYNC);
	else    PlaySound(NULL, NULL, SND_ASYNC);
}

// Resets the position values of the robot, along with any limb manipulation
void resetPosition() {
	robotPositionZ = robotPositionY = robotPositionX = 0.0;
	angle = 0;
	up = true;
	down = false;
	forwards = 2.0;
	backwards = 1.0;
	robotRotate = 270.0;
	rightShoulderAngle = 0.0, rightElbowAngle = 0.0, rightUpperLegAngle = 0.0, rightLowerLegAngle = 0.0;
	leftShoulderAngle = 0.0, leftElbowAngle = 0.0, leftUpperLegAngle = 0.0, leftLowerLegAngle = 0.0;
	robotPositionX = 0, robotPositionY = 0, robotPositionZ = 0;
	robotRotationX = 0, robotRotationY = 0, robotRotationZ = 0;
	if (currentPattern == polishCow)    currentPattern = straight;
	music = false;
	playSomeMusic();
}

// solidSphere(w, h, d) makes a sphere with width w, height h and
// depth d centered at the origin. It uses the GLUT solid sphere function.
// The calls to glPushMatrix and glPopMatrix are essential here; they enable
// this function to be called from just about anywhere and guarantee that
// the glScalef call does not pollute code that follows a call to mySolidSphere.
// (Note: Function based on original wireSphere function) 
void solidSphere(GLdouble width, GLdouble height, GLdouble depth) {
	glPushMatrix();
	glColor3f(1, 1, 1);
	glScalef(width, height, depth);
	glutSolidSphere(1.0, 50.0, 50.0);
	glPopMatrix();
}

void drawAxes()
{
	// Draw a red x-axis, a green y-axis, and a blue z-axis.
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(5, 0, 0);
	glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 5, 0);
	glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 5);
	glEnd();
}

void drawScene()
{
	// Body
	glPushMatrix();

	// Draw the upper body at the orgin
	solidBox(BODY_WIDTH, BODY_HEIGHT, BODY_DEPTH);

	glPopMatrix();


	// Left Arm
	glPushMatrix();

	// Left Shoulder
		// Draw the upper arm, rotated shoulder degrees about the z-axis. Note that
		// the thing about glutWireBox is that normally its origin is in the middle
		// of the box, but we want the "origin" of our box to be at the left end of
		// the box, so it needs to first be shifted 1 unit in the x direction, then
		// rotated.
	glTranslatef(1.0, 1.5, 0.0); // (4) move to the right end of the upper body (attachment)
	glRotatef(-90, 0.0, 0.0, 1.0);
	glRotatef((GLfloat)leftShoulderAngle, 0.0, 1.0, 0.0); //(3) then rotate shoulder
	glTranslatef(1.0, 0.0, 0.0); // (2) shift to the right on the x axis to have the left end at the origin
	solidBox(2.0, 0.4, 1.0); // (1) draw the upper arm box

	// Left Elbow
		// Now we are ready to draw the lower arm. Since the lower arm is attached
		// to the upper arm we put the code here so that all rotations we do are
		// relative to the rotation that we already made above to orient the upper
		// arm. So, we want to rotate elbow degrees about the z-axis. But, like
		// before, the anchor point for the rotation is at the end of the box, so
		// we translate <1,0,0> before rotating. But after rotating we have to
		// position the lower arm at the end of the upper arm, so we have to
		// translate it <1,0,0> again.
	glTranslatef(1.0, 0.0, 0.0); // (4) move to the right end of the upper arm
	glRotatef((GLfloat)leftElbowAngle, 0.0, 0.0, 1.0); // (3) rotate
	glTranslatef(1.0, 0.0, 0.0); // (2) shift to the right on the x axis to have the left end at the origin
	solidBox(2.0, 0.4, 1.0); // (1) draw the lower arm .

	glPopMatrix();


	// Right Arm
	glPushMatrix();

	// Right Shoulder
	glTranslatef(-1.0, 1.5, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(-90, 0.0, 0.0, 1.0);
	glRotatef((GLfloat)rightShoulderAngle, 0.0, 1.0, 0.0);
	glTranslatef(1.0, 0.0, 0.0);
	solidBox(2.0, 0.4, 1.0);

	// Right Elbow
	glTranslatef(1.0, 0.0, 0.0);
	glRotatef((GLfloat)rightElbowAngle, 0.0, 0.0, 1.0);
	glTranslatef(1.0, 0.0, 0.0);
	solidBox(2.0, 0.4, 1.0);

	glPopMatrix();


	// Left Leg
	glPushMatrix();

	// Upper Left Leg
	glTranslatef(0.8, -2.0, 0.0);
	glRotatef((GLfloat)leftUpperLegAngle, 1.0, 0.0, 0.0);
	glTranslatef(0.0, -1.0, 0.0);
	solidBox(0.4, 2.0, 1.0);

	//Lower Left Leg
	glTranslatef(0.0, -1.0, 0.0);
	glRotatef((GLfloat)leftLowerLegAngle, 1.0, 0.0, 0.0);
	glTranslatef(0.0, -1.0, 0.0);
	solidBox(0.4, 2.0, 1.0);

	glPopMatrix();


	// Right Leg
	glPushMatrix();

	// Upper Right Leg
	glTranslatef(-0.8, -2.0, 0.0);
	glRotatef(180, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 0.0, 1.0);
	glRotatef((GLfloat)rightUpperLegAngle, 1.0, 0.0, 0.0);
	glTranslatef(0.0, -1.0, 0.0);
	solidBox(0.4, 2.0, 1.0);

	//Lower Right Leg
	glTranslatef(0.0, -1.0, 0.0);
	glRotatef((GLfloat)rightLowerLegAngle, 1.0, 0.0, 0.0);
	glTranslatef(0.0, -1.0, 0.0);
	solidBox(0.4, 2.0, 1.0);

	glPopMatrix();


	// Head
	glPushMatrix();

	glTranslatef(0.0, 3.0, 0.0);
	solidSphere(1.0, 1.0, 1.0);

	glPopMatrix();
}

// Displays the arm in its current position and orientation. The whole
// function is bracketed by glPushMatrix and glPopMatrix calls because every
// time we call it we are in an "environment" in which a gluLookAt is in
// effect. (Note that in particular, replacing glPushMatrix with
// glLoadIdentity makes you lose the camera setting from gluLookAt).
void display() {
	glMatrixMode(GL_MODELVIEW); //make sure we aren't changing the projection matrix!
	glLoadIdentity();
	gluLookAt(x, y, z, //camera is located at (x,y,z)
		0, 0, 0, //camera is looking at (0,0,0)
		0.0f, 1.0f, 0.0f); //up vector is (0,1,0) (positive Y)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw the ground (a plane)
	glPushMatrix();

	glTranslatef(0, -6.1, 0);
	solidBoxColor(1000.0, 0, 1000.0, 0.9, 0.7, 0.9);

	glPopMatrix();

	// Draw Path
	glPushMatrix();

	if (currentPattern == circular && path) {
		glRotatef(90, 1.0, 0.0, 0.0);
		glColor3f(0.3, 0.4, 0.5);
		glTranslatef(0, 0, 10.3);
		glutSolidTorus(5.625, 14.35, 16, 40);
	}
	else if (currentPattern == straight && path) {
		glTranslatef(0, -6.0, 0);
		solidBoxColor(10.0, 0, 1000.0, 0.7, 0.6, 0.5);
	}

	glPopMatrix();

	// Draw one Robot with manipulated position
	glPushMatrix();
	glRotatef(robotRotationX, 1, 0, 0);
	glRotatef(robotRotationY, 0, 1, 0);
	glRotatef(robotRotationZ, 0, 0, 1);
	glTranslatef(robotPositionX, robotPositionY, robotPositionZ);
	drawScene();
	glPopMatrix();

	if (baxis) drawAxes(); // draw axes
	glutSwapBuffers();
	glFlush();
}

// As usual we reset the projection transformation whenever the window is
// reshaped.  This is done (of course) by setting the current matrix mode
// to GL_PROJECTION and then setting the matrix.  It is easiest to use the
// perspective-projection-making matrix from the GL utiltiy library.  Here
// we set a perspective camera with a 60-degree vertical field of view,
// an aspect ratio to perfectly map into the system window, a near clipping
// plane distance of 1.0 and a far clipping distance of 40.0.  The last
// thing done is to reset the current matrix mode to GL_MODELVIEW, as
// that is expected in all the calls to display().
void reshape(GLint w, GLint h) {
	aspectRatio = w / (float)h;
	windowWidth = w;
	windowHeight = h; //update the viewport to fill the window
	glViewport(0, 0, w, h); //update the projection matrix with the new window properties
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, aspectRatio, 0.1, 100);
}

///////////////////////////////////////////////////////////////
// GLUT callback for mouse clicks. We save the state of the mouse button
// when this is called so that we can check the status of the mouse
// buttons inside the motion callback (whether they are up or down).
////////////////////////////////////////////////////////////////////////////////
void procMouse(int button, int state, int thisX, int thisY)
{
	//update the left and right mouse button states, if applicable
	if (button == GLUT_LEFT_BUTTON)
		leftMouseButton = state;
	else if (button == GLUT_RIGHT_BUTTON)
		rightMouseButton = state;
	//and update the last seen X and Y coordinates of the mouse
	mouseX = thisX;
	mouseY = thisY;
}


void procKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '1': // Wireframe Mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case '2': // Solid Mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case '3': baxis = !baxis; break; // Axis Switch
	case '4': path = !path; break; // Toggles the path
	case 'r': resetPosition(); break; // Restarts the position of the robot
	case 'a': walking = !walking; break; // Toggles on or off the walking
	case 'p': resetPosition();  walking = true; // Toggles which walking animation is used (circular or straight)
		if (currentPattern == straight)    currentPattern = circular;
		else if (currentPattern == circular)    currentPattern = straight;
		break;
	case 'c': resetPosition(); music = !music; playSomeMusic(); currentPattern = polishCow; break;
	case 27: exit(0); break; // Default Case
	}
	glutPostRedisplay();
}

// Function controls the animation of the limbs, with each side being opposite of each other;
// along with a slight up and down movement.
void moveRobot() {
	// If movement is going upwards
	if (up) {
		robotPositionY = robotPositionY + 0.002;
		if (rightShoulderAngle < 0)    rightShoulderAngle = (GLfloat)rightShoulderAngle + forwards;
		else    rightShoulderAngle = (GLfloat)rightShoulderAngle + backwards;
		if (leftShoulderAngle < 0)    leftShoulderAngle = (GLfloat)leftShoulderAngle + forwards;
		else    leftShoulderAngle = (GLfloat)leftShoulderAngle + backwards;

		if (rightUpperLegAngle < 0)    rightUpperLegAngle = (GLfloat)rightUpperLegAngle + forwards;
		else    rightUpperLegAngle = (GLfloat)rightUpperLegAngle + backwards;
		if (leftUpperLegAngle < 0)    leftUpperLegAngle = (GLfloat)leftUpperLegAngle + forwards;
		else    leftUpperLegAngle = (GLfloat)leftUpperLegAngle + backwards;

		if (rightLowerLegAngle < 0)    rightLowerLegAngle = (GLfloat)rightLowerLegAngle + forwards;
		else    rightLowerLegAngle = (GLfloat)rightLowerLegAngle + backwards;
		if (leftLowerLegAngle < 0)    leftLowerLegAngle = (GLfloat)leftLowerLegAngle + forwards;
		else    leftLowerLegAngle = (GLfloat)leftLowerLegAngle + backwards;

		if (robotPositionY > 0.1) {
			up = false;
			down = true;
		}
	}
	// If movement is going downwards
	if (down) {
		robotPositionY = robotPositionY - 0.002;
		if (rightShoulderAngle > 0)    rightShoulderAngle = (GLfloat)rightShoulderAngle - forwards;
		else    rightShoulderAngle = (GLfloat)rightShoulderAngle - backwards;
		if (leftShoulderAngle > 0)    leftShoulderAngle = (GLfloat)leftShoulderAngle - forwards;
		else    leftShoulderAngle = (GLfloat)leftShoulderAngle - backwards;

		if (rightUpperLegAngle > 0)    rightUpperLegAngle = (GLfloat)rightUpperLegAngle - forwards;
		else    rightUpperLegAngle = (GLfloat)rightUpperLegAngle - backwards;
		if (leftUpperLegAngle > 0)    leftUpperLegAngle = (GLfloat)leftUpperLegAngle - forwards;
		else    leftUpperLegAngle = (GLfloat)leftUpperLegAngle - backwards;

		if (leftLowerLegAngle > 0)    leftLowerLegAngle = (GLfloat)leftLowerLegAngle - backwards;
		else    leftLowerLegAngle = (GLfloat)leftLowerLegAngle - forwards;
		if (rightLowerLegAngle > 0)    rightLowerLegAngle = (GLfloat)rightLowerLegAngle - forwards;
		else    rightLowerLegAngle = (GLfloat)rightLowerLegAngle - backwards;

		if (robotPositionY < 0.002) {
			up = true;
			down = false;
		}
		angle = angle + 0.000001f;
	}
}

void danceRobot() {
	if (u < 60) robotRotationY = 45.0;
	// First Raise
	if (u > 60 && u < 80) {
		leftUpperLegAngle = -22.5;
		leftShoulderAngle = -22.5;
	}
	if (u > 101 && u < 120) {
		leftUpperLegAngle = -45;
		leftShoulderAngle = -45;
	}
	if (u > 121 && u < 140) {
		leftUpperLegAngle = -55;
		leftShoulderAngle = -55;
	}
	if (u > 161 && u < 180) {
		leftUpperLegAngle = -45;
		leftShoulderAngle = -45;
	}
	if (u > 181 && u < 200) {
		leftUpperLegAngle = -22.5;
		leftShoulderAngle = -22.5;
	}
	if (u > 201 && u < 220) {
		leftUpperLegAngle = 0.0;
		leftShoulderAngle = 0.0;
	}

	// Second Raise
	if (u > 221 && u < 240) {
		leftUpperLegAngle = -22.5;
		leftShoulderAngle = -22.5;
	}
	if (u > 241 && u < 260) {
		leftUpperLegAngle = -45;
		leftShoulderAngle = -45;
	}
	if (u > 261 && u < 280) {
		leftUpperLegAngle = -55;
		leftShoulderAngle = -55;
	}
	if (u > 281 && u < 300) {
		leftUpperLegAngle = -45;
		leftShoulderAngle = -45;
	}
	if (u > 301 && u < 320) {
		leftUpperLegAngle = -22.5;
		leftShoulderAngle = -22.5;
	}
	if (u > 321 && u < 340) {
		leftUpperLegAngle = 0.0;
		leftShoulderAngle = 0.0;
	}

	// First Jump
	if (u > 341 && u < 360) {
		robotRotationY = -45.0;
		robotPositionY = 0.5;
	}
	if (u > 361 && u < 380) {
		robotRotationY = -90.0;
		robotPositionY = 1.5;
	}
	if (u > 381 && u < 400) {
		robotRotationY = -135.0;
		robotPositionY = 0.5;
	}
	if (u > 401 && u < 420) {
		robotRotationY = -145.0;
		robotPositionY = 0.0;
	}

	// Third Raise
	if (u > 421 && u < 440) {
		rightShoulderAngle = 22.5;
		rightUpperLegAngle = 22.5;
	}
	if (u > 441 && u < 460) {
		rightShoulderAngle = 45;
		rightUpperLegAngle = 45;
	}
	if (u > 461 && u < 480) {
		rightShoulderAngle = 55;
		rightUpperLegAngle = 55;
	}
	if (u > 481 && u < 500) {
		rightShoulderAngle = 45;
		rightUpperLegAngle = 45;
	}
	if (u > 501 && u < 520) {
		rightShoulderAngle = 22.5;
		rightUpperLegAngle = 22.5;
	}
	if (u > 521 && u < 540) {
		rightShoulderAngle = 0.0;
		rightUpperLegAngle = 0.0;
	}

	// Fourth Raise
	if (u > 541 && u < 560) {
		rightShoulderAngle = 22.5;
		rightUpperLegAngle = 22.5;
	}
	if (u > 561 && u < 580) {
		rightShoulderAngle = 45;
		rightUpperLegAngle = 45;
	}
	if (u > 581 && u < 600) {
		rightShoulderAngle = 55;
		rightUpperLegAngle = 55;
	}
	if (u > 601 && u < 620) {
		rightShoulderAngle = 45;
		rightUpperLegAngle = 45;
	}
	if (u > 621 && u < 640) {
		rightShoulderAngle = 22.5;
		rightUpperLegAngle = 22.5;
	}
	if (u > 641 && u < 660) {
		rightShoulderAngle = 0.0;
		rightUpperLegAngle = 0.0;
	}

	// Second Jump
	if (u > 661 && u < 680) {
		robotRotationY = -135.0;
		robotPositionY = 0.5;
	}
	if (u > 681 && u < 700) {
		robotRotationY = -90.0;
		robotPositionY = 1.5;
	}
	if (u > 701 && u < 720) {
		robotRotationY = -45.0;
		robotPositionY = 0.5;
	}
	if (u > 721 && u < 740) {
		robotRotationY = 0.0;
		robotPositionY = 0.0;
	}

	if (u >= 740)    u = 0;

}

// Timer function that allows the animation of either a circular or straight walk to be enabled.
void timer(int v) {
	u = u + 1;
	if (currentPattern == circular && walking) {
		robotPositionZ = sin(angle) * 15;
		robotPositionX = -cos(angle) * 15;
		moveRobot();
		robotRotationY = robotRotate;
		robotRotate = fmod((robotRotate + 1.0), 360);
	}
	else if (currentPattern == straight && walking) {
		robotPositionZ = robotPositionZ + 0.075;
		moveRobot();
	}
	else if (currentPattern == polishCow && !walking)     danceRobot();
	glLoadIdentity();
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, timer, v);
}

// Initialize program, setting depth and other toggles for OpenGL
void init() {
	glShadeModel(GL_FLAT);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glLoadIdentity();
}

//////////////////////////////////////////////////////
// This function updates the camera's position in cartesian coordinates based
// on its position in spherical coordinates. Should be called every time
// cameraTheta, cameraPhi, or cameraRadius is updated.
//////////////////////////////////////////////////////////////////////////////
void recomputeOrientation()
{
	x = cameraRadius * sinf(cameraTheta) * sinf(cameraPhi);
	z = cameraRadius * cosf(cameraTheta) * sinf(cameraPhi);
	y = cameraRadius * cosf(cameraPhi);
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////
// GLUT callback for mouse movement. We update
// cameraPhi, cameraTheta, and /or cameraRadius based
// on how much the user has moved the mouse in the X
// or Y directions(in screen space) and whether they have
// held down the left or right mouse buttons.If the user
// hasn't held down any buttons, the function just updates
// the last seen mouse X and Y coords.
///////////////////////////////////////////////////////////////
void mouseMotion(int x, int y)
{
	if (leftMouseButton == GLUT_DOWN)
	{
		cameraTheta += (mouseX - x) * 0.005;
		cameraPhi += (mouseY - y) * 0.005;
		// make sure that phi stays within the range (0, PI)
		if (cameraPhi <= 0)
			cameraPhi = 0 + 0.001;
		if (cameraPhi >= PI)
			cameraPhi = PI - 0.001;
		recomputeOrientation(); //update camera (x,y,z)
	}
	// camera zoom in/out
	else if (rightMouseButton == GLUT_DOWN) {
		double totalChangeSq = (x - mouseX) + (y - mouseY);
		cameraRadius += totalChangeSq * 0.01;
		//limit the camera radius to some reasonable values so the user can't get lost
		if (cameraRadius < 2.0)
			cameraRadius = 2.0;
		if (cameraRadius > 50.0)
			cameraRadius = 50.0;
		recomputeOrientation();     //update camera (x,y,z) based on (radius,theta,phi)
	}
	mouseX = x;
	mouseY = y;
}

// Initializes GLUT, the display mode, and main window; registers callbacks;
// does application initialization; enters the main event loop.
int main(int argc, char** argv) {
	printf("\n\
-----------------------------------------------------------------------\n\
 OpenGL Sample Program for a robot:\n\
 - '1': display a wireframe (mesh only) model \n\
 - '2': display a solid model \n\
 - '3': toggle on off to draw axes \n\
 - '4': increment the shoulderAngle \n\
 - 'r': move the robot to the initial position to be animated \n\
 - 'a': animation walking toggle ON/OFF (animation only) \n\
 - 'p': walking path options of the robot (circular or straight) \n\
 - Left Click + Drag: camera rotation \n\
 - Right Click + Drag: zoom in and out \n\
 - 'ESC': terminate the program \n\
-----------------------------------------------------------------------\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow(title);
	cameraRadius = 7.0f;
	cameraTheta = 2.80;
	cameraPhi = 2.0;
	glutTimerFunc(100, timer, 0);
	recomputeOrientation();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(procKeys);
	glutMouseFunc(procMouse);
	glutMotionFunc(mouseMotion);
	init();
	glutMainLoop();
	return(0);
}