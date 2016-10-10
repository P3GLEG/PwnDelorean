/*
 * @Author Paul Ganea
 */
extern "C" {
    #include "../deps/libgit2/include/git2.h"
}

#include <map>
#include <fstream>
#include <re2.h>
#include <re2/set.h>
#include "gitengine.h"
#include "util.h"
#include "filesystemengine.h"
#define ARGS_INFO_INIT { argc, argv, 0 }

const char *title = 
"########  ##      ## ##    ## ########  ######## ##        #######  ########  ########    ###    ##    ## \n"
"##     ## ##  ##  ## ###   ## ##     ## ##       ##       ##     ## ##     ## ##         ## ##   ###   ## \n"
"##     ## ##  ##  ## ####  ## ##     ## ##       ##       ##     ## ##     ## ##        ##   ##  ####  ## \n"
"########  ##  ##  ## ## ## ## ##     ## ######   ##       ##     ## ########  ######   ##     ## ## ## ## \n"
"##        ##  ##  ## ##  #### ##     ## ##       ##       ##     ## ##   ##   ##       ######### ##  #### \n"
"##        ##  ##  ## ##   ### ##     ## ##       ##       ##     ## ##    ##  ##       ##     ## ##   ### \n"
"##         ###  ###  ##    ## ########  ######## ########  #######  ##     ## ######## ##     ## ##    ##\n";

const char *delorean = 
 "           __---~~~~--__                      __--~~~~---__\n"
 "            `\\---~~~~~~~~\\\\                    //~~~~~~~~---/'  \n"
 "              \\/~~~~~~~~~\\||                  ||/~~~~~~~~~\\/ \n"
 "                          `\\\\                //'\n"
 "                            `\\\\            //'\n"
 "                              ||          ||      \n"
 "                    ______--~~~~~~~~~~~~~~~~~~--______              \n"
 "               ___ // _-~                        ~-_ \\\\ ___  \n"
 "              `\\__)\\/~                              ~\\/(__/'          \n"
 "               _--`-___                            ___-'--_        \n"
 "             /~     `\\ ~~~~~~~~------------~~~~~~~~ /'     ~\\        \n"
 "            /|        `\\         ________         /'        |\\     \n"
 "           | `\\   ______`\\_      \\------/      _/'______   /' |          \n"
 "           |   `\\_~-_____\\ ~-________________-~ /_____-~_/'   |  \n"
 "           `.     ~-__________________________________-~     .'       \n"
 "            `.      [_______/------|~~|------\\_______]      .'\n"
 "             `\\--___((____)(________\\/________)(____))___--/'           \n"
 "              |>>>>>>||                            ||<<<<<<|\n"
 "              `\\<<<<>>>>/'                       `\\<<<<>>>>/'\n";

 const char *argument_list = 
"\t-o Clone Directory\n"
"\t-l Local Git Directory\n"
"\t-u Remote Git Repo URL\n"
"\t-f Scan Filesystem Location\n"
"\t-iL List of Remote Git Repos\n"
"\t-v Log Verbosity 1-4\n"

;

struct opts {
    const char *filesystem_dir;
	const char *clone_dir;
    const char *repo_url;
    const char *local_repo; 
    const char *input_file;
    const char *verbosity;
	int action;
	int verbose;
};

struct args_info {
	int    argc;
	char **argv;
	int    pos;
};

void parse_opts(struct opts *o, int argc, char *argv[]);

void usage(const char *message, const char *arg)
{
    if (message && arg)
	    std::cout << RED <<  "+ ERROR: " << message << ": " <<  arg << "\n\n" << RESET ;
	else if (message){
        std::cout << RED << "+ ERROR: " <<  message << "\n\n" << RESET;
    }
        std::cout << WHITE << "Usage: pwndelorean -u https://github.com/pegleg2060/PwnDelorean.git -o /tmp/repo \n" ;
        std::cout << WHITE << argument_list << RESET;

}

size_t is_prefixed(const char *str, const char *pfx){
	size_t len = strlen(pfx);
	return strncmp(str, pfx, len) ? 0 : len;
}

int match_str_arg(const char **out, struct args_info *args, const char *opt){
	const char *found = args->argv[args->pos];
	size_t len = is_prefixed(found, opt);

	if (!len){
		return 0;
    }
	if (!found[len]) {
		if (args->pos + 1 == args->argc)
			LOG_ERROR <<"expected value following argument" << opt;
		args->pos += 1;
		*out = args->argv[args->pos];
		return 1;
	}

	if (found[len] == '=') {
		*out = found + len + 1;
		return 1;
	}
	return 0;
}

void parse_opts(struct opts *o, int argc, char *argv[]){
	struct args_info args = ARGS_INFO_INIT;
	for (args.pos = 1; args.pos < argc; ++args.pos) {
		char *a = argv[args.pos];
        if (!strcmp(a, "-o")){
            o->clone_dir = argv[++args.pos];
        }
        else if (!strcmp(a, "-f")){
            o->filesystem_dir = argv[++args.pos];
        }
        else if (!strcmp(a, "-l")){
            o->local_repo = argv[++args.pos];
        }
        else if (!strcmp(a, "-u")){
            o->repo_url = argv[++args.pos];
        }
        else if (!strcmp(a, "-iL")){
            o->input_file = argv[++args.pos];
        }
        else if (!strcmp(a, "-v")){
            o->verbosity = argv[++args.pos];
        }
		else if (!strcmp(a, "--help") || !strcmp(a, "-h")){
			usage(NULL, NULL);
        }
		else if (!match_str_arg(&o->clone_dir, &args, "--rawr")){
			usage("Unknown option", a);
        }
	}
    if(argc == 1) {
        usage(NULL,NULL);
        exit(FAILURE);
    }

    else if ((strcmp(o->repo_url, "") == 0 ^ strcmp(o->local_repo,"") != 0) ^ strcmp(o->filesystem_dir,"") !=0)  {
        usage("Unable to identify what you want me to scan",NULL);
    }

}

int init(void) {
    static plog::RollingFileAppender<plog::CsvFormatter> fileAppender("debug.txt", 100000000, 3);
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::info, &consoleAppender).addAppender(&fileAppender);
    return SUCCESS;
}

int main(int argc, char *argv[]) {
    std::cout << CYAN << title << "\n";
    std::cout << delorean << "\n";
    std::cout << "---------------------------------------------------------------------------" << "\n" <<RESET;
    if (init() != 0) {
        LOG_ERROR << "Failed to initialize the program";
        exit(FAILURE);
    }
	struct opts opts = { "", "", "","", "", "", 0, 0, };
	parse_opts(&opts, argc, argv);
    if(strcmp(opts.verbosity,"") !=0){
        plog::get()->setMaxSeverity(plog::debug);
    }
    if(strcmp(opts.filesystem_dir,"") != 0){
        FilesystemEngine filesystem;
        filesystem.start(opts.filesystem_dir);
    }else if(strcmp(opts.local_repo,"") != 0) {
        GitEngine git;
        LOG_DEBUG << "Local Repo location set to : " << opts.repo_url;
        git.local_start(opts.local_repo);
    }else{
        GitEngine git;
        if (strcmp(opts.clone_dir, "") == 0) {
            usage("Please provide a directory to clone to", NULL);
            exit(FAILURE);
        }
        LOG_DEBUG << "Repo URL set to : " << opts.repo_url;
        LOG_DEBUG << "Clone Directory set to : " << opts.clone_dir;
        git.remote_start(opts.repo_url, opts.clone_dir);
    }
    return SUCCESS;

}

