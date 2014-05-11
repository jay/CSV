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

#ifndef STRESSTEST_READ_
#define STRESSTEST_READ_

#include <list>
#include <string>
#include <vector>

#include "CSV.hpp"

bool read_records(
    const char *filename,
    const bool utf8bom,
    const int max_ramdisk_size,
    const bool process_empty,
    const size_t expected_records_count,
    jay::util::CSVread &csv_read, // INOUT
    std::list<std::vector<std::string>> &records // OUT
);

#endif // STRESSTEST_READ_
