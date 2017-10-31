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

/** read csv test
*/

#include "read.hpp"

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


// no CSVread::Close() on fail
bool read_records(
    const char *filename,
    const bool utf8bom,
    const int max_ramdisk_size,
    const bool process_empty,
    const size_t expected_records_count,
    jay::util::CSVread &csv_read, // INOUT
    list<vector<string>> &records // OUT
)
{
    bool b = false;

    ifstream in_file;

    bool use_association = getrand<bool>();

    jay::util::CSVread::Flags flags = jay::util::CSVread::none;

    if( process_empty )
    {
        flags |= jay::util::CSVread::process_empty_records;
    }

    // Maybe skip the BOM check, only if there's no BOM
    bool skip_utf8_bom = !utf8bom && getrand<bool>();
    if( skip_utf8_bom )
    {
        flags |= jay::util::CSVread::skip_utf8_bom_check;
    }

    bool strict_mode = getrand<bool>();
    if( strict_mode )
    {
        flags |= jay::util::CSVread::strict_mode;
    }

    bool use_flags = ( flags != jay::util::CSVread::none ) || getrand<bool>();

    if( use_association )
    {
        in_file.open( filename, ios::binary );

        DEBUG_IF( ( !in_file.is_open() ),
            "Problem opening file " << filename << " : " << jay::util::ios_strerror( in_file.rdstate() ) );

        if( use_flags )
        {
            b = csv_read.Associate( &in_file, flags );
        }
        else
        {
            b = csv_read.Associate( &in_file );
        }

        DEBUG_IF( ( b == csv_read.error ),
            "Logic mismatch on csv_read.Associate(). b: " << b << ", csv_read.error: " << csv_read.error );

        DEBUG_IF( ( csv_read.error ),
            "Problem associating stream: " << csv_read.error_msg );
    }
    else
    {
        if( use_flags )
        {
            b = csv_read.Open( filename, flags );
        }
        else
        {
            b = csv_read.Open( filename );
        }

        DEBUG_IF( ( b == csv_read.error ),
            "Logic mismatch on csv_read.Open(). b: " << b << ", csv_read.error: " << csv_read.error );

        DEBUG_IF( ( csv_read.error ),
            "Problem opening file " << filename << ": " << csv_read.error_msg );
    }

    bool use_resize_buffer = getrand<bool>();
    if( use_resize_buffer )
    {
        b = csv_read.ResizeBuffer( getrand( 1, max_ramdisk_size * 2 ) );

        DEBUG_IF( ( b == csv_read.error ),
            "Logic mismatch on csv_read.ResizeBuffer(). b: " << b << ", csv_read.error: " << csv_read.error );

        DEBUG_IF( ( csv_read.error ),
            "Problem resizing buffer: " << csv_read.error_msg );
    }

    bool use_sequential_read = getrand<bool>();
    if( use_sequential_read )
    {
        for( ;; )
        {
            b = csv_read.ReadRecord();

            DEBUG_IF( ( b == csv_read.error ),
                "Sequential acccess: Logic mismatch on csv_read.ReadRecord(). b: " << b
                    << ", csv_read.error: " << csv_read.error );

            if( !b )
            {
                break;
            }

            records.push_back( csv_read.fields );
        }

        DEBUG_IF( ( !csv_read.eof
                || ( csv_read.record_num != csv_read.end_record_num )
                || ( expected_records_count != csv_read.end_record_num ) ),
            "Sequential acccess: End record unknown." );
    }
    else
    {
        vector<int> tmp, indexes;

        for( size_t i = 1; i <= expected_records_count; ++i )
        {
            tmp.push_back( i );
        }

        while( tmp.size() )
        {
            size_t n = getrand<size_t>( 0, tmp.size() - 1 );
            indexes.push_back( tmp[ n ] );
            tmp.erase( tmp.begin() + n );
        }

        vector<vector<string>> tmprec( expected_records_count );

        for( size_t i = 0; i < indexes.size(); ++i )
        {
            b = csv_read.ReadRecord( indexes[ i ] );

            DEBUG_IF( ( b == csv_read.error ),
                "Random access: Logic mismatch on csv_read.ReadRecord(). b: " << b << ", csv_read.error: " << csv_read.error );

            DEBUG_IF( ( !b ),
                "Random access: Failed to read record " << indexes[ i ] << "." );

            tmprec[ indexes[ i ] - 1 ] = csv_read.fields;
        }

        b = csv_read.ReadRecord( expected_records_count + 1 );

        DEBUG_IF( ( b == csv_read.error ),
            "Random access: Logic mismatch on csv_read.ReadRecord(). b: " << b << ", csv_read.error: " << csv_read.error );

        DEBUG_IF( ( b ),
            "Random access: More records than expected." );

        DEBUG_IF( !csv_read.eof || ( expected_records_count != csv_read.end_record_num ),
            "Random access: End record unknown." );

        records.assign( tmprec.begin(), tmprec.end() );
    }

    DEBUG_IF( ( csv_read.end_record_not_terminated ),
        "End record not terminated!" );

    return true;
}
