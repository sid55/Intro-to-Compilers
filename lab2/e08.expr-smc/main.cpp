// $Id: cppstrtok.cpp,v 1.1 2017-04-17 14:54:52-07 - - $

// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.
#include <vector>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "string_set.h"
#include "auxlib.h"
#include "astree.h"
#include "lyutils.h"

const string CPP = "/usr/bin/cpp -nostdinc";
//constexpr size_t LINESIZE = 1024;
int LINESIZE = 1024;

// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
   DEBUGF('x', "Successfully makes last char null string\n");
}

//check if file exists or not
bool fileExists(const std::string& filename)
{
   DEBUGF('x', "Check if filename.str already exists or not\n");
   struct stat buf;
   if (stat(filename.c_str(), &buf) != -1)
   {
      return true;
   }
   return false;
}

// Run cpp against the lines of the file.
void cpplines (FILE* pipe, char* filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      //printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
      int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, inputname);
      if (sscanf_rc == 2) {
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         DEBUGF('x', "token %d.%d: [%s]\n",
                 linenr, tokenct, token);
         string_set::intern(token); 
      }
      ++linenr;
   }
   FILE *pFile;
   std::string str(filename);
   string substring = str.substr(0,str.length() - 3);
   string strFilename = substring + ".str";
   bool fileThere = fileExists(strFilename);
   if (fileThere == true){
      remove(strFilename.c_str());
   }
   pFile = fopen(strFilename.c_str(),"w");
   string_set::dump(pFile);
   fclose(pFile);
   DEBUGF('x', "Dumped output into string ADT successfully\n");
}

int main (int argc, char** argv) {
   exec::execname = basename(argv[0]);
   const char* execname = basename (argv[0]);
   int exit_status = EXIT_SUCCESS;
   //for (int argi = 1; argi < argc; ++argi) {

      int superBreak = -1;     
      int argi = 1;
      int dFlag = -1;
      string dString;
 
      //go through all the options
      int opt;
      while ((opt = getopt(argc, argv, "D:@:")) != -1) {
         if (opt == 'D') {
            std::string optarg2(optarg);
            dString = optarg2;
            dFlag = 0;
         } else if (opt == '@') {
            std::string optarg2(optarg);
            if (optarg2.compare("x") == 0){
               set_debugflags("x");
            } else if (optarg2.compare("z") == 0){
               set_debugflags("z");
            } else if (optarg2.compare("a") == 0){
               //CUSTOM FLAG HERE?
            } else{
               superBreak = 0;
               break;
            }
         } else if (opt == 'l'){

         } else if (opt == 'y'){

         } else{
            superBreak = 0;
            break;
         }
         argi++;
      }

      if (superBreak == 0){
         exit_status = EXIT_FAILURE;
         //PRINT => print error message here for invalid option
      }else{
         char* filename = argv[argi];
         string command;
         if (dFlag == 0){
            command = CPP + " -D" + dString + " " + filename; 
         }else{
            command = CPP + " " + filename;
         }
         FILE* pipe = popen (command.c_str(), "r");
         if (pipe == NULL) {
            exit_status = EXIT_FAILURE;
            fprintf (stderr, "%s: %s: %s\n",
                     execname, command.c_str(), strerror (errno));
         }else {
            cpplines (pipe, filename);
            int pclose_rc = pclose (pipe);
            eprint_status (command.c_str(), pclose_rc);
            if (pclose_rc != 0) exit_status = EXIT_FAILURE;
         }
      }
   //}
   return exit_status;
}

