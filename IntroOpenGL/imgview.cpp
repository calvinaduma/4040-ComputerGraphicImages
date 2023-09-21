/*
 * Calvin Aduma
 *
 * CPSC 4040	Ioannis Karamouzas
 *
 * This program reads in an image as an input and
 * allows the user to press keys: 
 * W: write pixmap to a file
 * R: read from a file and display image
 * Q: exit program
 * I: Invert color of pixmap and display image
 * N: create noisy version of pixmap and display image
 * 
 */

#include <OpenImageIO/imageio.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <time.h> 

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;
using namespace OIIO;

int WIDTH  = 600;
int HEIGHT = 600;
int CHANNELS = 4;

typedef struct RGBA_pixel{
	unsigned char red;
	unsigned char blue;
	unsigned char green;
	unsigned char alpha;
} RGBA_pixel;

//RGBA_pixel ** pixmap;
vector<unsigned char> pixmap(HEIGHT*WIDTH*CHANNELS);

/*
* Displays currrent pixmap
*/
void display(){
	glClear(GL_COLOR_BUFFER_BIT);
	// convert to array before passing into glDrawPixels
	unsigned char* pixmap_array = &pixmap[0];
	glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixmap_array );
	glFlush();
}

/*
* Writes current frame buffer to an image file
*/
void writeToFile( string filename ){
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	unsigned char pixmap_new[4 * w * h];

	auto out = ImageOutput::create( filename );
	if( !out ){
		cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
		return;
	}

	// get the current pixels from the OpenGL framebuffer and store in pixmap
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixmap_new);

	ImageSpec spec( w, h, CHANNELS, TypeDesc::UINT8 );
	if(!out->open(filename, spec)){
    cerr << "Could not open " << outfilename << ", error = " << geterror() << endl;
    return;
  	}

	// write the image to the file. All channel values in the pixmap are taken to be
	// unsigned chars
	if(!out->write_image(TypeDesc::UINT8, pixmap_new)){
		cerr << "Could not write image to " << outfilename << ", error = " << geterror() << endl;
		return;
	} else
		cout << "Square is stored" << endl;

	// close the image file after the image is written
	if(!outfile->close()){
    cerr << "Could not close " << outfilename << ", error = " << geterror() << endl;
    return;
  	}
}

/*
* Reads from file and displays image in window
*/
void readFromFile( string filename ){
	auto inp = ImageInput::open( filename );
	if ( ! inp )
		return;
	// contains information about image
	const ImageSpec &spec = inp->spec();
	WIDTH = spec.width;
	HEIGHT = spec.height;
	pixmap.resize(HEIGHT*WIDTH*CHANNELS);
	// reshape window to match current image specs
	glutReshapeWindow( WIDTH, HEIGHT );
	inp->read_image( TypeDesc::UINT8, &pixmap[0] );
	inp->close();
}

/*
* Keyboard Callback Routine:
* w: writes pixmap to file
* r: reads from file
* q: exits program
* i: inverts color of pixmap
* n: creates noisy version of image
*/
void handleKey(unsigned char key, int x, int y){
	string userInput;
	switch(key){
		/* prompt user for an output image file name
		write this image file from the pixmap */
		case 'w':
		case 'W':
			cout << "Enter name of file to write to: ";
			cin >> userInput;
			cout << endl;
			writeToFile( userInput );
			break;
		/* prompt user for input image file name
		read image file, stored in appropriately sized pixmap,
		update display to show image from pixmap */
		case 'r':
		case 'R':
			cout << "Enter name of file to read from: ";
			cin >> userInput;
			cout << endl;
			readFromFile( userInput );
			display();
			break;
		/* exit program */
		case 'q':
		case 'Q':
			exit(0);
		/* invert color of image, update display image */
		case 'i':
		case 'I':
			int size = pixmap.size()
			for (int i=0; i<size; i++)
				pixmap[i] = 255 - pixmap[i];
			display()
			break;
		/* creates noisy version of image.
		randomly set 20% of image pixels to black */
		case 'n':
		case 'N':
			break;
		default:
			return;
	}
}

/*
* Reshape Callback Routine: sets up the viewport and drawing coordinates
* This routine is called when the window is created and every time the window
* is resized by the program or computer
*/
void handleReshape(int w, int h){
	// set the viewport to be the entire window
	glViewport(0,0,w,h);
	
	// define the drawing coordinate system on the viewport
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0,w,0,h );
}

int main(int argc, char* argv[]){
	// startup the glut utilities
	glutInit( &argc, argv );

	// create the graphics window- giving width, height, and title text
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );
	glutInitWindowSize( WIDTH, HEIGHT );
	glutCreateWindow( "IMGVIEW" );

    if ( argc == 1 ){
        /* standard selection: displays empty window */
        // sets background color to black
        glClearColor( 0,0,0,0 );
        glClear( GL_COLOR_BUFFER_BIT );  // clear window to background color
        glFlush();
    } else {
        /* image selection */
        /* read image from file and display image */
		readFromFile(argv[1]);
    }

	// sets up callback routines to call when glutMainLoop() detects an event
	glutDisplayFunc( display );
	glutKeyboardFunc( handleKey );
	glutReshapeFunc( handleReshape );

	// routine that loops forever looking for events
	// calls the registered callback routine(s) to handle events
	glutMainLoop();

	return 0;
}

//
//  Routine to cleanup the memory.   
//
void destroy(){
 if (pixmap){
     delete pixmap[0];
	 delete pixmap;  
  }
}
