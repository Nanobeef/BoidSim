#include "basic.h"

#include "alg.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

u8* read_entire_binary_file(const char* path, Arena* arena)
{
	s32 fd = open(path, O_RDONLY);		
	if(fd == -1)
	{
		print("Failed to open file: %cs\n", path);
		return 0;
	}
	struct stat file_stats;

	if(fstat(fd, &file_stats) == -1)
	{
		print("Failed fetting file stats for OPEN file: %cs\n", path);
		return 0;
	}
	u64 file_size = file_stats.st_size;
	void* buffer = allocate_buffer(u8, file_size, arena);
	u64 bytes_read = read(fd, buffer, file_size);
	if(file_size != bytes_read)
	{
		print("Failed to read the entire file length: %cs\n", path);
	}
	close(fd);
	return buffer;

}
