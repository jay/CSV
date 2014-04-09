CSVread/CSVwrite
================

C++ classes to read/write CSV records from/to a stream.

Why..
-----

I couldn't find a good C++ library to handle multiline and arbitrary binary data in CSV files. I did find a great one written in C, [libcsv](http://sourceforge.net/projects/libcsv/) written by Robert Gamble. My classes use his library. If you want to learn more about the state of C++ CSV parsers look at stackoverflow:

* [parsing - How can I read and parse CSV files in C++? - Stack Overflow](http://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c)
* [How can I read and manipulate CSV file data in C++? - Stack Overflow](http://stackoverflow.com/questions/415515/how-can-i-read-and-manipulate-csv-file-data-in-c)

How do I...
-----------

The documentation is in [CSV/CSV.hpp](https://github.com/jay/CSV/blob/develop/CSV/CSV.hpp). The class source code is in the [CSV folder](https://github.com/jay/CSV/tree/develop/CSV) and at a minimum you'll need one of the GPLv3 license files and all h, c, hpp, and cpp files from that folder and a compiler that supports C89 and C++03. Include CSV.hpp in your source file. If you need an advanced feature only available in libcsv you'll have to include csv.h as well. If you have Visual Studio 2010+ you can add the project file CSV/CSV.vcxproj to your solution. Also there are two Visual Studio 2010 solutions included:


### CSV.sln
This solution will build the CSV library and run the stress test.

### Example/Example.sln
This solution will build the CSV library and run the example.


Tested?
-------

I wrote and executed stress testing in Visual Studio 2010 Professional x86 and have fixed every bug found. Each iteration the code path is randomized and so is the arbitrary CSV record data that is written to and read from the test file. Stress testing currently does not test text mode or append mode.

In Debug mode the most recent test completed 200,000+ iterations without fail before I ended it. In Release mode the test completed tens of millions of iterations before I ended it. The Debug tests are more thorough than the Release tests due to the extra checks Visual Studio does in debug mode but they are much much slower, especially when breaks, on-hit or watches are enabled and I use all of those. My meta info for debugging CSV.sln may or may not be found in breakpoints.xml, depending on the commit.

If an iteration of the stress test fails two cpu beeps (\a\a) are sent, the state of the random number generator is saved, the error output is saved and sent to stdout and then `__debugbreak()` is called. The stress test can be restarted using the failed iteration by passing the state of the random number generator at that time (error_DATE_TIME_state_iteration.txt) as an argument to stresstest.

Likely FAQ
----------


### Text mode or binary mode?

Libcsv handles files in binary mode and by default files are opened in binary mode by my classes. This should be fine for most purposes since libcsv autodetects CR,LF,CRLF as record terminators. However if you have records with multiline data that is not binary then you may prefer the text mode translation. When characters are translated though it can lead to consequences that may not be readily apparent and are usually unintended such as premature termination of a file (eg the EOF character is in a CSV field). I use binary mode for that reason. Also I have done extensive testing only in binary mode. I suggest using binary mode unless you have a really compelling reason otherwise.


### Why two separate classes, one for reading and one for writing?

I think it's the best design. Although both classes have some function names and behaviors that are the same the code is far between.


### No exceptions?

The CSVread/CSVwrite classes do not use exceptions. I want to be able to use this code in exception-free environments. It's possible I could add exceptions as an option at some point but it won't be the default.


### Can CSVread detect headers or comments?

No, CSVread/libcsv cannot auto-detect headers or comments. It parses them the same as any other CSV record. Headers should be CSV records anyway. You can check the parsed fields to see if it's your header. Is there any popular format for CSV comments? I can't see comments as being useful except maybe in text mode. Maybe I'm wrong? I'm open to adding a skip-comments option if someone gives me a good reason for it, but it won't be the default.


### How can I avoid corruption when appending CSV records?

**If there are records already in your stream the end record must already be terminated before you append records.** It is absolutely essential to do that to avoid corruption. CSVwrite appends by default and does NOT check for an unterminated end record already in the stream. A check could be resource intensive and may not even be possible depending on the stream. Therefore the easiest way to address this issue is to ensure that all of your CSV records are always terminated.

You may not be able to ensure that an existing CSV end record is always terminated, maybe due to some third party software. As long as you know what terminator was used --or it is a default terminator of one of CR,LF,CRLF-- you have several options, outlined below.

* If you are certain the existing CSV records are not multiline, are not binary and you can backtrack in the stream then you can back up a byte and check for whatever record terminator is used. The default terminators you could check for are CR,LF,CRLF.

* If your existing CSV records may contain multiline or binary data and you can backtrack in the stream then you would have to rewind to the beginning and use CSVread to read to the end record to determine if it's terminated. *Note that if the terminator character is not CR,LF,CRLF then after rewinding you'll first have to set a custom function that libcsv can call to identify the terminator. That is advanced behavior not covered in my documentation but it is in libcsv's documentation (not included).*

* If you cannot backtrack your stream you can call `bool CSVwrite::WriteTerminator()` after opening the file or associating the stream and if necessary setting your terminator character. If the end record is not terminated then you've just terminated it, and if it was already terminated then you've added an empty separate record which by default are ignored. This method seems sloppy to me, may not be compatible with other CSV parsers and I'd only use it as a last resort.


### How can I disable quoted fields written by CSVwrite?

CSVwrite surrounds all fields in double quotes. For example if you have a record with two fields, apple and orange, the record is written as `"apple","orange"`. Writing fields that aren't quoted doesn't seem like good practice to me. Unquoted fields when read by any CSV parser can lead to parsing errors if the data should have been quoted. CSVread/libcsv can read unquoted fields and will silently recover from parsing errors (unless in strict mode) but still it's not a good idea. I don't think I'm going to add an option to disable quoting but if I do it won't be the default.


### What's an empty record?

A record that has nothing before its terminator. A record that consists of just a field separator --eg a comma `,`-- is *not* considered an empty record by this parser, it is parsed as two empty fields.


### What are the advanced features of libcsv?

Download [libcsv](http://sourceforge.net/projects/libcsv/) and review its FAQ and documentation. They are not included with CSVread/CSVwrite.


### How do you license and can I have access to your source?

CSVread/CSVwrite is free software and it is licensed under the [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/copyleft/gpl.html), a license that will keep it free. [Libcsv](http://sourceforge.net/projects/libcsv/) if used separately without my code is licensed under the [GNU Lesser General Public License version 2.1 (LGPLv2.1)](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html). You may not remove my copyright, Robert Gamble's copyright or the copyright of any contributors under the terms of either license. The source code for CSVread/CSVwrite cannot be used in proprietary software, but you can for example execute a free software application from a proprietary software application. **In any case please review the GPLv3 license, which is designed to protect freedom, not take it away.**

The source can be found on [GitHub](https://github.com/jay/CSV). Since you're reading this maybe you're already there?


### Send me any questions you have

Jay Satiro `<raysatiro$at$yahoo{}com>` and put CSVread/CSVwrite in the subject.


TODO
----

* Add operators >> <<
* Add text and append modes to the stress test
