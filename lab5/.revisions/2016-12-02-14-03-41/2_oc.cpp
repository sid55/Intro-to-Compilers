// Dat Nguyen
// 09-30-2016

#include <unistd.h>
#include <iostream>
#include <cstring>
#include <libgen.h>

#include "string_set.h"
#include "lyutils.h"
#include "symbol_table.h"
#include "emit.h"

using namespace std;

// CPP program
const string CPP = "/usr/bin/cpp";
string CPP_option = "";
constexpr size_t LINESIZE = 1024;

FILE* tok_file;

// Scanning and setting options
void scan_option (int argc, char **argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "ly@:D:");
      if (option == EOF) break;
      switch (option) {
         case 'l':
            // Turnon yydebug
            yy_flex_debug = 1;
            break;
         case 'y':
            // Turn on flex_debug
            yydebug = 1;
            break;
         case '@':
            // Setting debug flags for DEBUGF
            set_debugflags(optarg);
            break;
         case 'D':
            // Setting CPP program's argument
            CPP_option = CPP_option + " -D" + optarg;
            break;
         default:
            break;
      }
   }
}

int main (int argc, char** argv) {
   // Get the program's name
   exec::execname = basename (argv[0]);
   // Scan the options and do some stuffs with it
   // Make sure to set yy_flex_debug (global) to 0
   yy_flex_debug = 0;
   scan_option(argc, argv);
   // Get the actual "oc" file that is needed to be run
   char* filename = argv[argc-1];

   // Now, try to dump the set to a file
   string str(basename(filename));
   size_t found = str.find_last_of(".");

   // Check if it's ".oc" file
   if (found == string::npos || 
         str.substr(found, str.length() - 1) != ".oc") {
      fprintf (stderr, "%s: not found\n", str.c_str());
      return EXIT_FAILURE;
   }

   string basefilename = str.substr(0, found);
   string name = basefilename + ".tok";
   string sym_file_name = basefilename + ".sym";
   string oil_file_name = basefilename + ".oil";

   // Make up a command for "CPP" including the options
   string command = CPP + CPP_option + " " + filename;

   // Running the command
   yyin = popen (command.c_str(), "r");
   if (yyin == NULL) {
      fprintf (stderr, "%s: %s: %s\n",
               exec::execname.c_str(), 
               command.c_str(), 
               strerror (errno));
      return EXIT_FAILURE;
   }else {
      // Open tok_file name
      tok_file = fopen(name.c_str(), "w");
      FILE *sym_file = fopen(sym_file_name.c_str(), "w");
      FILE *oil_file = fopen (oil_file_name.c_str(), "w");

      // Process the outputs of "CPP" program line-by-line
      yyparse();

      SymbolStack stack(parser::root, sym_file);

      Emit em (oil_file);
      em.emit (parser::root);

      // Close sym file
      int sym_close = fclose (sym_file);
      eprint_status (sym_file_name.c_str(), sym_close);
      if (sym_close != 0) return EXIT_FAILURE;

      // Close sym file
      int oil_close = fclose (oil_file);
      eprint_status (oil_file_name.c_str(), oil_close);
      if (oil_close != 0) return EXIT_FAILURE;

      string ast_name = basefilename + ".ast";
      FILE *ast_file = fopen (ast_name.c_str(), "w");
      parser::root->dump_tree (ast_file, 0);
      int ast_close = fclose (ast_file);
      eprint_status (ast_name.c_str(), ast_close);
      if (ast_close != 0) return EXIT_FAILURE;

      int tok_close = fclose(tok_file);
      eprint_status (name.c_str(), tok_close);
      if (tok_close != 0) return EXIT_FAILURE;

      // Close yyin
      int pclose_rc = pclose (yyin);
      eprint_status (command.c_str(), pclose_rc);
      if (pclose_rc != 0) return EXIT_FAILURE;

      name = basefilename + ".str";
      FILE *str_file = fopen(name.c_str(), "w");
      string_set::dump(str_file);
      int fclose_rc = fclose(str_file);
      eprint_status (name.c_str(), fclose_rc);
      if (fclose_rc != 0) return EXIT_FAILURE;
   }
   
   destroy (parser::root);
   return EXIT_SUCCESS;
}
