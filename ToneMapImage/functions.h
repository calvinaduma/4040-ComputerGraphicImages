#ifndef FUNCTIONS
#define FUNCTIONS

#include <iostream>
#include <OpenImageIO/imageio.h>
#include <string>
#include <math.h>
#include <fstream>
#include <algorithm>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING;

int imCHANNELS;
int imWIDTH, imHEIGHT;
int winWIDTH, winHEIGHT;
int vpWIDTH, vpHEIGHT;
int Xoffset, Yoffset;

struct Pixel{
    float r, g, b, a;
};
Pixel** original_pixmap = NULL;
Pixel** pixmap = NULL;
int pixel_format;
string output_filename = "";
string input_filename;
float gamma_value;
bool gamma_func = false;

void readImage( string );
void writeImage( string );
void displayImage();
void invertImage();
void destroy();
void handleKey( unsigned char, int, int );
void handleDisplay();
void handleReshape(int, int);
void baselineToneMapping();
void flipImageVertically();
void convertToOriginalPixmap();

void readImage( string infile_name ){
    // Create the oiio file handler for the image, and open the file for reading the image.
    // Once open, the file spec will indicate the width, height and number of channels.
    auto infile = ImageInput::open( infile_name );
    if ( !infile ){
        cerr << "Failed to open input file: " << input_filename << ". Exiting... " << endl;
        exit( 1 );
    }
    // Record image width, height and number of channels in global variables
    imWIDTH = infile->spec().width;
    imHEIGHT = infile->spec().height;
    imCHANNELS = infile->spec().nchannels;
    winWIDTH = imWIDTH;
    winHEIGHT = imHEIGHT;
    // allocate temporary structure to read the image
    float temp_pixels[ imWIDTH * imHEIGHT * imCHANNELS ];

    if( !infile->read_image( TypeDesc::FLOAT, &temp_pixels[0] )){
        cerr << "Failed to read file " << input_filename << ". Exiting... " << endl;
        exit( 0 );
    }
    // get rid of the old pixmap and make a new one of the new size
    // allocates the space necessary for data struct
    pixmap = new Pixel*[ imHEIGHT ];
    pixmap[0] = new Pixel[ imWIDTH * imHEIGHT ];
    for( int i=1; i<imHEIGHT; i++ ) pixmap[i] = pixmap[i-1] + imWIDTH;
    original_pixmap = new Pixel*[ imHEIGHT ];
    original_pixmap[0] = new Pixel[ imWIDTH * imHEIGHT ];
    for( int i=1; i<imHEIGHT; i++ ) original_pixmap[i] = original_pixmap[i-1] + imWIDTH;

    int idx;
    for( int y=0; y<imHEIGHT; ++y ){
        for( int x=0; x<imWIDTH; ++x ){
            idx = ( y * imWIDTH + x ) * imCHANNELS;
            if( imCHANNELS == 1 ){
                pixmap[y][x].r = temp_pixels[idx];
                pixmap[y][x].g = temp_pixels[idx];
                pixmap[y][x].b = temp_pixels[idx];
            } else {
                pixmap[y][x].r = temp_pixels[idx];
                pixmap[y][x].g = temp_pixels[idx + 1];
                pixmap[y][x].b = temp_pixels[idx + 2];
                if( imCHANNELS == 4 ) {
                    pixmap[y][x].a = temp_pixels[idx + 3];
                } else {
                    pixmap[y][x].a = 0.0;
                }
            }
        }
    }

    // flips image vertically
    flipImageVertically();

    // close the image file after reading, and free up space for the oiio file handler
    infile->close();
    pixel_format = GL_RGBA;
    imCHANNELS = 4;
}

void writeImage( string filename ){
    flipImageVertically();
    glDrawPixels(imWIDTH, imHEIGHT, pixel_format, GL_FLOAT, pixmap[0]);
    // make a pixmap that is the size of the window and grab OpenGL framebuffer into it
    // alternatively, you can read the pixmap into a 1d array and export this
    float temp_pixmap[ winWIDTH * winHEIGHT * imCHANNELS ];
    glReadPixels( 0, 0, winWIDTH, winHEIGHT, pixel_format, GL_FLOAT, temp_pixmap );

    // create the oiio file handler for the image
    auto outfile = ImageOutput::create( filename );
    if( !outfile ){
        cerr << "Failed to create output file: " << filename << ". Exiting... " << endl;
        exit( 1 );
    }

    // Open a file for writing the image. The file header will indicate an image of
    // width WinWidth, height WinHeight, and ImChannels channels per pixel.
    // All channels will be of type unsigned chars
    ImageSpec spec( winWIDTH, winHEIGHT, imCHANNELS, TypeDesc::FLOAT );
    if (!outfile->open( filename, spec )){
        cerr << "Failed to open output file: " << filename << ". Exiting... " << endl;
        exit( 1 );
    }

    // Write the image to the file. All channel values in the pixmap are taken to be
    // unsigned chars. While writing, flip the image upside down by using negative y stride,
    // since OpenGL pixmaps have the bottom scanline first, and oiio writes the top scanline first in the image file.
    int scanline_size = winWIDTH * imCHANNELS * sizeof( float );
    if( !outfile->write_image( TypeDesc::FLOAT, &temp_pixmap[0] )){
        cerr << "Failed to write to output file: " << filename << ". Exiting... " << endl;
        exit( 1 );
    }

    // close the image file after the image is written and free up space for the
    // ooio file handler
    outfile->close();
    flipImageVertically();
    glDrawPixels(imWIDTH, imHEIGHT, pixel_format, GL_FLOAT, pixmap[0]);
}

/*
    flips image vertically
*/
void flipImageVertically(){
    for( int r=0; r<imHEIGHT; r++ )
        for( int c=0; c<imWIDTH; c++ )
            original_pixmap[(imHEIGHT-1)-r][c] = pixmap[r][c];
    convertToOriginalPixmap();
}

/*
    Routine to invert the colors of the displayed pixmap
*/
void invertImage(){
    for( int r=0; r<imHEIGHT; r++ ){
        for( int c=0; c<imWIDTH; c++ ){
            pixmap[r][c].r = 255.0 - (float)pixmap[r][c].r;
            pixmap[r][c].g = 255.0 - (float)pixmap[r][c].g;
            pixmap[r][c].b = 255.0 - (float)pixmap[r][c].b;
        }
    }
}

/*
    Routine to cleanup the memory.
*/
void destroy(){
    if ( pixmap ){
        delete pixmap[0];
        delete pixmap;
    }
    if ( original_pixmap ){
        delete original_pixmap[0];
        delete original_pixmap;
    }
}

/*
    converts pixamp to the original pixmap
*/
void convertToOriginalPixmap(){
  for( int i=0; i<imHEIGHT; i++)
    for( int j=0; j<imWIDTH; j++){
      pixmap[i][j] = original_pixmap[i][j];
    }
}

/*
    luma function to calculate perceied luminance of an HDR image
*/
void lumaFunction( float** &world_luminance ){
    for( int y=0; y<imHEIGHT; y++ )
        for( int x=0; x<imWIDTH; x++ ){
            world_luminance[y][x] = (0.229 * pixmap[y][x].r) + (0.587 * pixmap[y][x].g) + (0.114 * pixmap[y][x].b);
        }
}

/*
    compresses the world luminance
*/
void compressWorldLuminance( float** &world_luminance, float** &displayed_luminance ){
    for( int y=0; y<imHEIGHT; y++ )
        for( int x=0; x<imWIDTH; x++ ){
            displayed_luminance[y][x] = world_luminance[y][x] / (world_luminance[y][x] + 1);
        }
}

/*
    compresses the world luminance
*/
void compressWorldLuminanceGamma( float** &world_luminance, float** &displayed_luminance ){
    for( int y=0; y<imHEIGHT; y++ )
        for( int x=0; x<imWIDTH; x++ ){
            displayed_luminance[y][x] = ( float )pow( world_luminance[y][x], gamma_value );
        }
}

/*
    scales RGB to each pixel relative to target display luminance and input world luminance
*/
void scaleRGBtoLuminance( float** world_luminance, float** displayed_luminance ){
    for( int y=0; y<imHEIGHT; y++ )
        for( int x=0; x<imWIDTH; x++ ){
            pixmap[y][x].r = ( displayed_luminance[y][x] / world_luminance[y][x] ) * pixmap[y][x].r / 2.2;
            pixmap[y][x].g = ( displayed_luminance[y][x] / world_luminance[y][x] ) * pixmap[y][x].g / 2.2;
            pixmap[y][x].b = ( displayed_luminance[y][x] / world_luminance[y][x] ) * pixmap[y][x].b / 2.2;
        }
}
/*
    baseline tone maps image
*/
void baselineToneMapping(){
    float** world_luminance = new float*[ imHEIGHT ];
    world_luminance[0] = new float[ imWIDTH*imHEIGHT ];
    for( int i=1; i<imHEIGHT; i++ ) world_luminance[i] = world_luminance[i-1] + imWIDTH;
    float** displayed_luminance = new float*[ imHEIGHT ];
    displayed_luminance[0] = new float[ imWIDTH*imHEIGHT ];
    for( int i=1; i<imHEIGHT; i++ ) displayed_luminance[i] = displayed_luminance[i-1] + imWIDTH;
    lumaFunction( world_luminance );
    if( gamma_func ){
        compressWorldLuminanceGamma( world_luminance, displayed_luminance );
    } else {
        compressWorldLuminance( world_luminance, displayed_luminance );
    }
    scaleRGBtoLuminance( world_luminance, displayed_luminance );
    delete world_luminance[0];
    delete world_luminance;
    delete displayed_luminance[0];
    delete displayed_luminance;
}
/*
    Keyboard Call Back Routine:
        q,Q: Quits program
        Default: do nothing
*/
void handleKey( unsigned char key, int x, int y ){
    switch( key ){
        case 'q': case 'Q':
            destroy();
            exit( 0 );
        case 'f': case 'F':
            destroy();
            cout << "Enter an input filename: ";
            cin >> input_filename;
            readImage( input_filename );
            glutReshapeWindow( imWIDTH, imHEIGHT );
            glutPostRedisplay();
            break;
        case 'w': case 'W':
            if( output_filename == ""){
                cout << "Enter an output filename: ";
                cin >> output_filename;
            }
            writeImage( output_filename );
            break;
        case 'r': case 'R':
            convertToOriginalPixmap();
            glutPostRedisplay();
            break;
        case 'i': case 'I':
            invertImage();
            glutPostRedisplay();
            break;
        case 't': case 'T':
            baselineToneMapping();
            glutPostRedisplay();
            break;
        case 'g': case 'G':
            gamma_func = true;
            cout << "Enter gamma value: ";
            cin >> gamma_value;
            baselineToneMapping();
            glutPostRedisplay();
            break;
        case 'b': case 'B':
            baselineToneMapping();
            glutPostRedisplay();
            break;
        default:
            return;
    }
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
        glDrawPixels(imWIDTH, imHEIGHT, pixel_format, GL_FLOAT, pixmap[0]);
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

#endif
