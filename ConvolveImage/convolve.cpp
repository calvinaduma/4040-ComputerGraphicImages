/*
    Calvin Aduma

    CPSC 4040 Ioannis Karamouzas

    Project 4

    This program filters images using convolution
*/
#include <fstream>
#include "functions.h"

int main( int argc, char* argv[] ){
    if( argc > 4 ){
        cout << "Command Line Error: Too many args! Exiting..." << endl;
        return( 0 );
    }
    if( argc < 3 ){
        cout << "Command Line Error: Missing args! Exiting..." << endl;
        return( 0 );
    }

    parseFilter( argv[1] );
    normalizeFilter();
    flipKernel();

    readImage( argv[2] );

    if( argc == 4 ) output_filename = argv[3];
    else output_filename = "";

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA );
    glutInitWindowSize( winWIDTH, winHEIGHT );
    glutCreateWindow( "CONVOLVE" );

    glutDisplayFunc( handleDisplay );
    glutKeyboardFunc( handleKey );
    glutReshapeFunc( handleReshape );

    glutMainLoop();
    return( 0 );
}
