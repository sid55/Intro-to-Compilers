// This class is to be declared in oc.cpp main()
// It will try to emit the parser::root into
// an *.oil file
#ifndef EMIT_H
#define EMIT_H

#include "astree.h"
#include "lyutils.h"

// These are for emit_print()
#include "stdio.h"
#include "stdarg.h"

class Emit {
   private:
      FILE* oil_file; 

      // These are emit helpers
      void emit_struct (astree *);
      void emit_string (astree *);
      void emit_variables (astree *);
      void emit_function (astree *);
      string emit_rec (astree *, bool right = false);

      // Registers
      string vreg(astree*);
      // Types
      string gettype (astree *);

      // Handle printing (either to file or stdout)
      void emit_print (const char *, ...);
   public:
      Emit (FILE*);
      void emit (astree *);
};

#endif
