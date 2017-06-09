// $Id: astree.cpp,v 1.8 2016-09-21 17:13:03-07 - - $

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"

astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = string_set::intern (info);
   // vector defaults to empty -- no children
}

astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2) {
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
   symbol = symbol_;
   return adopt (child);
}


void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd) {%zd} %s",
            parser::get_tname (symbol), lexinfo->c_str(),
            lloc.filenr, lloc.linenr, lloc.offset,
            block_nr, attributes.c_str());
}

void astree::dump_tree (FILE* outfile, int depth) {
   if (depth == 0)
      fprintf (outfile, "|");
   for (int i = 1; i <= depth; i++)
      fprintf (outfile, "|%*s", 2, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (NULL);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   fprintf (outfile, "%4zd", tree->lloc.filenr);
   fprintf (outfile, "%4zd.%.3zd %4d  %-14s (%s)\n",
            tree->lloc.linenr, tree->lloc.offset, tree->symbol, 
            parser::get_tname (tree->symbol), tree->lexinfo->c_str());
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void astree::print (astree* tree, int depth) {
   printf ("Break (depth: %d)\n", depth);
   printf ("%4zd", tree->lloc.filenr);
   printf ("%4zd.%.3zd %4d  %-14s (%s)\n",
            tree->lloc.linenr, tree->lloc.offset, tree->symbol, 
            parser::get_tname (tree->symbol), tree->lexinfo->c_str());
   //for (astree* child: tree->children) {
   //   astree::print (child, depth + 1);
   //}
}

void destroy (astree* tree1, astree* tree2) {
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}
