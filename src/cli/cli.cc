/** @file cli.cc
 * @brief Command line interface.
 */
/* Copyright (c) 2010 Richard Boulton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <config.h>

#include "cli.h"

#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <xapian.h>

RestPose::CliOptions::CliOptions()
	: datadir(),
	  action(ACT_DEFAULT),
	  port(7777),
	  pedantic(false),
	  dbname(),
	  searchfiles(),
	  languages(),
	  mongo_import()
{
}

int
RestPose::CliOptions::parse(const char * progname, int argc, char * const* argv)
{
    static const struct option longopts[] = {
	{ "help",       no_argument,            NULL, 'h' },
	{ "version",    no_argument,            NULL, 'v' },
	{ "datadir",    required_argument,      NULL, 'd' },
	{ "action",     required_argument,      NULL, 'a' },

	{ "port",       required_argument,      NULL, 'p' },
	{ "pedantic",   no_argument,            NULL, 'P' },

	{ "dbname",     required_argument,      NULL, 'n' },
	{ "searchfile", required_argument,      NULL, 'f' },

	{ "mongo_import", required_argument,    NULL, 'm' },
	{ 0, 0, NULL, 0 }
    };

    int getopt_ret;
    while ((getopt_ret = getopt_long(argc, argv, "hvd:a:p:n:s:i:f:Im:l:",
					 longopts, NULL)) != -1) {
	switch (getopt_ret) {
	    case 'h':
		std::cout << progname << " - RESTfull search server\n\n"
"Usage: " << progname << " [OPTIONS]\n\n"
"Options:\n"
"  -h, --help             display usage help\n"
"  -v, --version          display version number\n"
"  -d, --datadir=DATADIR  directory to store data in\n"
"  -a, --action=ACTION    action: one of:\n"
"                         \"serve\" (default) to run a server\n"
"                         \"search\" to perform a command immediately\n"
"                         \"train\" to train a classifier\n"
"\n"
"Options for \"serve\" action\n"
"  -p, --port=PORT        port number to listen on\n"
"  -P, --pedantic         specify to be pedantic about request handling; use\n"
"                         for testing clients.\n"
"  -m, --mongo_import=CFG start a mongo importer, with some JSON config\n"
"\n"
"Options for \"search\" action\n"
"  -n, --dbname=DBNAME    name of database for \"cmd\" action\n"
"  -f, --searchfile=PATH  perform a search stored in a file\n"
"                         (or - to read from stdin)\n"
"\n"
"Options for \"train\" action\n"
"  -l, --lang=LANGUAGE    a language to train\n"
"\n";
		return 0;
	    case 'v':
		std::cout << progname << " version: " << PACKAGE_VERSION << "\n"
			     "xapian version: " << Xapian::version_string() <<
			     "\n";
		return 0;
	    case 'd':
		datadir = optarg;
		break;
            case 'a':
		if (action != ACT_DEFAULT) {
		    std::cerr << progname << ": action must only be specified once" << std::endl;
		    return 1;
		}
		if (strcmp(optarg, "server") == 0) {
		    action = ACT_SERVE;
		} else if (strcmp(optarg, "search") == 0) {
		    action = ACT_SEARCH;
		} else if (strcmp(optarg, "train") == 0) {
		    action = ACT_TRAIN;
		} else {
		    std::cerr << progname << ": invalid action specified" << std::endl;
		    return 1;
		}
		break;
	    case 'p':
		port = atoi(optarg);
		break;
	    case 'P':
		pedantic = true;
		break;
	    case 'n':
		dbname = optarg;
		break;
	    case 'f':
		searchfiles.push_back(optarg);
		break;
	    case 'm':
		mongo_import = optarg;
		break;
	    case 'l':
		languages.push_back(optarg);
		break;
	    case ':':
		return 1;
	    case '?':
		return 1;
	}
    }

    if (optind < argc) {
	std::cerr << progname << ": excess parameters" << std::endl;
	return 1;
    }

    if (action == ACT_DEFAULT) {
	action = ACT_SERVE;
    }
    if (datadir.empty()) {
	datadir = "rspdbs";
    }
    return 0;
}
