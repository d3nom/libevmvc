/*
MIT License

Copyright (c) 2019 Michel Dénommée

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


// the configured options and settings for libevent-mvc

#ifndef _EVENT_MVC_CONFIGURATION_HEADER_GUARD_H_
#define _EVENT_MVC_CONFIGURATION_HEADER_GUARD_H_

#define EVENT_MVC_VERSION_MAJOR 
#define EVENT_MVC_VERSION_MINOR 

#if _DEBUG
#define CMAKE_COMMAND "/usr/local/bin/cmake"
#define EVENT_MVC_PROJECT_SOURCE_DIR "/home/mdenommee/devel/libevent-mvc"
#define EVENT_MVC_PROJECT_BINARY_DIR "/home/mdenommee/devel/libevent-mvc/build"
#endif

// defines the type of build
/* #undef EVENT_MVC_BUILD_DEBUG */
/* #undef EVENT_MVC_BUILD_TEST */
/* #undef EVENT_MVC_BUILD_SHIPPING */

#if !(defined(EVENT_MVC_BUILD_DEBUG) || defined(EVENT_MVC_BUILD_TEST) || defined(EVENT_MVC_BUILD_SHIPPING))
    #define EVENT_MVC_BUILD_DEBUG 1
#endif

#ifndef EVENT_MVC_BUILD_DEBUG
    #define EVENT_MVC_BUILD_DEBUG 0
#endif
#ifndef EVENT_MVC_BUILD_TEST
    #define EVENT_MVC_BUILD_TEST 0
#endif
#ifndef EVENT_MVC_BUILD_SHIPPING
    #define EVENT_MVC_BUILD_SHIPPING 0
#endif


// arch
// Check windows
#if _WIN32 || _WIN64
    #if _WIN64
        #define ARCH64
        #define PLATFORM_64BITS 1
    #else
        #define ARCH32
        #define PLATFORM_64BITS 0
    #endif
#endif

// Check GCC
#if __GNUC__
    #if __x86_64__ || __ppc64__
        #define ARCH64
        #define PLATFORM_64BITS 1
    #else
        #define ARCH32
        #define PLATFORM_64BITS 0
    #endif
#endif

#if WIN32
    #define EVENT_MVC_FORCEINLINE __forceinline
#elif _GCC || __GNUC__
    #define EVENT_MVC_FORCEINLINE inline __attribute((always_inline))
#endif


/* #undef EVENT_MVC_THREAD_SAFE */

#endif //_EVENT_MVC_CONFIGURATION_HEADER_GUARD_H_
