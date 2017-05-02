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
#include <fstream>
#include <iomanip>

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

//similar method as above but for assignment 2 now
void cpplines2 (char* filename) {
   std::string str(filename);
   string substring = str.substr(0,str.length() - 3);
   string strFilename = substring + ".tok";
   bool fileThere = fileExists(strFilename);
   if (fileThere == true){
      remove(strFilename.c_str());
   }
   ofstream output(strFilename.c_str(), ios::out);

   char buffer[1024];
   int a;
   int myFileNum = (int)lexer::lloc.filenr;

   while ((a = yylex())){

      int myFileNum2 = (int)lexer::lloc.filenr;

      if (myFileNum!=myFileNum2){
         myFileNum = (int)lexer::lloc.filenr;
         bzero(buffer, 1024);
         sprintf(buffer, "# %d \"%s\"\n", (int)(lexer::lloc.linenr), 
               (lexer::filename(lexer::lloc.filenr))->c_str());
         output << buffer;
      }

      string tname = parser::get_tname(a);
      
      bzero(buffer, 1024);
      sprintf(buffer, "%d", (int)(lexer::lloc.filenr));
      output << right << setw(3) << buffer;

      bzero(buffer, 1024);
      sprintf(buffer, "%d.", (int)(lexer::lloc.linenr));
      output << right << setw(4) << buffer;

      bzero(buffer, 1024);
      int offset = (int)(lexer::lloc.offset);
      int onesNum = offset%10;
      int tensNum = (offset%100)/10;
      int hundredsNum = (offset%1000)/100;
      sprintf(buffer, "%d%d%d", hundredsNum, tensNum, onesNum );
      output << left << setw(5) << buffer;

      bzero(buffer, 1024);
      sprintf(buffer, "%d", a);
      output << right << setw(3) << buffer;

      bzero(buffer, 1024);
      sprintf(buffer, "  %s", tname.c_str());
      output << left << setw(15) << buffer;

      bzero(buffer, 1024);
      sprintf(buffer, "(%s)\n", yytext);
      output << left << buffer;

      
      /*
      std::cout << lexer::filename << std::endl;
      std::cout << lexer::lloc.filenr << std::endl;
      std::cout << lexer::lloc.offset <<std::endl;
      std::cout << a << std::endl;
      std::cout << tname << std::endl;
      std::cout << yytext << std::endl;
      std::cout << "\n\n" << std::endl;
      */
      //astree myTree; //needs a, string, and something else?
      //call dump method next of astree?
      //astree *myTree = parser::root->adopt_sym(parser::root, a);
      //parser::root->adopt(parser::root, myTree); 
   }

   //parser::root->print(pFile, parser::root, 2);
   //parser::root->printer(); 

   //fclose(pFile);
   DEBUGF('x', "Dumped output into string ADT successfully\n");
}


int main (int argc, char** argv) {
   exec::execname = basename(argv[0]);
   const char* execname = basename (argv[0]);
   int exit_status = EXIT_SUCCESS;
   //for (int argi = 1; argi < argc; ++argi) {

      //int superBreak = -1;     
      int argi = 1;
      int dFlag = -1;
      string dString;
 
      //go through all the options
      int opt;
      yy_flex_debug = 0;
      yydebug = 0;
      while ((opt = getopt(argc, argv, "D:l:@:")) != -1) {
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
               //superBreak = 0;
               break;
            }
         } else if (opt == 'l'){
            yy_flex_debug = 1;
         } else if (opt == 'y'){
            yydebug = 1;
         } else{
            //superBreak = 0;
            break;
         }
         argi++;
      }

      /* 
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
            //eprint_status (command.c_str(), pclose_rc);
            if (pclose_rc != 0) exit_status = EXIT_FAILURE;
         }
      }
      */
      
      char* filename = argv[argi];
      string command;
      if (dFlag == 0){
         command = CPP + " -D" + dString + " " + filename; 
      }else{
         command = CPP + " " + filename;
      }
      yyin = popen (command.c_str(), "r");
      if (yyin == NULL) {
         exit_status = EXIT_FAILURE;
         fprintf (stderr, "%s: %s: %s\n",
                  execname, command.c_str(), strerror (errno));
      }else {
         lexer::newfilename(command);
         cpplines2 (filename);
         int pclose_rc = pclose (yyin);

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


         eprint_status (command.c_str(), pclose_rc);
         if (pclose_rc != 0) exit_status = EXIT_FAILURE;

         //astree::print(stdout, parser::root); 
      }
      

   //}
   return exit_status;
}

