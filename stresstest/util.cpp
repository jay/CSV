/*
Copyright (C) 2014 Jay Satiro <raysatiro@yahoo.com>
All rights reserved.

This file is part of stresstest/CSV/jay::util.

https://github.com/jay/CSV

jay::util is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

jay::util is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with jay::util. If not, see <http://www.gnu.org/licenses/>.
*/

/** utilities
*/

#define _CRT_SECURE_NO_WARNINGS
#include "util.hpp"

#include <algorithm>
#include <functional>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "CSV.hpp"
#include "strerror.hpp"


using namespace std;


/* Cannot properly save and restore the state of the mersenne twister in older versions of gcc.
For example the bug is present in gcc-4.6.3.
http://stackoverflow.com/questions/5999144/how-to-save-state-of-c0x-random-number-generator
*/
bool is_mt19937_state_bug_present()
{
    stringstream ss;
    mt19937 engine1, engine2;
    engine1();
    ss.setf( ios::left );
    ss << engine1;
    ss >> engine2;
    return ( ( engine1 != engine2 ) || ( engine1() != engine2() ) );
}


/* random_device notes

It isn't always available and may not work even when it is. According to cplusplus it should throw
an exception if it's not available to produce random numbers but from what I've seen that may not
happen in some versions of gcc. And it probably won't work in MinGW since apparently there's no
working generator. It would have to be replaced by something like rand_s and/or seed sequence.

random_device notes from cplusplus:

"Notice that random devices may not always be available to produce random numbers (and in some
systems, they may even never be available). This is signaled by throwing an exception derived from
the standard exception on construction or when a number is requested with operator()."

"Unless the program really requires a stochastic process to generate random numbers, a portable
program is encouraged to use an alternate pseudo-random number generator engine instead, or at least
provide a recovery method for such exceptions."
*/

#ifdef __GNUC__
#error "GCC random_device support is broken. Remove this check if you have a working random_device."
#endif

mt19937 mersenne;

static void init_mersenne()
{
    if( is_mt19937_state_bug_present() )
    {
        cerr << "FATAL: init_mersenne(): "
            << "There is a bug in your compiler's implementation of the mersenne twister." << endl;
        exit( 1 );
    }

    //
    // Visual Studio 2013 compatible initialization of mersenne twister.
    // http://connect.microsoft.com/VisualStudio/feedback/details/875492
    //
    vector<uint32_t> v( mt19937::state_size );
    random_device rd;

    generate( v.begin(), v.end(), ref( rd ) );
    seed_seq seed( v.begin(), v.end() );

    mersenne.seed( seed );
}


// must be called by main init()
void util_init()
{
    init_mersenne();
}


string DateTimeForFilename()
{
    time_t now = time( 0 );
    struct tm tm = *localtime( &now );
    stringstream ss;
    ss << setfill( '0' )
        << setw( 4 ) << ( 1900 + tm.tm_year )
        << setw( 2 ) << ( tm.tm_mon + 1 )
        << setw( 2 ) << tm.tm_mday
        << "_"
        << setw( 2 ) << tm.tm_hour
        << setw( 2 ) << tm.tm_min
        << setw( 2 ) << tm.tm_sec;
    return ss.str();
}


bool SaveOutputToFile( const string &filename, const string &output )
{
    bool func_retval = true;

    ofstream file( filename );
    file.setf( ios::left );
    file << output;
    if( !file )
    {
        cerr << endl << "WARNING: Failed to save " + filename << endl;
        func_retval = false;
    }
    file.close();

    return func_retval;
}


bool SaveErrorState( const string &errmsg )
{
    bool func_retval = true;

    extern size_t iteration;
    extern stringstream mersenne_state_initial, mersenne_state_iteration, mersenne_state_iteration_prev;

    string prefix = "error_" + DateTimeForFilename();

    stringstream ss;
    ss << "Iteration " << FormatWithCommas<size_t>( iteration ) << endl << errmsg << endl;
    if( !SaveOutputToFile( prefix + "_message.txt", ss.str() ) )
    {
        func_retval = false;
    }

    if( !SaveOutputToFile( prefix + "_state_initial.txt", mersenne_state_initial.str() ) )
    {
        func_retval = false;
    }

    if( !SaveOutputToFile( prefix + "_state_iteration.txt", mersenne_state_iteration.str() ) )
    {
        func_retval = false;
    }

    if( !SaveOutputToFile( prefix + "_state_iteration_prev.txt", mersenne_state_iteration_prev.str() ) )
    {
        func_retval = false;
    }

    if( !SaveOutputToFile( "error_last.txt", prefix ) )
    {
        func_retval = false;
    }

    return func_retval;
}


void RemoveTrailingSpaces( string &s )
{
    s.erase( find_if( s.rbegin(), s.rend(), bind1st( not_equal_to<char>(), ' ' ) ).base(), s.end() );
}
