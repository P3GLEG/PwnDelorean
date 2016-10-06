#ifndef _UTIL_
#define _UTIL_
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>


#define SUCCESS 0
#define FAILURE -1


#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */



static bool yes_or_no(const char* message)
{
    std::string line;
    std::cout << message << std::endl;
    if (std::getline(std::cin, line))
    {
        if(line[0] == 'y' || line[0] == 'y'){
            return true;
        }else{
            return false;
        }
    }
    return false;
}

static bool check_filepath_backslash_required(const char* path){
	if(path[strlen(path) - 1] == '/')
		return false;
     else
		return true;

}
#endif
