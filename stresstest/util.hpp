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

#ifndef STRESSTEST_UTIL_
#define STRESSTEST_UTIL_

#include <iostream>
#include <locale>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>


extern std::mt19937 mersenne;

bool is_mt19937_state_bug_present();
void util_init();
bool SaveErrorState( const std::string &errmsg );
void RemoveTrailingSpaces( std::string &s );


// http://stackoverflow.com/questions/7276826/c-format-number-with-commas/7276879#7276879
template<class T>
std::string FormatWithCommas(T value)
{
    static std::locale l("");
    std::stringstream ss;
    ss.imbue(l);
    ss << std::fixed << value;
    return ss.str();
}

/* These getrand<>() don't use a static std::uniform_int_distribution() so depending on the
implementation they may waste a lot of bits. Also they're not thread safe.
I'm using mersenne engine so that I can save the state and reproduce. If I were to use a static
uniform_int_distribution I'd have to save that state as well for any reproduciton.
*/

// Visual Studio 2010's uniform_int_distribution doesn't handle signed types properly.
// http://connect.microsoft.com/VisualStudio/feedback/details/712984
// Here I convert to unsigned, get the random number, then convert back to signed.
template<typename T>
typename std::enable_if<
    std::is_same<T, short>::value
        || std::is_same<T, int>::value
        || std::is_same<T, long>::value
        || std::is_same<T, long long>::value,
    T>::type getrand( T min, T max )
{
    typedef typename std::make_unsigned<T>::type utype;
    const utype adjust = ( utype( -1 ) >> 1 ) + 1;
    const utype min_u = ( utype( min ) < adjust ) ? ( utype( min ) + adjust ) : ( utype( min ) - adjust );
    const utype max_u = ( utype( max ) < adjust ) ? ( utype( max ) + adjust ) : ( utype( max ) - adjust );
    utype rand_u = std::uniform_int_distribution<utype>( min_u, max_u )( mersenne );
    rand_u = ( utype( rand_u ) < adjust ) ? ( utype( rand_u ) + adjust ) : ( utype( rand_u ) - adjust );
    return (
        ( rand_u > utype( (std::numeric_limits<T>::max)() ) )
        ? -(T)( utype( -1 ) - rand_u ) - 1
        : rand_u
    );
}

template<typename T>
typename std::enable_if<
    std::is_same<T, unsigned short>::value
        || std::is_same<T, unsigned int>::value
        || std::is_same<T, unsigned long>::value
        || std::is_same<T, unsigned long long>::value,
    T>::type getrand( T min, T max )
{
    T rand_u = std::uniform_int_distribution<T>( min, max )( mersenne );
    return rand_u;
}

template<typename T>
typename std::enable_if<
    std::is_same<T, short>::value
        || std::is_same<T, int>::value
        || std::is_same<T, long>::value
        || std::is_same<T, long long>::value
        || std::is_same<T, unsigned short>::value
        || std::is_same<T, unsigned int>::value
        || std::is_same<T, unsigned long>::value
        || std::is_same<T, unsigned long long>::value,
    T>::type getrand()
{
    return getrand( (std::numeric_limits<T>::min)(), (std::numeric_limits<T>::max)() );
}

template<typename T>
typename std::enable_if<
    std::is_same<T, char>::value
        || std::is_same<T, signed char>::value
        || std::is_same<T, unsigned char>::value,
    T>::type getrand( T min, T max )
{
    return static_cast<T>(
        getrand<std::conditional<std::is_signed<T>::value, signed, unsigned>::type>( min, max )
    );
}

template<typename T>
typename std::enable_if<
    std::is_same<T, char>::value
        || std::is_same<T, signed char>::value
        || std::is_same<T, unsigned char>::value,
    T>::type getrand()
{
    return static_cast<T>(
        getrand<std::conditional<std::is_signed<T>::value, signed, unsigned>::type>(
            (std::numeric_limits<T>::min)(), (std::numeric_limits<T>::max)()
        )
    );
}

template<typename T>
typename std::enable_if<
    std::is_same<T, bool>::value,
    T>::type getrand()
{
    return ( getrand( 0, 1 ) ? true : false );
}


#define DEBUG_IF(expr, msg)   \
    if( expr ) \
    { \
        std::string filename_d_(__FILE__); \
        size_t pos_d_ = filename_d_.find_last_of( "\\/" ); \
        if( pos_d_ != std::string::npos ) \
        { \
            filename_d_.erase( 0, pos_d_ + 1 ); \
        } \
        std::stringstream ss_d_; \
        ss_d_ << "ERROR: Expression is true: " << #expr << std::endl \
            << filename_d_ << ":" << __LINE__ << " , " << __FUNCTION__ << "(): " << msg; \
        std::cerr << std::endl << "\a\a" << ss_d_.str() << std::endl; \
        SaveErrorState( ss_d_.str() ); \
        __debugbreak(); \
        return false; \
    }

#endif // STRESSTEST_UTIL_
