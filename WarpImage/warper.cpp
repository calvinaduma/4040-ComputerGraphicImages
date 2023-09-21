/*
 * Program sample to warp an image using matrix-based warps
 * 
 * Command line parameters are as follows:
 *
 * warper infile.png [outfile.png] 
 *
 * Author: Ioannis Karamouzas, 10/20/19, modifed on 10/16/21
 * modified by Calvin Aduma 11/15/21

 */

#include "matrix.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <OpenImageIO/imageio.h>
#include <fstream>
#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING;
using std::string;

struct Pixel{
    unsigned char r,g,b,a;
};

int CHANNELS = 4;
int imWIDTH, imHEIGHT;
int winWIDTH, winHEIGHT;
int vpWIDTH, vpHEIGHT;
int Xoffset, Yoffset;
int pixel_format;
Pixel** IN = NULL;
Pixel** OUT = NULL;
string output_filename = "";
string input_filename = "";
bool readInput = false;
Matrix3D M;

void readImage( string input_filename ){
    // Create the oiio file handler for the image, and open the file for reading the image.
    // Once open, the file spec will indicate the width, height and number of channels.
    auto infile = ImageInput::open( input_filename );
    if ( !infile ){
        cerr << "Failed to open input file: " << input_filename << ". Exiting... " << endl;
        exit( 1 );
    }
    // Record image width, height and number of channels in global variables
    imWIDTH = infile->spec().width;
    imHEIGHT = infile->spec().height;
    CHANNELS = infile->spec().nchannels;
	winWIDTH = imWIDTH;
	winHEIGHT = imHEIGHT;

    // allocate temporary structure to read the image
    unsigned char temp_pixels[ imWIDTH * imHEIGHT * CHANNELS ];
    // read the image into the tmp_pixels from the input file, flipping it upside down using negative y-stride,
    // since OpenGL pixmaps have the bottom scanline first, and
    // oiio expects the top scanline first in the image file.
    int scanline_size = imWIDTH * CHANNELS * sizeof( unsigned char );
    if( !infile->read_image( TypeDesc::UINT8, &temp_pixels[0] + (imHEIGHT - 1) * scanline_size, AutoStride, -scanline_size)){
        cerr << "Failed to read file " << input_filename << ". Exiting... " << endl;
        exit( 0 );
    }

    // allocates the space necessary for data struct
    IN = new Pixel*[imHEIGHT];
	IN[0] = new Pixel[imHEIGHT*imWIDTH];
	for( int i=1; i<imHEIGHT; i++ ) IN[i] = IN[i-1] + imWIDTH;


    int idx;
    for( int y=0; y<imHEIGHT; ++y ){
        for( int x=0; x<imWIDTH; ++x ){
            idx = ( y * imWIDTH + x ) * CHANNELS;
            if( CHANNELS == 1 ){
                IN[y][x].r = temp_pixels[idx];
                IN[y][x].g = temp_pixels[idx];
                IN[y][x].b = temp_pixels[idx];
                IN[y][x].a = 255;
            } else {
                IN[y][x].r = temp_pixels[idx];
                IN[y][x].g = temp_pixels[idx + 1];
                IN[y][x].b = temp_pixels[idx + 2];
				// no alpha value present
                if( CHANNELS < 4 ) IN[y][x].a = 255;
				// alpha value present
                else IN[y][x].a = temp_pixels[idx + 3];
            }
        }
    }

    // close the image file after reading, and free up space for the oiio file handler
    infile->close();
    pixel_format = GL_RGBA;
    CHANNELS = 4;
}

void writeImage( string filename ){
    // make a pixmap that is the size of the window and grab OpenGL framebuffer into it
    // alternatively, you can read the pixmap into a 1d array and export this
    unsigned char temp_pixmap[ winWIDTH * winHEIGHT * CHANNELS ];
    glReadPixels( 0, 0, winWIDTH, winHEIGHT, pixel_format, GL_UNSIGNED_BYTE, temp_pixmap );

    // create the oiio file handler for the image
    auto outfile = ImageOutput::create( filename );
    if( !outfile ){
        cerr << "Failed to create output file: " << filename << ". Exiting... " << endl;
        exit( 1 );
    }

    // Open a file for writing the image. The file header will indicate an image of
    // width WinWidth, height WinHeight, and ImChannels channels per pixel.
    // All channels will be of type unsigned char
    ImageSpec spec( winWIDTH, winHEIGHT, CHANNELS, TypeDesc::UINT8 );
    if (!outfile->open( filename, spec )){
        cerr << "Failed to open output file: " << filename << ". Exiting... " << endl;
        exit( 1 );
    }

    // Write the image to the file. All channel values in the pixmap are taken to be
    // unsigned chars. While writing, flip the image upside down by using negative y stride,
    // since OpenGL pixmaps have the bottom scanline first, and oiio writes the top scanline first in the image file.
    int scanline_size = winWIDTH * CHANNELS * sizeof( unsigned char );
    if( !outfile->write_image( TypeDesc::UINT8, temp_pixmap + (winHEIGHT - 1) * scanline_size, AutoStride, -scanline_size )){
        cerr << "Failed to write to output file: " << filename << ". Exiting... " << endl;
        exit( 1 );
    }

    // close the image file after the image is written and free up space for the
    // ooio file handler
    outfile->close();
}

/*
Multiply M by a rotation matrix of angle theta
*/
void Rotate(Matrix3D &M, float theta) {
	Matrix3D R;  // this initializes R to identity  
	double rad = PI * theta / 180.0; // convert degrees to radians

	// todo: populate the rotation matrix
	R[0][0] = cos(rad);
	R[0][1] = sin(rad);
	R[1][0] = -sin(rad);
	R[1][1] = cos(rad);
	
	M = R * M; //append the rotation to your transformation matrix
}
/*
Multiply M by a scale matrix
*/
void Scale(Matrix3D &M, float sx, float sy){
	Matrix3D S;
	S[0][0] = sx;
	S[1][1] = sy;
	M = S * M;
	cout << "calling scale" << endl;
}
/*
Multiply M by shear matrix
*/
void Shear(Matrix3D &M, float sx, float sy){
	Matrix3D S;
	S[0][1] = sx;
	S[1][0] = sy;
	M = S * M;
	cout << "calling shear" << endl;
}
/*
Multiply M by translation matrix 
*/
void Translate(Matrix3D &M, float tx, float ty){
	Matrix3D T;
	T[0][2] = tx;
	T[1][2] = ty;
	M = T * M;
	cout << "calling translate" << endl;
}
/*
Mulitply M by flip matrix
*/
void Flip(Matrix3D &M, int fx, int fy){
	Matrix3D F;
	if (fx == 1) F[0][0] = -1;
	if (fy == 1) F[1][1] = -1;
	M = F * M;
	cout << "calling flip" << endl;
}
/*
Mulitply M by Perspective Matrix
*/
void Perspective(Matrix3D &M, float px, float py){
	Matrix3D P;
	P[2][0] = px;
	P[2][1] = py;
	M = P * M;
	cout << "calling perspective" << endl;
}
/*
Build a transformation matrix from input text
*/
void read_input(Matrix3D &M) {
	string cmd;
	/*
		Affine transformations: A -> B = B*A
	*/
	/* prompt for user input */
	do
	{
		cout << "enter r, s, t, h, p, f, or d" << endl;
		cout << "> ";
		cin >> cmd;
		if (cmd.length() != 1){
			cout << "invalid command, enter r, s, t, h, f, p, d\n";
		}
		else {
			switch (cmd[0]) {
				case 'r':{		/* Rotation, accept angle in degrees */
					float theta;
					cout << "theta: ";
					cin >> theta;
					if (cin) {
						cout << "calling rotate\n" << endl;
						Rotate(M, theta);
					}
					else {
						cerr << "invalid rotation angle\n";
						cin.clear();
					}						
					break;
				}
				case 's':{		/* scale, accept scale factors */
					float sx, sy;
					cout << "x: ";
					cin >> sx;
					if (sx == 0 || sx == 0.0){
						cout << "invalid scale factor! Cannot be 0" << endl;
						cin.clear();
						break;
					}
					cout << "y: ";
					cin >> sy;
					cout << endl;
					if (sy == 0 || sy == 0.0){
						cout << "invalid scale factor! Cannot be 0" << endl;
						cin.clear();
						break;
					}
					Scale(M, sx, sy);		
					break;
				}
				case 't':{		/* Translation, accept translations */
					float tx, ty;
					cout << "x: ";
					cin >> tx;
					cout << "y: ";
					cin >> ty;
					cout << endl;
					Translate(M, tx, ty);
					break;
				}
				case 'h':{		/* Shear, accept shear factors */
					float sx, sy;
					cout << "x: ";
					cin >> sx;
					cout << "y: ";
					cin >> sy;
					cout << endl;
					Shear(M, sx, sy);
					break;
				}
				case 'f':{		/* Flip, accept flip factors */
					int fx, fy;
					cout << "x: ";
					cin >> fx;
					cout << "y: ";
					cin >> fy;
					cout << endl;
					Flip(M, fx, fy);
					break;
				}
				case 'p':{		/* Perspective, accept perspective factors */
					float px, py;
					cout << "x: ";
					cin >> px;
					if (px >= 1.0 || px <= -1.0){
						cout << "invalid perspective factor! Has to be in range -1.0 to 1.0 exclusively" << endl;
						cin.clear();
						break;
					}
					cout << "y: ";
					cin >> py;
					if (py >= 1.0 || py <= -1.0){
						cout << "invalid perspective factor! Has to be in range -1.0 to 1.0 exclusively" << endl;
						cin.clear();
						break;
					}
					cout << endl;
					Perspective(M, px, py);
					break;
				}	
				case 'd':		/* Done, that's all for now */
					break;
				default:
					cout << "invalid command, enter r, s, t, h, f, p, d\n";
			}
		}
	} while (cmd.compare("d")!=0);
}

/*
Normalize Vector3D for forward Map
*/
void NormalizeVector3D(Vector3D &v){
	if (v.z > 1){
		v.x /= v.z;
		v.y /= v.z;
		v.z /= v.z;
	}
}
/*
Forward Maps Image
*/
void ForwardMap(Matrix3D &M){
	// create bounding box for four corners
	Vector3D IN_top_left, IN_top_right, IN_bottom_left, IN_bottom_right;
	IN_bottom_left.x = IN_bottom_left.y = IN_bottom_right.y = IN_top_left.x = 0.0;
	IN_bottom_right.x = IN_top_right.x = double(imWIDTH);
	IN_top_left.y = IN_top_right.y = double(imHEIGHT);

	// multiple each corner by matrix M
	Vector3D MID_top_left, MID_top_right, MID_bottom_left, MID_bottom_right;
	MID_top_left = M * IN_top_left;
	MID_top_right = M * IN_top_right;
	MID_bottom_left = M * IN_bottom_left;
	MID_bottom_right = M * IN_bottom_right;

	// divide each corner by w' to normalize w
	NormalizeVector3D(MID_bottom_left);
	NormalizeVector3D(MID_bottom_right);
	NormalizeVector3D(MID_top_left);
	NormalizeVector3D(MID_top_right);

	// create new out corners
	Vector2D OUT_top_left(MID_top_left.x, MID_top_left.y);
	Vector2D OUT_top_right(MID_top_right.x, MID_top_right.y);
	Vector2D OUT_bottom_left(MID_bottom_left.x, MID_bottom_left.y);
	Vector2D OUT_bottom_right(MID_bottom_right.x, MID_bottom_right.y);
	
	// compute bounding box
	int right = max(OUT_top_left.x,max(OUT_bottom_left.x,max(OUT_top_right.x,OUT_bottom_right.x)));
	int left = min(OUT_top_left.x,min(OUT_bottom_left.x,min(OUT_top_right.x,OUT_bottom_right.x)));
	int top = max(OUT_top_left.y,max(OUT_bottom_left.y,max(OUT_top_right.y,OUT_bottom_right.y)));
	int bottom = min(OUT_top_left.y,min(OUT_bottom_left.y,min(OUT_top_right.y,OUT_bottom_right.y)));
	int imHEIGHTnew = top - bottom;
	int imWIDTHnew = right - left;
	// cout << "New HEIGHT: " << imHEIGHTnew << ", New WIDTH: " << imWIDTHnew << endl;
	winWIDTH = imWIDTHnew;
	winHEIGHT = imHEIGHTnew;
	// cout << "left: " << left << ", right: " << right << ", top: " << top << ", bottom: " << bottom << endl << endl;

	// allocate output image
	OUT = new Pixel*[imHEIGHTnew];
	OUT[0] = new Pixel[imHEIGHTnew*imWIDTHnew];
	for( int i=1; i<imHEIGHTnew; i++ ) OUT[i] = OUT[i-1] + imWIDTHnew;

	// origin
	Vector3D origin(left,bottom,0);
	// cout << "origin: (" << left << "," << bottom << "," << 0 << ")" << endl;
	
	// double _tr[3][3] = {{1,0,-min(OUT_bottom_left.x,min(OUT_bottom_right.x,min(OUT_top_left.x,OUT_top_right.x)))},
	// 					{0,1,-min(OUT_bottom_left.y,min(OUT_bottom_right.y,min(OUT_top_left.y,OUT_top_right.y)))},
	// 					{0,0,1}};

	// Matrix3D tr( _tr );
	// cout << "tr matrix: " << endl;
	// tr.print();
	// M = tr * M;
	// cout << "M * tr: " << endl;
	// M.print();

	// inverse matrix M
	Matrix3D invM = M.inverse();
	// cout << "invM: " << endl;
	//invM.print();

	// apply inverse map and interpolation
	for (int y=0; y<imHEIGHTnew; y++)
		for (int x=0; x<imWIDTHnew; x++){
			Vector3D pixel_out(x,y,1);
			pixel_out.x += origin.x;
			pixel_out.y += origin.y;
			pixel_out.z += origin.z;

			// inverse map 
			Vector3D pixel_in = invM * pixel_out;

			// coorindates of pixels from the input
			double normalizedU = pixel_in.x / pixel_in.z;
			double normalizedV = pixel_in.y / pixel_in.z;

			// round to get whole number
			int u = round(normalizedU);
			int v = round(normalizedV);

			// if u and/or v not within bounds of IN image, set to max or min respectively
			if ((u>=0 && u<imWIDTH) && (v>=0 && v<imHEIGHT)){
				// set OUT to IN
				OUT[y][x] = IN[v][u];
				// cout << "width: " << imWIDTH << ", height: " << imHEIGHT << endl;
				// cout << "new width: " << imWIDTHnew << ", new height: " << imHEIGHTnew << endl;
				// cout << "IN[" << v << "][" << u << "].r: " << IN[v][u].r << endl;
				// cout << "OUT[" << y << "][" << x << "].r: " << OUT[y][x].r << endl;
			} else {
				if (u > imWIDTH-1) u = imWIDTH-1;
				else if (u < 0) u = 0;
				if (v > imHEIGHT-1) v = imHEIGHT-1;
				else if (v < 0) v = 0;
				// set OUT to IN
				OUT[y][x] = IN[v][u];
			}
		}
	
	// place OUT data back into IN for display
	delete IN[0];
	delete IN;
	IN = new Pixel*[imHEIGHTnew];
	IN[0] = new Pixel[imWIDTHnew*imHEIGHTnew];
	for (int i=1; i<imHEIGHTnew; i++) IN[i] = IN[i-1] + imWIDTHnew;

	for (int y=0; y<imHEIGHTnew; y++)
		for (int x=0; x<imWIDTHnew; x++){
			IN[y][x] = OUT[y][x];
			// IN[y][x].r = OUT[y][x].r;
			// IN[y][x].g = OUT[y][x].g;
			// IN[y][x].b = OUT[y][x].b;
			// cout << "OUT[" << y << "][" << x << "].r: " << OUT[y][x].r << endl;
			// cout << "IN[" << y << "][" << x << "].r: " << IN[y][x].r << endl;
			
		}
	cout << "Affine Transformation Complete!" << endl;

	// delete OUT since no longer in use
	delete OUT[0];
	delete OUT;

	// assign width and height to new values for display purposes
	imWIDTH = imWIDTHnew;
	imHEIGHT = imHEIGHTnew;
}

/*
    Routine to display a pixmap in the current window
*/
void displayImage(){
    // if the window is smaller than the image, scale it down, otherwise do not scale
    if(winWIDTH < imWIDTH  || winHEIGHT < imHEIGHT)
        glPixelZoom(float(vpWIDTH) / imWIDTH, float(vpHEIGHT) / imHEIGHT);
    else
        glPixelZoom(1.0, 1.0);

        // display starting at the lower lefthand corner of the viewport
        glRasterPos2i(0, 0);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glDrawPixels(imWIDTH, imHEIGHT, pixel_format, GL_UNSIGNED_BYTE, IN[0]);
}

/*
* Displays currrent pixmap
*/
void handleDisplay(){
    // specify window clear (background) color to be opaque black
    glClearColor( 0, 0, 0, 1 );
    // clear window to background color
    glClear( GL_COLOR_BUFFER_BIT );

    // only draw the image if it is of a valid size
    if( imWIDTH > 0 && imHEIGHT > 0) displayImage();

    // flush the OpenGL pipeline to the viewport
    glFlush();
}

/*
    Reshape Callback Routine: If the window is too small to fit the image,
    make a viewport of the maximum size that maintains the image proportions.
    Otherwise, size the viewport to match the image size. In either case, the
    viewport is centered in the window.
*/
void handleReshape(int w, int h){
    float imageaspect = ( float )imWIDTH / (float )imHEIGHT;	// aspect ratio of image
    float newaspect = ( float  )w / ( float )h; // new aspect ratio of window

    // record the new window size in global variables for easy access
    winWIDTH = w;
    winHEIGHT = h;

    // if the image fits in the window, viewport is the same size as the image
    if( w >= imWIDTH && h >= imHEIGHT ){
    Xoffset = ( w - imWIDTH ) / 2;
    Yoffset = ( h - imHEIGHT ) / 2;
    vpWIDTH = imWIDTH;
    vpHEIGHT = imHEIGHT;
    }
    // if the window is wider than the image, use the full window height
    // and size the width to match the image aspect ratio
    else if( newaspect > imageaspect ){
    vpHEIGHT = h;
    vpWIDTH = int( imageaspect * vpHEIGHT );
    Xoffset = int(( w - vpWIDTH) / 2 );
    Yoffset = 0;
    }
    // if the window is narrower than the image, use the full window width
    // and size the height to match the image aspect ratio
    else{
    vpWIDTH = w;
    vpHEIGHT = int( vpWIDTH / imageaspect );
    Yoffset = int(( h - vpHEIGHT) / 2 );
    Xoffset = 0;
    }

    // center the viewport in the window
    glViewport( Xoffset, Yoffset, vpWIDTH, vpHEIGHT );

    // viewport coordinates are simply pixel coordinates
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0, vpWIDTH, 0, vpHEIGHT );
    glMatrixMode( GL_MODELVIEW );
}

/*
	Handles Mouse click on the image
*/
void handleMouseClick(int button, int state, int x, int y){
	switch( button ){
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_UP){
				if (readInput == false){
					readInput = true;
					// reads user input for affine transformations
					read_input(M);
					// prints value of matrix
					cout << "Accumulated Matrix: " << endl;
    				M.print();
					// completes the affine transformation with forward mapping
					ForwardMap(M);
				} else {
					cout << "Exiting program..." << endl;
					delete IN[0];
					delete IN;
					exit(0);
				}
				break;
			}
		default:
			return;
	}
}

/*
	Handles Keyboard click
	Any key press exits program
*/
void handleKeyboard( unsigned char key, int x, int y ){
	switch ( key ){
		default:
			delete IN[0];
			delete IN;
			exit( 0 );
	}
}
/*
   Main program to read an image file, then ask the user
   for transform information, transform the image and display
   it using the appropriate warp.  Optionally save the transformed
   images in  files.
*/
int main(int argc, char *argv[]){
	if( argc < 2 ){
		cout << "ERROR! Not enough arguments input to command line! Exiting..." << endl;
		exit(1);
	} else if( argc > 4 ){
		cout << "ERROR! Too many arguments input to command line! Exiting..." << endl;
		exit(1);
	} else if( argc == 3 ){
		output_filename = argv[2];
	}

	//your code to read in the input image
	input_filename = argv[1];
	readImage( input_filename );

	// reads user input for affine transformations
	read_input(M);
	// prints value of matrix
	cout << "Accumulated Matrix: " << endl;
	M.print();
	// completes the affine transformation with forward mapping
	ForwardMap(M);

	//your code to display the warped image
	glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA );
    glutInitWindowSize( winWIDTH, winHEIGHT );
    glutCreateWindow( "WARPED IMAGE" );

	glutDisplayFunc( handleDisplay );
	glutKeyboardFunc( handleKeyboard );
    glutReshapeFunc( handleReshape );	
	//glutMouseFunc( handleMouseClick );
	
	// write image ONLY if file is specified
	if( output_filename != "" ) writeImage( output_filename );

	glutMainLoop();
   	return 0;
}