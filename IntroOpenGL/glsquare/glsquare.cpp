/*
   Sample OpenGL/GLUT Program to Draw a Square
 
   Program opens a window and draws a colored square on a white background
   Colors cycle through R, Y, G, C, B, M with 'c' keypress
   Program quits when either 'q' or ESC are pressed

   CPSC 4040/6040            Ioannis Karamouzas         

*/

#include <cstdlib>

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

#define WIDTH	    600	/* window dimensions */
#define HEIGHT		600

static int icolor = 0;



void handleReshape(int w, int h) {
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// define the drawing coordinate system on the viewport
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);// sets up a 2D orthographic viewing region
}

/*
   Display Callback Routine: clear the screen and draw a square
   This routine is called every time the window on the screen needs
   to be redrawn, like if the window is iconized and then reopened
   by the user, and when the window is first created. It is also
   called whenever the program calls glutPostRedisplay()
*/
void drawSquare(){
  // red, yellow, green, cyan, blue, magenta
  float colors[6][3] = {{1, 0, 0}, {1, 1, 0}, {0, 1, 0},
			{0, 1, 1}, {0, 0, 1}, {1, 0, 1}};

  // specify window clear (background) color to be opaque white
  glClearColor(1, 1, 1, 1);

  // clear window to background color
  glClear(GL_COLOR_BUFFER_BIT);  

  // first set the drawing color
  glColor3f(colors[icolor][0], colors[icolor][1], colors[icolor][2]);
  
  // draw the square
  glBegin(GL_POLYGON);
    glVertex2i(100, 100);
    glVertex2i(100, 500);
    glVertex2i(500, 500);
    glVertex2i(500, 100);
  glEnd();

  // flush the OpenGL pipeline to the viewport
  glFlush();
}

/*
  Keyboard Callback Routine: 'c' cycle through colors, 'q' or ESC quit
  This routine is called every time a key is pressed on the keyboard
*/
void handleKey(unsigned char key, int x, int y){
  
  switch(key){
    case 'c':		// 'c' - cycle to next color
    case 'C':
      icolor = (icolor + 1) % 6;
      glutPostRedisplay(); // forces the current window to be redisplayed
      break;
      
    case 'q':		// q - quit
    case 'Q':
    case 27:		// esc - quit
      exit(0);
      
    default:		// not a valid key -- just ignore it
      return;
  }
}

/*
   Main program to draw the square, change colors, and wait for quit
*/
int main(int argc, char* argv[]){

  // start up the glut utilities
  glutInit(&argc, argv);

  // create the graphics window, giving width, height, and title text
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutCreateWindow("A Simple Square");

  // set up the callback routines to be called when glutMainLoop() detects
  // an event
  glutDisplayFunc(drawSquare);	  // display callback
  glutKeyboardFunc(handleKey);	  // keyboard callback
  glutReshapeFunc(handleReshape); // window resize callback

 
  // Routine that loops forever looking for events. It calls the registered 
  // callback routine to handle each event that is detected
  glutMainLoop();
  return 0;
}
