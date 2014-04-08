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

/* random_device probably won't work in mingw since apparently there's no working generator.
It would have to be replaced by something like rand_s and/or seed sequence. re random_device:

"Unless the program really requires a stochastic process to generate random numbers, a portable
program is encouraged to use an alternate pseudo-random number generator engine instead, or at least
provide a recovery method for such exceptions."
*/
static random_device seedgen;
mt19937 mersenne( seedgen );


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
