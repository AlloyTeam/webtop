//
// This header file defines the ZlibEngine class used
// perform file compression and decompression using
// Zlib.
//
// The ZlibEngine is a Tiny Software (tm) project.
// You may use the code in this project without restriction.
// Contact markn@tiny.com for more information.
//

#if !defined( _ZLIBENGN_H )
#define _ZLIBENGN_H

//
// All of the Zlib code is compiled in C modules.  Fortunately,
// I can wrap the entire header in an 'extern "C"' declaration,
// and it will then link properly!
//

extern "C" {
#include "zlib.h"
}

#include <stdio.h>
#include <io.h>

class ZlibEngine : public z_stream {
    public :
        ZlibEngine();
        int compress( const char *input,
                      const char *output,
                      int level = 6 );
        int decompress( const char *input,
                        const char *output );
        void set_abort_flag( int i ){ m_AbortFlag = i; }
//
// These three functions are only used internally.
//
    protected :
        int percent();
        int load_input();
        int flush_output();
//
// Derived classes can provide versions of this
// virtual fns in order to customize their
// program's user interface.  The abort flag
// can be set by those functions.
//
    protected :
        virtual void progress( int percent ){};
        virtual void status( char *message ){};
        int m_AbortFlag;
//
// The remaining data members are used internally/
//
    protected :
        FILE *fin;
        FILE *fout;
        long length;
        int err;
        enum { input_length = 4096 };
        unsigned char input_buffer[ input_length ];
        enum { output_length = 4096 };
        unsigned char output_buffer[ output_length ];
};

//
// I define one error code in addition to those
// found in zlib.h
//
#define Z_USER_ABORT (-7)

#endif  // #if !defined( _ZLIBENGN_H )
