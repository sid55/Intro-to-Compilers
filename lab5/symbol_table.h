#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "lyutils.h"
#include "auxlib.h"

#include <iostream>
#include <unordered_map>
#include <bitset>

using namespace std;

// These codes are taken from the PDF file provided for
// assignment 4 description.

enum { ATTR_void, ATTR_int, ATTR_null, ATTR_string,
       ATTR_struct, ATTR_array, ATTR_function, ATTR_variable,
       ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval, ATTR_const,
       ATTR_vreg, ATTR_vaddr, ATTR_prototype, ATTR_bitset_size,
     };
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_table = unordered_map<const string*,symbol*>;
using symbol_entry = symbol_table::value_type;

struct symbol {
   const string *type_name;
   const string *lexinfo;
   const string *field_of;
   attr_bitset attributes;
   symbol_table* fields;
   size_t filenr, linenr, offset;
   size_t block_nr;
   vector<symbol*>* parameters;

   ~symbol () {
      // Delete fields if it exists
      if (fields != nullptr)
         for (auto &it: *fields) 
            if (it.second != nullptr) delete it.second;

      // Delete parameters if it exists
      if (parameters != nullptr)
         for (symbol *s: *parameters)
            delete s;
   };
};

class SymbolStack {
   private:
      vector<symbol_table*> symbol_stack;
      symbol_table struct_table;
      size_t next_block;
      vector<size_t> block_count;
      FILE* sym_file;

      symbol * scan (astree *);

      // Helpers
      symbol* createSymbol (astree *);
      symbol_table* getTable ();
      symbol* lookup (const string*, bool local = false);
      void dump_symbol (symbol *, int depth = 0);
      void dump_table ();
      string get_attr (symbol *, bool tree = false, bool ident = false);
      size_t get_type_attr (symbol *);
      void enterBlock ();
      void leaveBlock ();
      int getScope ();
   public:
      SymbolStack (astree*, FILE*);
};

#endif
