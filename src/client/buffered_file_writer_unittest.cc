// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: dtorres@kabaminc.com (Dan Waylonis)

/*
 g++ -I../ buffered_file_writer.cc \
 buffered_file_writer_unittest.cc \
 -o buffered_file_writer_unittest
 */

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>


#include "buffered_file_writer.h"

using google_breakpad::BufferedFileWriter;

#define ASSERT_TRUE(cond) \
if (!(cond)) { \
  fprintf(stderr, "FAILED: %s at %s:%d\n", #cond, __FILE__, __LINE__); \
    return false; \
}

#define ASSERT_EQ(e1, e2) ASSERT_TRUE((e1) == (e2))
#define ASSERT_NE(e1, e2) ASSERT_TRUE((e1) != (e2))

#define STRING_TO_WRITE "Writing this repeatedly to a file...   "
#define BUFFER_SIZE     256     

static bool WriteFile(const char *path, int count) {
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  ASSERT_NE(fd, -1);
  
  BufferedFileWriter writer(BUFFER_SIZE);
  
  int strLen = strlen(STRING_TO_WRITE);
  while(count > 0)
  {
    writer.Write(fd, STRING_TO_WRITE, strLen);
    count--;
  }
  
  ASSERT_TRUE(writer.Flush());

  return close(fd) == 0;
}

static bool WriteFileWithSeek(const char *path, int count) {
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  ASSERT_NE(fd, -1);
  
  char buffer[BUFFER_SIZE];
  
  BufferedFileWriter writer(BUFFER_SIZE);
  
  int originalCount = count;
  int strLen = strlen(STRING_TO_WRITE);
  while(count > 0)
  {
    writer.Write(fd, STRING_TO_WRITE, strLen);
    count--;
  }
  
  count = originalCount;

  off_t toSeek = 0;
  while(count > 0)
  {
    printf("Count = %d\n", count);

    //off_t toSeek = ((((originalCount - count)*101) / 701) % originalCount)*strLen;
    toSeek += (3 - ((toSeek * 701) % 5)) * strLen;
    if(toSeek < 0) toSeek = 0;
    //printf("%d\n", (((originalCount - count)*101) / 701));
    writer.Lseek(fd, toSeek, SEEK_SET);
    writer.Write(fd, STRING_TO_WRITE, strLen);
    
    count--;
  }
  
  ASSERT_TRUE(writer.Flush());

  return close(fd) == 0;
}

static bool VerifyFile(const char *path, int count) {
  
  size_t expected_byte_count = strlen(STRING_TO_WRITE) * count;
  printf("expected_byte_count = %lu\n", expected_byte_count);
  int fd = open(path, O_RDONLY, 0600);
  void *buffer = malloc(expected_byte_count);
  ASSERT_NE(fd, -1);
  ASSERT_TRUE(buffer);
  ASSERT_EQ(read(fd, buffer, expected_byte_count), 
            static_cast<ssize_t>(expected_byte_count));

  bool hasFailed = false;
  char *b1;
  b1 = reinterpret_cast<char*>(buffer);
  while(count > 0 && !hasFailed)
  {
    char *b2 = (char*)(STRING_TO_WRITE);
    while(*b2 == *b1 && *b2 != 0) { b1++; b2++; }
    
    hasFailed = *b2 != 0 && b1 != (reinterpret_cast<char*>(buffer) + expected_byte_count);
    
    count--;
  }
  
  printf("%d\n", static_cast<int>(b1 - (char*)buffer));
  
  ASSERT_TRUE(!hasFailed);
  
  return !hasFailed;
}

static bool RunTests() {
  const char *path = "/tmp/buffered_file_writer_unittest.dmp";
  
  int count = 10;
  printf("\nStarting test with count = %d\n\n", count);
  ASSERT_TRUE(WriteFile(path, count));
  ASSERT_TRUE(VerifyFile(path, count));
  
  count = 100;
  printf("\nStarting test with count = %d\n\n", count);
  ASSERT_TRUE(WriteFile(path, count));
  ASSERT_TRUE(VerifyFile(path, count));
  
  printf("\nStarting seek test with count = %d\n\n", count);
  ASSERT_TRUE(WriteFileWithSeek(path, count));
  ASSERT_TRUE(VerifyFile(path, count));
  
  unlink(path);
  return true;
}

extern "C" int main(int argc, const char *argv[]) {
  return RunTests() ? 0 : 1;
}
