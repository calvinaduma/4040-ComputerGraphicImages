#include <iostream>
#include <OpenImageIO/imageio.h>
#include <string>
#include <math.h>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING;


struct Pixel{
    unsigned char r, g, b, a;
};

Pixel** IN = NULL;
Pixel** ORIGINAL = NULL;
int pixel_format;
vector<Pixel> palette1, palette2, palette3, palette4, palette5, palette6, palette7, palette8, palette9;
int paletteHEIGHT, paletteWIDTH, paletteCHANNELS;

int CHANNELS;
int imWIDTH, imHEIGHT;
int winWIDTH, winHEIGHT;
int vpWIDTH, vpHEIGHT;
int Xoffset, Yoffset;
string output_filename = "";
string input_filename;

/*
    Reads an image from input_file.
*/
void readImage( string input_file );

/* 
    Writes stored image into output_file
*/
void writeImage( string output_file );

/*
    Routine to cleanup the memory.
*/
void destroy();

/*
	Handles Mouse click on the image.
    Left Click stops twirling.
*/
void handleMouseClick( int button, int state, int x, int y );

/*
* Displays currrent pixmap
*/
void handleDisplay();

/*
    Routine to display a pixmap in the current window
*/
void displayImage();

/*
    Reshape Callback Routine: If the window is too small to fit the image,
    make a viewport of the maximum size that maintains the image proportions.
    Otherwise, size the viewport to match the image size. In either case, the
    viewport is centered in the window.
*/
void handleReshape( int w, int h );

/*
    Keyboard Call Back Routine:
        q,Q: Quits program
        f,F: Reads new image from file
        w,W: Writes saved image to file
        r,R: Replays current image
        Default: do nothing
*/
void handleKey( unsigned char key, int x, int y );
// returns true if color is not in palette. false otherwise

void createNewImage();
void mapPalette();
void readColorPalette( string palette_file, vector<Pixel> palette );
void convertToOriginalImage();


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

    ORIGINAL = new Pixel*[imHEIGHT];
    ORIGINAL[0] = new Pixel[imHEIGHT*imWIDTH];
    for( int i=1; i<imHEIGHT; i++ ) ORIGINAL[i] = ORIGINAL[i-1] + imWIDTH;

    // reads in image pixel into data structure
    int idx;
    for( int y=0; y<imHEIGHT; ++y ){
        for( int x=0; x<imWIDTH; ++x ){
            idx = ( y * imWIDTH + x ) * CHANNELS;
            if( CHANNELS == 1 ){
                IN[y][x].r = temp_pixels[idx];
                IN[y][x].g = temp_pixels[idx];
                IN[y][x].b = temp_pixels[idx];
                IN[y][x].a = 255;
                ORIGINAL[y][x].r = temp_pixels[idx];
                ORIGINAL[y][x].g = temp_pixels[idx];
                ORIGINAL[y][x].b = temp_pixels[idx];
                ORIGINAL[y][x].a = 255;
            } else {
                IN[y][x].r = temp_pixels[idx];
                IN[y][x].g = temp_pixels[idx + 1];
                IN[y][x].b = temp_pixels[idx + 2];
                ORIGINAL[y][x].r = temp_pixels[idx];
                ORIGINAL[y][x].g = temp_pixels[idx + 1];
                ORIGINAL[y][x].b = temp_pixels[idx + 2];
				// no alpha value present
                if( CHANNELS < 4 ){
                    IN[y][x].a = 255;
                    ORIGINAL[y][x].a = 255;
                }
				// alpha value present
                else{ 
                    IN[y][x].a = temp_pixels[idx + 3];
                    ORIGINAL[y][x].a = temp_pixels[idx + 3];
                }
            }
        }
    }

    // close the image file after reading, and free up space for the oiio file handler
    infile->close();
    pixel_format = GL_RGBA;
    CHANNELS = 4;
    infile->close();
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

void convertToOriginalImage(){
    for( int i=0; i<imHEIGHT; i++)
        for( int j=0; j<imWIDTH; j++)
            IN[i][j] = ORIGINAL[i][j];
}

void destroy(){
    if ( IN ){
        delete IN[0];
        delete IN;
    }
}

void handleKey( unsigned char key, int x, int y ){
    switch( key ){
        case 'q': case 'Q':
            destroy();
            cout << "Exiting program!" << endl;
            exit( 0 );
        case 'f': case 'F':
            destroy();
            cout << "Enter an input filename: ";
            cin >> input_filename;
            readImage( input_filename );
            handleReshape(imWIDTH,imHEIGHT);
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
            // redo the image
            convertToOriginalImage();
            glutPostRedisplay();
            break;
        default:
            return;
    }
}

void handleMouseClick(int button, int state, int x, int y){
	switch( button ){
		case GLUT_LEFT_BUTTON:
			if( state == GLUT_UP ){
				// left mouse click
                // stops twirl
                createNewImage();
                glutPostRedisplay();
				break;
			}
            break;
		default:
			return;
	}
}

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

void readColorPalette( string palette_file, vector<Pixel> palette ){
    auto infile = ImageInput::open( palette_file );
    if ( !infile ){
        cerr << "Failed to open input file: " << input_filename << ". Exiting... " << endl;
        exit( 1 );
    }
    // Record image width, height and number of channels in global variables
    paletteWIDTH = infile->spec().width;
    paletteHEIGHT = infile->spec().height;
    paletteCHANNELS = infile->spec().nchannels;

    // allocate temporary structure to read the image
    unsigned char temp_pixels[ paletteWIDTH * paletteHEIGHT * paletteCHANNELS ];
    // read the image into the tmp_pixels from the input file, flipping it upside down using negative y-stride,
    // since OpenGL pixmaps have the bottom scanline first, and
    // oiio expects the top scanline first in the image file.
    int scanline_size = paletteWIDTH * paletteCHANNELS * sizeof( unsigned char );
    if( !infile->read_image( TypeDesc::UINT8, &temp_pixels[0] + (paletteHEIGHT - 1) * scanline_size, AutoStride, -scanline_size)){
        cerr << "Failed to read file " << input_filename << ". Exiting... " << endl;
        exit( 0 );
    }

    // allocates the space necessary for data struct
    // 16 colors on palette
    int idx = 0;
    Pixel color;
    for( int y=0; y<paletteHEIGHT*paletteWIDTH; y++){
        color.r = temp_pixels[idx++];
        color.g = temp_pixels[idx++];
        color.b = temp_pixels[idx++];
        if( paletteCHANNELS < 4 ) color.a = 255;
        else color.a = temp_pixels[idx++];
        // adds to color palette only if not in palette already
        if( palette.size()==0 ){
            palette.push_back( color );
            return;
        }
        for( int i=0; i<palette.size(); i++){
            if( palette[i].r == color.r && palette[i].g == color.g && palette[i].b == color.b) return;
            if( i==palette.size()-1 ) palette.push_back( color );
        }
    }
    infile->close();
}

// calculatd using distance formula
void mapPalette(){
    Pixel best_pixel;
    int least_distance, current_distance;
    int palette_num = 1;
    int x_offset = 0;
    int y_offset = 0;
    int x_bounds = imWIDTH/3;
    int y_bounds = imHEIGHT/3;
    Pixel current_color;
    while (palette_num < 10){
        switch (palette_num){
            case 1:
                for( int y=y_offset; y<x_bounds+y_offset; y++ )
                    for( int x=x_offset; x<y_bounds+x_offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette1 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                palette_num++;
                break;
            case 2:
                for( int y=y_offset; y<x_bounds+y_offset; y++ )
                    for( int x=x_offset; x<y_bounds+x_offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette2 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                palette_num++;
                break;
            case 3:
                for( int y=y_offset; y<x_bounds+y_offset; y++ )
                    for( int x=x_offset; x<y_bounds+x_offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette3 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                palette_num++;
                break;
            case 4:
                for( int y=y_offset; y<x_bounds+y_offset; y++ )
                    for( int x=x_offset; x<y_bounds+x_offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette4 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                break;
            case 5:
                for( int y=y_offset; y<x_bounds+y_offset; y++ )
                    for( int x=x_offset; x<y_bounds+x_offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette5 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                palette_num++;
                break;
            case 6:
                for( int y=y_offset; y<x_bounds+y_offset; y++ )
                    for( int x=x_offset; x<y_bounds+x_offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette6 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                palette_num++;
                break;
            case 7:
                for( int y=y_offset; y<x_bounds+offset; y++ )
                    for( int x=x_offset; x<y_bounds+offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette7 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                palette_num++;
                break;
            case 8:
                for( int y=y_offset; y<x_bounds+y_offset; y++ )
                    for( int x=x_offset; x<y_bounds+x_offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette8 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                palette_num++;
                break;
            case 9:
                for( int y=y_offset; y<x_bounds+y_offset; y++ )
                    for( int x=x_offset; x<y_bounds+x_offset; x++ ){
                        least_distance = 256;
                        current_color = { IN[y][x].r, IN[y][x].g, IN[y][x].b, IN[y][x].a };
                        for( auto &iter : palette9 ){
                            // gets smallest distance between color in palette vs color in image for mapping
                            current_distance = sqrt( pow((float)current_color.r - (float)iter.r,2) + pow((float)current_color.g - (float)iter.g,2) + pow((float)current_color.b - (float)iter.b,2) + pow((float)current_color.a - (float)iter.a,2) );
                            if( current_distance < least_distance ){
                                // sets best color
                                best_pixel = iter;
                                least_distance = current_distance;
                            }
                        }
                        IN[y][x] = best_pixel;
                    }
                x_offset += x_bounds;
                y_offset += y_bounds;
                palette_num++;
                break;
        }
    }
}

void createNewImage(){
    readColorPalette( "colorPalette1.png", palette1 );
    readColorPalette( "colorPalette2.png", palette2 );
    readColorPalette( "colorPalette3.png", palette3 );
    readColorPalette( "colorPalette4.png", palette4 );
    readColorPalette( "colorPalette5.png", palette5 );
    readColorPalette( "colorPalette6.png", palette6 );
    readColorPalette( "colorPalette7.png", palette7 );
    readColorPalette( "colorPalette8.png", palette8 );
    readColorPalette( "colorPalette9.png", palette9 );
    mapPalette();
}