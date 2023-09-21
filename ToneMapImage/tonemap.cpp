/*
    Calvin Aduma

    CPSC 4040 Ioannis Karamouzas

    Project 5

    This program tone maps HDR images
*/

#include "functions.h"

int main( int argc, char* argv[] ){

    if( argc < 2 ){
        cout << "Command Line Error: Too little args! Exiting..." << endl;
        return( 0 );
    }

    if( argc == 3) output_filename = argv[2];

    if( argc > 3 ){
        cout << "Command Line Error: Too many args! Exiting..." << endl;
        return( 0 );
    }

    readImage( argv[1] );

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA );
    glutInitWindowSize( winWIDTH, winHEIGHT );
    glutCreateWindow( "TONEMAP" );

    glutDisplayFunc( handleDisplay );
    glutKeyboardFunc( handleKey );
    glutReshapeFunc( handleReshape );

    glutMainLoop();

    return ( 0 );
}
