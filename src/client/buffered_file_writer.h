#ifndef CLIENT_BUFFERED_FILE_WRITER_H__
#define CLIENT_BUFFERED_FILE_WRITER_H__

#include <sys/types.h>

namespace google_breakpad {

	class BufferedFileWriter {
	public:
		
		BufferedFileWriter(ssize_t length);
		
		~BufferedFileWriter();
		
		ssize_t Write(int fd, const void *buf, size_t count);
		off_t Lseek(int fd, off_t offset, int whence);
		
		bool Flush();

	private:
		void* buffer_;
		ssize_t length_;
		
		off_t currentSeek_;
		
		off_t bufferSeekStart_;
		
		off_t farthestBufferWrite_;
		
		int latestFd_;
		
		bool WriteBufferToFile(int fd, off_t offset, size_t amount);
	};
}

#endif
