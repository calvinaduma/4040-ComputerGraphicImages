/*
    Calvin Aduma

    CPSC 4040 Ioannis Karamouzas

    Final Project

    This program takes 9 color palettes and maps them into a 3x3
    square on the image.

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
    glutCreateWindow( "REVEAL" );

    glutDisplayFunc( handleDisplay );
    glutKeyboardFunc( handleKey );
    glutReshapeFunc( handleReshape );
    glutMouseFunc( handleMouseClick );
    
    glutMainLoop();

    return ( 0 );
}