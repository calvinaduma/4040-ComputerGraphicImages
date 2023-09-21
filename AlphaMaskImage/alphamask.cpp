/*
    Calvin Aduma

    CPSC 4040 Ioannis Karamouzas

    Project 3

    This program takes an image input and produces a 4 channel version of the image

*/
#include <iostream>
#include <stdlib.h>
#include <OpenImageIO/imageio.h>
#include <string>
#include <math.h>
#include <fstream>
#include "BasicFunctions.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

int main( int argc, char* argv[] ){
    string input_file = "";
    string output_file = "";

    // checks to make sure input is valid
    if ( argc != 3 ){
        cout << "You MUST enter an input file and outputfile ONLY!" << endl;
        cout << "Enter name of input file: ";
        cin >> input_file;
        cout << "\nEnter name of output file: ";
        cin >> output_file;
        cout << endl;
    } else {
        input_file = argv[ 1 ];
        output_file = argv[ 2 ];
    }

    Pixmap pixmap = Pixmap( input_file );
    pixmap.convertToFourChannels();
    pixmap.chromaKey();
    pixmap.writeImage( output_file );
    return 0;
}