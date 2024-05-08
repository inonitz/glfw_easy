#include "file.hpp"
#include "base.hpp"
#include "ifcrash.hpp"
#include <filesystem>


bool loadFile(
	const char* path, /* filepath 														   */ 
	size_t* 	size, /* the size of the out array that was pre-allocated.                 */
	char*		out   /* Where to place the file contents (Buffer of Min-Size 'size')      */
) {
	namespace fs = std::filesystem;
	ifcrashfmt(!fs::exists(path), "loadFile() => Couldn't find Path [%s]\n", path);

	FILE*  file;
	size_t fsize = fs::file_size(path);
	if(*size < fsize || out == nullptr) {
		/* A buffer wasn't allocated OR the size of the buffer is too small. */
		*size = fsize;
		return false;
	}
	fsize = fopen_s(&file, path, "rb");
	ifcrashfmt(file == nullptr, "Couldn't get handle to file at Path [%s]\n", path);


	fsize = fread(out, sizeof(u8), *size, file);
	ifcrashdo(fsize != *size, {
		fclose(file); 
		printf("Something went wrong - file size is %llu bytes, read only %llu bytes\n", *size, fsize);
	});


	fsize = fclose(file);
	ifcrashfmt(fsize, "Couldn't close file handle. ERROR CODE: %llu\n", fsize);
	return true;
}