/*
    Calvin Aduma

    CPSC 4040 Ioannis Karamouzas

    Basic Functions for Project3

*/
#include <iostream>
#include <OpenImageIO/imageio.h>
#include <string>
#include <math.h>
#include <fstream>
#include "RGBtoHSV.cpp"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING;

class Pixmap{
    public:
        int width;
        int height;
        int channels;
        unsigned char* pixels;
        string ext;

    /*
        Default Constructor
        Initializes all values to 0 or NULL
    */
    Pixmap(){
        width = 0;
        height = 0;
        channels = 0;
        pixels = NULL;
        ext = "";
    }
    /*
        Input: filename[str]
        Output: Pixmap
        Creates pixmap object of specs of image from input
    */

    Pixmap( string filename ){
        auto input = ImageInput::open( filename );
        if ( !input ){
            cout << "Failed to open file: " << filename << ". Exiting... " << endl;
            exit( 1 );
        }
        const ImageSpec &spec = input->spec();
        width = spec.width;
        height = spec.width;
        channels = spec.nchannels;
        ext = filename.substr( filename.find("."), filename.length() );
        pixels = new unsigned char[width*height*channels];
        input->read_image( TypeDesc::UINT8, &pixels[0]);
        input->close();
    }

    /*
        Writes image to a file
        Input: filename[str]
        Output: void
    */
    void writeImage( string filename ){
        const int x = this->width;
        const int y = this->height;
        const int channels = this->channels;
        auto output = ImageOutput::create( filename );
        if ( !output ){
            cout << "Failed to open file: " << filename << ". Exiting... " << endl;
            exit( 1 );
        }
        ImageSpec spec( x, y, channels, TypeDesc::UINT8 );
        output->open( filename, spec );
        output->write_image( TypeDesc::UINT8, pixels );
        output->close();
    }

    /*
        Produces 4 channel version of pixels from image.
        "RGB" channels are unchanged. "A" channel added.
        Input: None
        Output: None
    */
    void convertToFourChannels(){
        if( this->channels == 4) return;

        double h, s, v;
        unsigned char* temp_pixels = new unsigned char[width*height*4];

        int j=0;
        for( int i=0; i<( this->width*this->height*this->channels )-this->channels; i+=this->channels ){
            RGBtoHSV( this->pixels[i], this->pixels[i+1], this->pixels[i+2], h, s, v );
            temp_pixels[j] = this->pixels[i];
            temp_pixels[j+1] = this->pixels[i+1];
            temp_pixels[j+1] = this->pixels[i+2];
            temp_pixels[j+3] = 255;
            j += 4;
        }
        this->pixels = temp_pixels;
        this->channels = 4;
    }

    /*
        pixel values between hue range have alpha values set to 0.
        pixel values outside of hue range have alpha values set to 255.
        Input: None
        Output: None
    */
    void chromaKey(){
        double hueMin, hueMax, satMax, valMax;
        unsigned char* temp_pixels = new unsigned char[this->width*this->height*this->channels];
        ifstream in;
        in.open("maskValues.txt");
        if( in ){
            in >> hueMin >> hueMax >> satMax >> valMax;
        } else {
            cout << "No input values found. Default values used." << endl;
            // default values for Green Chroma-Key
            hueMin = 70;
            hueMax = 170.0;
            satMax = 81.0;
            valMax = 27.0;
        }

        double h, s, v;
        int j = 3;
        for( int i=0; i<( this->width*this->height*this->channels )-this->channels; i+=this->channels ){
            /* prevent rewrite of pixel[j] */
            // if( i == j ){
            //     j += 4;
            //     continue;
            // }
            RGBtoHSV( this->pixels[i], this->pixels[i+1], this->pixels[i+2], h, s, v );

            // alphamasking using h,s and v values
            if(( h > hueMin && h < hueMax ) && ( s > 0 && s <= satMax) && ( v > 0 && v <= valMax ))
                temp_pixels[j] = 0;
            else
                temp_pixels[j] = 255;

            // pre-multiplying each color channel
            temp_pixels[i] = temp_pixels[j]*this->pixels[i];
            j+=4;
        }
        this->pixels = temp_pixels;
    }

    /*
        A over B.

        Input: image[Pixmap]
        Output: void
    */
    void compose( Pixmap image ){
        double r, g, b, a;
        double newR, newG, newB, newA;
        int j=0;
        int currentRow = 0;
        int w = image.width*image.channels;
        for( int i=0; i<image.width*image.height*image.channels; i+=image.channels ){
            if( i>(w+(w*currentRow)) ){
                currentRow++;
                j = ( this->width*this->channels )*currentRow;
            }
            a = image.pixels[i+3]/255;
            if( a==0){
                j += this->channels;
                continue;
            }
            r = image.pixels[i]*a;
            g = image.pixels[i+1]*a;
            b = image.pixels[i+2]*a;

            // Perform Over operation
            newR = r+(1-a)*this->pixels[i];
            newG = g+(1-a)*this->pixels[i+1];
            newB = b+(1-a)*this->pixels[i+2];
            newA = a+(1-a)*this->pixels[i+3];
            this->pixels[j] = newR;
            this->pixels[j+1] = newG;
            this->pixels[j+2] = newB;
            this->pixels[j+3] = newA;
            j += image.channels;
        }
    }
};

Pixmap pixmap = Pixmap();

/*
* Displays currrent pixmap onto screen
*/
void display(){
    glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT);
    glutReshapeWindow(pixmap.width,pixmap.height);
	// convert to array before passing into glDrawPixels
	unsigned char* pixmap_array = pixmap.pixels;
	glDrawPixels(pixmap.width, pixmap.height, GL_RGB, GL_UNSIGNED_BYTE, pixmap_array );
	glFlush();
}

/*
    Keyboard Call Back Routine:
        q,Q: Quits program
        Default: do nothing
*/
void handleKey( unsigned char key, int x, int y ){
    switch( key ){
        case 'q':
        case 'Q':
            exit( 0 );
        case 'w':
        case 'W':
        {
            string output_file;
            cout << "Enter the name of the file to write to: ";
            cin >> output_file;
            cout << endl;
            pixmap.writeImage( output_file );
            break;
        }
        default:
            return;
    }
}

/*
    handles reshape of window displayed
    Input: w[int], h[int]
        w = width of window
        h = height of window
    Output: void
*/
void handleReshape( int w, int h ){
    // set the viewport to be the entire window
	glViewport(0,0,w,h);

	// define the drawing coordinate system on the viewport
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, w, 0, h, 0.1, 1 );

    // flip the image
    glPixelZoom( 1, -1 );
    glRasterPos3f( 0, h-1, -0.3 );
}
