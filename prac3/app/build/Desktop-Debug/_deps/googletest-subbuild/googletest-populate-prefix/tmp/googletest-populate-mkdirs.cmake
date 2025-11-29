# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-src"
  "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-build"
  "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-subbuild/googletest-populate-prefix"
  "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-subbuild/googletest-populate-prefix/tmp"
  "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
  "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-subbuild/googletest-populate-prefix/src"
  "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/alex/Desktop/prac/prac3/app/build/Desktop-Debug/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
