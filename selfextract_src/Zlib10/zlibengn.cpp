//
// This file contains the implementation the
// the ZlibEngine class, used to simplify
// compression and decompression of files
// using the Zlib engine.
//
// The ZlibEngine is a Tiny Software (tm) project.
// You may use the code in this project without restriction.
// Contact markn@tiny.com for more information.
//

#if defined( _WINDOWS )
#include <windows.h>
#endif

#include "zlibengn.h"

//
// The constructor initializes a couple of members
// of the z_stream class.  See the Zlib documentation
// for details on what those members do
//

ZlibEngine::ZlibEngine()
{
    zalloc = 0;  //z_stream member
    zfree = 0;   //z_stream member
    opaque = 0;  //z_stream member
//
// I initialize these members just for tidiness.
//
    fin = 0;
    fout = 0;
}

//
// compress() is the public function used to compress
// a single file.  It has to take care of opening the
// input and output files and setting up the buffers for
// Zlib.  It then calls deflate() repeatedly until all
// input and output processing has been done, and finally
// closes the files and cleans up the Zlib structures.
//
int ZlibEngine::compress( const char *input,
                          const char *output,
                          int level )
{
    err = Z_OK;
    avail_in = 0;
    avail_out = output_length;
    next_out = output_buffer;
    m_AbortFlag = 0;

    fin  = fopen( input, "rb" );
    fout = fopen( output, "wb" );
    length = filelength( fileno( fin ) );
    deflateInit( this, level );
    for ( ; ; ) {
        if ( m_AbortFlag )
            break;
        if ( !load_input() )
            break;
        err = deflate( this, Z_NO_FLUSH );
        flush_output();
        if ( err != Z_OK )
            break;
        progress( percent() );
    }
    for ( ; ; ) {
        if ( m_AbortFlag )
            break;
        err = deflate( this, Z_FINISH );
        if ( !flush_output() )
            break;
        if ( err != Z_OK )
            break;
    }
    progress( percent() );
    deflateEnd( this );
    if ( m_AbortFlag )
        status( "User Abort" );
    else if ( err != Z_OK && err != Z_STREAM_END )
        status( "Zlib Error" );
    else {
        status( "Success" );
        err = Z_OK;
    }
    fclose( fin );
    fclose( fout );
    fin = 0;
    fout = 0;
    if ( m_AbortFlag )
        return Z_USER_ABORT;
    else
        return err;
}

//
// decompress has to do most of the same chores as compress().
// The only major difference it has is the absence of the level
// parameter.  The level isn't needed when decompressing data
// using the deflate algorithm.
//

int ZlibEngine::decompress( const char *input,
                            const char *output )
{
    err = Z_OK;
    avail_in = 0;
    avail_out = output_length;
    next_out = output_buffer;
    m_AbortFlag = 0;

    fin  = fopen( input, "rb" );
    fout = fopen( output, "wb" );
    length = filelength( fileno( fin ) );
    inflateInit( this );
    for ( ; ; ) {
        if ( m_AbortFlag )
            break;
        if ( !load_input() )
            break;
        err = inflate( this, Z_NO_FLUSH );
        flush_output();
        if ( err != Z_OK )
            break;
        progress( percent() );
    }
    for ( ; ; ) {
        if ( m_AbortFlag )
            break;
        err = inflate( this, Z_FINISH );
        if ( !flush_output() )
            break;
        if ( err != Z_OK )
            break;
    }
    progress( percent() );
    inflateEnd( this );
    if ( m_AbortFlag )
        status( "User Abort" );
    else if ( err != Z_OK && err != Z_STREAM_END )
        status( "Zlib Error" );
    else {
        status( "Success" );
        err = Z_OK;
    }
    if ( fin )
        fclose( fin );
    fin = 0;
    if ( fout )
        fclose( fout );
    fout = 0;
    if ( m_AbortFlag )
        return Z_USER_ABORT;
    else
        return err;
}
//
//  This function is called so as to provide the progress()
//  virtual function with a reasonable figure to indicate
//  how much processing has been done.  Note that the length
//  member is initialized when the input file is opened.
//
int ZlibEngine::percent()
{
    if ( length == 0 )
        return 100;
    else if ( length > 10000000L )
        return ( total_in / ( length / 100 ) );
    else
        return ( total_in * 100 / length );
}

//
//  Every time Zlib consumes all of the data in the
//  input buffer, this function gets called to reload.
//  The avail_in member is part of z_stream, and is
//  used to keep track of how much input is available.
//  I churn the Windows message loop to ensure that
//  the process can be aborted by a button press or
//  other Windows event.
//
int ZlibEngine::load_input()
{
#if defined( _WINDOWS )
    MSG msg;
    while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
#endif
    if ( avail_in == 0 ) {
        next_in = input_buffer;
        avail_in = fread( input_buffer, 1, input_length, fin );
    }
    return avail_in;
}

//
//  Every time Zlib filsl the output buffer with data,
//  this function gets called.  Its job is to write
//  that data out to the output file, then update
//  the z_stream member avail_out to indicate that more
//  space is now available.  I churn the Windows message
//  loop to ensure that the process can be aborted by a
//  button press or other Windows event.
//

int ZlibEngine::flush_output()
{
#if defined( _WINDOWS )
    MSG msg;
    while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
#endif
    unsigned int count = output_length - avail_out;
    if ( count ) {
        if ( fwrite( output_buffer, 1, count, fout ) != count ) {
            err = Z_ERRNO;
            return 0;
        }
        next_out = output_buffer;
        avail_out = output_length;
    }
    return count;
}


