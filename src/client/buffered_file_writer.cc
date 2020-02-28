#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <stdlib.h>

#if defined(__linux__) && __linux__
#include "third_party/lss/linux_syscall_support.h"
#endif

#include "client/buffered_file_writer.h"

#if 0
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#else
#define debug_print(fmt, ...)
#endif

namespace google_breakpad {
	
	BufferedFileWriter::BufferedFileWriter(ssize_t length) :
		length_(length),
		latestFd_(-1),
		currentSeek_(0),
		bufferSeekStart_(0),
		farthestBufferWrite_(0)
	{
		assert(length_ > 0);
		buffer_ = malloc(length);
	}
	
	BufferedFileWriter::~BufferedFileWriter()
	{
		free(buffer_);
	}
	
	ssize_t BufferedFileWriter::Write(int fd, const void *buf, size_t count)
	{
		assert(latestFd_ == -1 || latestFd_ == fd);
		
		latestFd_ = fd;
		
		char* curBuf = (char*)buf;
		
		size_t originalCount = count;
		
		while(count > 0)
		{
			off_t seekInBuffer = (currentSeek_ - bufferSeekStart_);
			
			// remaining byte amount in buffer from currentSeek_!
			// big difference from farthestBufferWrite_!!!!
			size_t remainingSize = static_cast<size_t>(length_ - seekInBuffer);
			
			assert(farthestBufferWrite_ >= 0);
			
			debug_print("BufferedFileWriter::write() - remainingSize = %lu | count = %lu | farthestBufferWrite_ = %lld\n", remainingSize, count, farthestBufferWrite_);			
			if(remainingSize < count )
			{
				if(remainingSize > 0)
				{
					memcpy((void*)(((const char*)buffer_) + seekInBuffer), curBuf, remainingSize);
				}
				
				if(!WriteBufferToFile(latestFd_, bufferSeekStart_, length_))
				{
					return 0;
				}
				
				count -= remainingSize;
				curBuf += remainingSize;
				bufferSeekStart_ = bufferSeekStart_ + length_;
				farthestBufferWrite_ = 0;
				
				currentSeek_ += remainingSize;
				assert(currentSeek_ == bufferSeekStart_);
			} else
			{
				memcpy((void*)(((const char*)buffer_) + seekInBuffer), curBuf, count);
				currentSeek_ += count;
				if(seekInBuffer + count > static_cast<size_t>(farthestBufferWrite_))
				{
					farthestBufferWrite_ = seekInBuffer + count;
				}
				count = 0;
			}
			assert(farthestBufferWrite_ <= length_);
		}
		
		debug_print("BufferedFileWriter::write() - bufferSeekStart_ = %lld | currentSeek_ = %lld | farthestBufferWrite_ = %lld\n", 
									bufferSeekStart_, currentSeek_, farthestBufferWrite_);	
		
		return originalCount;
	}
	
	bool BufferedFileWriter::WriteBufferToFile(int fd, off_t offset, size_t amount)
	{
		debug_print("BufferedFileWriter::WriteBufferToFile() - offset = %lld | amount = %lu\n", offset, amount);	
		  // Seek and write the data
		#if defined(__linux__) && __linux__
		  if (sys_lseek(fd, offset, SEEK_SET) == static_cast<off_t>(offset)) {
		    if (sys_write(fd, buffer_, amount) == amount) {
		      return true;
		    }
		  }
		#else
		  if (::lseek(fd, offset, SEEK_SET) == static_cast<off_t>(offset)) {
		    if (::write(fd, buffer_, amount) == static_cast<ssize_t>(amount)) {
		      return true;
		    }
		  }
		#endif
		
		return false;
	}
	
	off_t BufferedFileWriter::Lseek(int fd, off_t offset, int whence)
	{
		assert(latestFd_ == -1 || latestFd_ == fd);
		
		debug_print("BufferedFileWriter::lseek() - offset = %lld | bufferSeekStart_ = %lld | currentSeek_ = %lld | farthestBufferWrite_ = %lld\n", 
						offset, bufferSeekStart_, currentSeek_, farthestBufferWrite_);	
		
		latestFd_ = fd;
		
		assert(whence == SEEK_SET);
				
		if(offset < bufferSeekStart_ || offset > currentSeek_)
		{
			if(currentSeek_ > bufferSeekStart_ && !WriteBufferToFile(latestFd_, bufferSeekStart_, static_cast<size_t>(farthestBufferWrite_)))
			{
				return -1;
			}
			farthestBufferWrite_ = 0;
			bufferSeekStart_ = offset;
		}
		 
		currentSeek_ = offset;
		
		return currentSeek_;
	}
	
	bool BufferedFileWriter::Flush()
	{
		debug_print("BufferedFileWriter::flush() - bufferSeekStart_ = %lld | farthestBufferWrite_ = %lld\n", bufferSeekStart_, farthestBufferWrite_);
		if(!WriteBufferToFile(latestFd_, bufferSeekStart_, static_cast<size_t>(farthestBufferWrite_)))
		{
			return false;
		}
		farthestBufferWrite_ = 0;
		bufferSeekStart_ = currentSeek_;
		return true;
	}
}
