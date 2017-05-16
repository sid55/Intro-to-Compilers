// Utility Code provided by Wesley Mackey

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
using namespace std;

#include "auxlib.h"

struct location {
   size_t filenr;
   size_t linenr;
   size_t offset;
};

struct astree {

   // Fields.
   int symbol;               // token code
   location lloc;            // source location
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node

   // Functions.
   astree (int symbol, const location&, const char* lexinfo);
   ~astree();

   // nullptr switch
   astree* adopt (astree* child1, astree* child2 = 0);
   astree* adopt_sym (astree* child, int symbol);
   astree* swap_sym (astree* tree, int symbol);
   void dump_node (FILE*);
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
};


// nullptr switch
void destroy (astree* tree1, astree* tree2 = 0);

void errllocprintf (const location&, const char* format, const char*);

#endif
