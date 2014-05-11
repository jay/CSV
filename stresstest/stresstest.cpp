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

/** Stresstest for CSV
Expects a temporary ramdisk on drive T. Refer to comments at the beginning of
generate_and_compare() definition.
*/

#include <Windows.h>
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <list>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "read.hpp"
#include "util.hpp"
#include "write.hpp"

#include "CSV.hpp"
#include "strerror.hpp"


using namespace std;


static void pause( void )
{
        system( "pause" );
        return;
}

void init()
{
    /* If the program is started in its own window then pause before exit
    (eg user clicks on program in explorer, or vs debugger initiated program)
    */
    {
        HANDLE hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        ZeroMemory( &csbi, sizeof( csbi ) );

        if( ( hOutput != INVALID_HANDLE_VALUE )
            && ( GetFileType( hOutput ) == FILE_TYPE_CHAR )
            && GetConsoleScreenBufferInfo( hOutput, &csbi )
            && !csbi.dwCursorPosition.X
            && !csbi.dwCursorPosition.Y
            && ( csbi.dwSize.X > 0 )
            && ( csbi.dwSize.Y > 0 )
            )
        {
            // increase console buffer size to 10000 lines and pause before exit

            if( csbi.dwSize.Y < 10000 )
                csbi.dwSize.Y = 10000;

            SetConsoleScreenBufferSize( hOutput, csbi.dwSize );
            atexit( pause );
        }
    }
}


// clears 'records' and 'chars' before generating
void generate_list_of_random_records(
    const int max_space,
    const size_t delimiter_length,
    const size_t terminator_length,
    const bool count_empty_record_terminators,
    list<vector<string>> &records, // OUT: a list of randomly generated records
    int &chars // OUT: how many bytes would be needed to write the records as CSV
)
{
    chars = 0;
    list<vector<string>>().swap( records );

    bool stop = false;
    int max_records = getrand( 0, max_space );

    for( int i = 0; ( i < max_records ) && !stop ; ++i )
    {
        records.push_back( vector<string>() ); // pending record

        int max_fields = getrand( 0, max_space );

        for( int j = 0; ( j < max_fields ) && !stop ; ++j )
        {
            records.back().push_back( string() ); // pending field

            int max_length = getrand( 0, max_space );

            for( int k = 0; k < max_length; ++k )
            {
                if( chars > max_space )
                {
                    stop = true;
                    break;
                }

                records.back().back().push_back( getrand<char>() );

                // double quotes are escaped by another double quote before they're written
                chars += ( records.back().back().back() == '"' ) ? 2 : 1;
            }

            chars += 2; // double quotes surround each field
            chars += ( j ? delimiter_length : 0 ); // delimiter between fields
        }

        if( records.back().size() || count_empty_record_terminators )
        {
            chars += terminator_length; // terminator between records
        }
    }
}


bool generate_and_compare(
    jay::util::CSVread &csv_read,
    jay::util::CSVwrite &csv_write
)
{
    const char *const filename = "T:\\temp.csv";

    // - create ramdisk
    // imdisk -a -s 512M -m T: -p "/fs:ntfs /q /y"
    //
    // - delete ramdisk
    // imdisk -d -m T:
    // or imdisk -D -m T: to force a removal.

    // the max bytes to use on the ramdisk, might be a few bytes over
    // also i make buffer up to twice this size elsewhere
    const int max_ramdisk_size = 100; //1048576; // * 100;

    list<vector<string>> randlist;
    int randlist_chars = 0; // how many bytes are needed when the list is converted to csv format
    bool randlist_process_empty = getrand<bool>();

    // Get a random list of records to write
    generate_list_of_random_records(
        max_ramdisk_size,
        csv_write.delimiter.length(),
        csv_write.terminator.length(),
        randlist_process_empty,
        randlist,
        randlist_chars
    );

    bool utf8bom = getrand<bool>();
    if( utf8bom )
    {
        randlist_chars += 3;
    }

    bool truncate = true;
    DEBUG_IF( ( !truncate ),
        "Not implemented." );

    DEBUG_IF( !write_records(
            filename,
            utf8bom,
            max_ramdisk_size,
            randlist_process_empty,
            truncate,
            randlist,
            csv_write
        ),
        "write_records() failed." );

    fstream file( filename, ios::in | ios::ate | ios::binary );
    DEBUG_IF( ( !file ),
        "While getting file size after write: Failed opening file." );

    DEBUG_IF( ( file.tellg() != (streampos)randlist_chars ),
        "File size " << FormatWithCommas<streampos>(file.tellg()) << " != "
            << FormatWithCommas<int>(randlist_chars) << " expected file size." );

    file.close();


    list<vector<string>> list2;
    size_t list2_expected_count = 0;
    bool list2_process_empty = getrand<bool>();

    for( list<vector<string>>::iterator it = randlist.begin(); it != randlist.end(); ++it )
    {
        if( it->size() || ( randlist_process_empty && list2_process_empty ) )
        {
            ++list2_expected_count;
        }
    }

    DEBUG_IF( !read_records(
            filename,
            utf8bom,
            max_ramdisk_size,
            list2_process_empty,
            list2_expected_count,
            csv_read,
            list2
        ),
        "read_records() failed." );

    DEBUG_IF( ( list2.size() != list2_expected_count ),
                    "Unexpected number of records read." );

    list<vector<string>>::iterator it1 = randlist.begin();
    list<vector<string>>::iterator it2 = list2.begin();
    while( ( it1 != randlist.end() ) && ( it2 != list2.end() ) )
    {
        /* REM randlist may contain empty records even if !randlist_process_empty,
        but list2 should not contain empty records if !list2_process_empty
        */

        if( it2->size() )
        {
            if( !it1->size() ) // empty record in randlist
            {
                /* if empty records were both written to and then read from the csv file
                then *it1 should equal *it2, clearly not the case at this point.
                */
                DEBUG_IF( ( randlist_process_empty && list2_process_empty ),
                    "*it1 != *it2" );

                ++it1;
                continue;
            }
        }
        else
        {
            DEBUG_IF( ( !randlist_process_empty || !list2_process_empty ),
                "*it2 is empty, but empty records shouldn't have been processed." );
        }

        DEBUG_IF( ( *it1 != *it2 ),
            "*it1 != *it2" );

        ++it1, ++it2;
    }

    if( ( it1 != randlist.end() )
        && ( it2 == list2.end() )
        && ( !randlist_process_empty || !list2_process_empty )
    )
    {
        /* REM randlist may contain empty records even if !randlist_process_empty,
        but list2 should not contain empty records if !list2_process_empty
        */
        while( ( it1 != randlist.end() ) && !it1->size() )
        {
            ++it1;
        }
    }

    DEBUG_IF( ( ( it1 != randlist.end() ) || ( it2 != list2.end() ) ),
        "Extra records found during comparison of randlist and list2." );

    return true;
}


size_t iteration;
stringstream mersenne_state_initial, mersenne_state_iteration, mersenne_state_iteration_prev;

int main( int argc, char *argv[] )
{
    init();

    if( argc > 1 )
    {
        string filename = argv[ 1 ];
        cout << "Restoring state from " << filename << endl;
        ifstream file( filename );
        if( !file.is_open() )
        {
            cerr << endl << "Failed to restore state " << endl;
            exit( 1 );
        }
        file >> mersenne;
        file.close();
        cout << endl;
    }

    // spec ios::left flag must be used on strinsgream before writing engine state
    mersenne_state_initial.setf( ios::left );
    mersenne_state_iteration.setf( ios::left );
    mersenne_state_iteration_prev.setf( ios::left );

    // copy the initial state of the PRNG
    mersenne_state_initial << mersenne;
    mersenne_state_iteration << mersenne;
    mersenne_state_iteration_prev << mersenne;

    jay::util::CSVread csv_read;
    DEBUG_IF( ( csv_read.error ),
        "Problem creating CSVread: " << csv_read.error_msg );

    jay::util::CSVwrite csv_write;
    DEBUG_IF( ( csv_write.error ),
        "Problem creating CSVwrite: " << csv_write.error_msg );

    for( iteration = 1; iteration < SIZE_MAX; ++iteration )
    {
        mersenne_state_iteration_prev.swap( mersenne_state_iteration );
        mersenne_state_iteration.swap( std::stringstream() );
        mersenne_state_iteration.setf( ios::left );
        mersenne_state_iteration << mersenne;

        cout << "Iteration " << FormatWithCommas<size_t>( iteration ) << endl;

        DEBUG_IF( ( !generate_and_compare( csv_read, csv_write ) ),
            "generate_and_compare() failed." );

        DEBUG_IF( !csv_write.Close(),
            "Failed to close." );

        DEBUG_IF( !csv_read.Close(),
            "Failed to close." );

        // This value must match the default size used
        DEBUG_IF( !csv_write.ResizeBuffer( 4096 ),
            "Failed to reset the buffer." );

        // This value must match the default size used
        DEBUG_IF( !csv_read.ResizeBuffer( 4096 ),
            "Failed to reset the buffer." );
    }

    return 0;
}
