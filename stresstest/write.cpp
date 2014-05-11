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

/** write csv test
*/

#include "write.hpp"

#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "util.hpp"

#include "CSV.hpp"
#include "strerror.hpp"


using namespace std;


// no CSVwrite::Close() on fail
bool write_records(
    const char *filename,
    const bool utf8bom,
    const int max_ramdisk_size,
    const bool process_empty,
    const bool truncate,
    const list<vector<string>> &records,
    jay::util::CSVwrite &csv_write // INOUT
)
{
    bool b = false;

    ofstream out_file;

    bool use_association = getrand<bool>();

    jay::util::CSVwrite::Flags flags = jay::util::CSVwrite::none;

    if( process_empty )
    {
        flags |= jay::util::CSVwrite::process_empty_records;
    }

    // Truncate flag may only be passed to Open()
    if( truncate && !use_association )
    {
        flags |= jay::util::CSVwrite::truncate;
    }

    bool use_flags = ( flags != jay::util::CSVwrite::none ) || getrand<bool>();

    if( use_association )
    {
        if( truncate )
        {
            out_file.open( filename, ios::trunc | ios::binary );
        }
        else
        {
            out_file.open( filename, ios::binary );
        }

        DEBUG_IF( ( !out_file.is_open() ),
            "Problem opening file " << filename << " : " << jay::util::ios_strerror( out_file.rdstate() ) );

        if( use_flags )
        {
            b = csv_write.Associate( &out_file, flags );
        }
        else
        {
            b = csv_write.Associate( &out_file );
        }

        DEBUG_IF( ( b == csv_write.error ),
            "Logic mismatch on csv_write.Associate(). b: " << b << ", csv_write.error: " << csv_write.error );

        DEBUG_IF( ( csv_write.error ),
            "Problem associating stream: " << csv_write.error_msg );
    }
    else
    {
        if( use_flags )
        {
            b = csv_write.Open( filename, flags );
        }
        else
        {
            b = csv_write.Open( filename );
        }

        DEBUG_IF( ( b == csv_write.error ),
            "Logic mismatch on csv_write.Open(). b: " << b << ", csv_write.error: " << csv_write.error );

        DEBUG_IF( ( csv_write.error ),
            "Problem opening file " << filename << ": " << csv_write.error_msg );
    }

    bool use_resize_buffer = getrand<bool>();
    if( use_resize_buffer )
    {
        b = csv_write.ResizeBuffer( getrand( 1, max_ramdisk_size * 2 ) );

        DEBUG_IF( ( b == csv_write.error ),
            "Logic mismatch on csv_write.ResizeBuffer(). b: " << b << ", csv_write.error: " << csv_write.error );

        DEBUG_IF( ( csv_write.error ),
            "Problem resizing buffer: " << csv_write.error_msg );
    }

    if( utf8bom )
    {
        DEBUG_IF( ( !truncate ),
            "trucate == false. Not implemented." );

        csv_write.WriteUTF8BOM();
    }

    for( list<vector<string>>::const_iterator it = records.begin(); it != records.end(); ++it ) // for each record
    {
        bool use_WriteRecord = getrand<bool>();
        bool use_WriteTerminator = ( it->size() ? getrand<bool>() : process_empty );

        if( use_WriteRecord )
        {
            if( use_WriteTerminator )
            {
                b = csv_write.WriteRecord( *it, false );
            }
            else
            {
                b = csv_write.WriteRecord( *it );
            }

            DEBUG_IF( ( b == csv_write.error ),
                "Logic mismatch on csv_write.WriteRecord(). b: " << b << ", csv_write.error: " << csv_write.error );

            DEBUG_IF( ( csv_write.error ),
                "Problem writing record: " << csv_write.error_msg );
        }
        else
        {
            for( size_t i = 0; i < it->size(); ++i )
            {
                if( ( ( i + 1 ) == it->size() ) && !use_WriteTerminator )
                {
                    b = csv_write.WriteField( (*it)[ i ], true );
                }
                else
                {
                    b = csv_write.WriteField( (*it)[ i ] );
                }

                DEBUG_IF( ( b == csv_write.error ),
                    "Logic mismatch on csv_write.WriteField(). b: " << b << ", csv_write.error: " << csv_write.error );

                DEBUG_IF( ( csv_write.error ),
                    "Problem writing field: " << csv_write.error_msg );
            }
        }

        if( use_WriteTerminator )
        {
            b = csv_write.WriteTerminator();

            DEBUG_IF( ( b == csv_write.error ),
                "Logic mismatch on csv_write.WriteTerminator(). b: " << b << ", csv_write.error: " << csv_write.error );

            DEBUG_IF( ( csv_write.error ),
                "Problem writing terminator: " << csv_write.error_msg );
        }
    }

    b = csv_write.Close();
    DEBUG_IF( ( b == csv_write.error ),
        "Logic mismatch on csv_write.Close(). b: " << b << ", csv_write.error: " << csv_write.error );

    return b;
}
