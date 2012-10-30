//
// This test program uses the ZlibEngine class
// to implement a short and sweet file compression
// program.  See the README file included with this
// source file for project building information.
//
// The ZlibEngine is a Tiny Software (tm) project.
// You may use the code in this project without restriction.
// Contact markn@tiny.com for more information.
//

#include <stdio.h>
#include <conio.h>
#include <limits.h>
#include "zlibengn.h"

//
// The only customization I need to do to the ZlibEngine
// class is to create my own progress function.  The
// progress function will be called periodically as the
// files are compressed and expanded.  If the user presses
// any key during the process an abort is signaled.
//

class MyZlibEngine : public ZlibEngine {
    public :
        void progress( int percent )
        {
            printf( "%3d%%\b\b\b\b", percent );
            if ( kbhit() ) {
                getch();
                m_AbortFlag = 1;
            }
        }
};

int main( int argc, char *argv[] )
{
    if ( argc < 2 ) {
        printf( "Usage: dostest input-file\n"
                "\nCompresses input-file to test.zl\n"
                "\nDecompresses test.zl to test.out\n" );
        return 0;
    }
    MyZlibEngine *c_stream = new MyZlibEngine;
    printf( "\nHit any key to abort!\n\n" );
    printf( "Compressing %s to test.zl.  "
            "Percent complete: ", argv[ 1 ] );
    int result = c_stream->compress( argv[ 1 ], "test.zl" );
    if ( result == Z_OK ) {
        printf( "\nDecompressing test.zl to test.out."
                "  Percent complete: " );
        result = c_stream->decompress( "test.zl", "test.out" );
        printf( "\n" );
    }
    if ( result != Z_OK )
        printf( "\nError! Result = %d\n", result );
    delete c_stream;
    return 1;
}

