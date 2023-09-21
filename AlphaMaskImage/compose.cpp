/*
    Calvin Aduma

    CPSC 4040 Ioannis Karamouzas

    Project 3

    This program takes in 2 images and produces an image of the first over the second.
    Or in other words: A over B.
    It then has the option to write this image onto a file.

*/

#include <string>
#include <iostream>
#include "BasicFunctions.h"

int main( int argc, char* argv[] ){
    string file_A = "";
    string file_B = "";
    string output_file = "";
    if ( argc < 3 || argc > 4 ){
        cout << "You MUST enter in format:" << endl;
        cout << "./compose ImageA.img ImageB.img" << endl;
        cout << "OR" << endl;
        cout << "./compose ImageA.img ImageB.img OutputFile.img" << endl;
    }

    file_A = argv[1];
    file_B = argv[2];
    if( argc == 4 ){
        output_file = argv[3];
    }

    Pixmap A = Pixmap(file_A);
    Pixmap B = Pixmap(file_B);

    B.convertToFourChannels();
    B.compose(A);

    if( argc == 4 ) B.writeImage( output_file );

    pixmap = B;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(pixmap.width, pixmap.height);
    glutCreateWindow("Compose");

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKey);
    glutReshapeFunc(handleReshape);
    glutMainLoop();
    return 0;
}
