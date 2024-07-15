#ifndef __UTIL_LOAD_FILE__
#define __UTIL_LOAD_FILE__


namespace util {


/* 
	Recommended to pass size = 0 and out = nullptr to find the size of the buffer.
	When you know the size, allocate an appropriate buffer and pass the args to the function.
	Then the function will load the File properly. 
*/
bool loadFile(
	const char* 		path, /* filepath 														   */ 
	unsigned long long* size, /* the size of the out array that was pre-allocated.                 */
	char*				out   /* Where to place the file contents (Buffer of Min-Size 'size')      */
);


}


#endif