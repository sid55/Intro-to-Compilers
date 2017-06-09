#include "symbol_table.h"

/* SymbolStack implementations */
SymbolStack::SymbolStack (astree* tree, FILE* outfile) {
   next_block = 1;

   // Create global table
   symbol_stack.push_back (new symbol_table);

   sym_file = outfile;
   // Do traversal here
   scan (tree);
}
symbol* SymbolStack::createSymbol (astree *node) {
   symbol *sym = new symbol;
   sym->lexinfo = node->lexinfo;
   sym->fields = nullptr;
   sym->linenr = node->lloc.linenr;
   sym->filenr = node->lloc.filenr;
   sym->offset = node->lloc.offset;
   sym->parameters = nullptr;
   sym->block_nr = next_block - 1;
   node->block_nr = sym->block_nr;
   return sym;
}

symbol * SymbolStack::scan (astree *node) {
   //astree::print (node);
   symbol *sym = nullptr;

   if (node == nullptr) return nullptr;

   switch (node->symbol) {
      case TOK_EQ:
      case TOK_NE:
      case TOK_LT:
      case TOK_LE:
      case TOK_GT:
      case TOK_GE: {
         sym = createSymbol (node);
         sym->attributes.set(ATTR_vreg);
         sym->attributes.set(ATTR_int);
         
         symbol *child1 = scan (node->children[0]);
         symbol *child2 = scan (node->children[1]);

         if ((child1 && child2) &&
             get_type_attr(child1) != get_type_attr(child2))
            errprintf("Types are not the same: %s and %s\n",
                     child1->lexinfo->c_str(),
                     child2->lexinfo->c_str());

         // Set the node
         node->attributes = get_attr (sym, true);
         break;
      }
      case '=': {
         sym = createSymbol (node);
         sym->attributes.set(ATTR_vreg);
         sym->attributes.set(ATTR_int);

         symbol *child1 = scan (node->children[0]);
         symbol *child2 = scan (node->children[1]);
         if ((child1 && child2) && 
             (get_type_attr(child1) != get_type_attr(child2)))
             errprintf ("Invalid type: %s or %s\n",
                        child1->lexinfo->c_str(),
                        child2->lexinfo->c_str());

         node->attributes = get_attr (sym, true);
         break;
      }
      case '+':
      case '-':
      case '*':
      case '/':
      case '%': {
         sym = createSymbol (node);
         sym->attributes.set(ATTR_vreg);
         sym->attributes.set(ATTR_int);

         symbol *child1 = scan (node->children[0]);
         symbol *child2 = scan (node->children[1]);
         if ((child1 && child2) && 
             (get_type_attr(child1) != ATTR_int ||
             get_type_attr(child2) != ATTR_int))
             errprintf ("Invalid type: %s or %s\n",
                        child1->lexinfo->c_str(),
                        child2->lexinfo->c_str());

         node->attributes = get_attr (sym, true);
         break;
      }
      case '!':
      case TOK_POS:
      case TOK_NEG: {
         sym = createSymbol (node);
         sym->attributes.set(ATTR_vreg);
         sym->attributes.set(ATTR_int);

         symbol *child = scan (node->children[0]);
         if (child &&
            get_type_attr(child) != ATTR_int)
            errprintf ("%s is not INT\n",
                     child->lexinfo->c_str());

         node->attributes = get_attr (sym, true);
         break;
      }
      case '.': {
         // Look up the stack
         //symbol *st = lookup(node->children[0]->lexinfo);
         symbol *st = scan (node->children[0]);
         if (st == nullptr) {
            errprintf ("Stack %s not found", 
                        node->children[0]->lexinfo->c_str());
            return nullptr;
         }

         // Look up its structure
         // Should find it
         auto it = struct_table.find (st->type_name);
         if (it == struct_table.end()) {
            errprintf ("Structure not declared\n");
            return nullptr;
         }

         // Look up the fields
         symbol_table *fields = it->second->fields;

         auto f = fields->find (node->children[1]->lexinfo);
         if (f == fields->end()) {
            errprintf ("Field %s not found",
                       node->children[1]->lexinfo->c_str());
            return nullptr;
         }

         sym = f->second;
         node->children[1]->block_nr = sym->block_nr;
         node->children[1]->attributes = get_attr (sym, true);
         node->children[1]->field_of = *sym->field_of;

         symbol *s = createSymbol (node);
         s->attributes.set (get_type_attr(f->second));
         s->attributes.set (ATTR_vaddr);
         s->attributes.set (ATTR_lval);

         node->attributes = get_attr(s, true);
         delete s;
         return sym;
         break;
      }
      case TOK_VOID: {
         sym = scan (node->children[0]);
         sym->attributes.set(ATTR_void);
         return sym;
         break;
      }
      case TOK_STRING: {
         symbol *s = createSymbol (node);
         // If it has array
         if (node->children[0]->symbol == TOK_ARRAY) {
            sym = scan (node->children[1]);
            // Add array attribute
            sym->attributes.set(ATTR_array);
            s->attributes.set(ATTR_array);
         }
         else
            sym = scan (node->children[0]);

         // Add string attribute to symbol
         sym->attributes.set(ATTR_string);
         s->attributes.set(ATTR_string);

         node->attributes = get_attr(s, true);
         delete s;
         return sym;
         break;
      }
      case TOK_STRINGCON: {
         sym = createSymbol (node);
         sym->attributes.set(ATTR_string);
         sym->attributes.set(ATTR_const);

         node->attributes = get_attr (sym, true);
         break;
      }
      case TOK_INT: {
         symbol *s = createSymbol (node);
         // If it has array
         if (node->children[0]->symbol == TOK_ARRAY) {
            sym = scan (node->children[1]);
            // Add array attribute
            sym->attributes.set(ATTR_array);
            s->attributes.set(ATTR_array);
         }
         else
            sym = scan (node->children[0]);

         // Add int attribute to symbol
         sym->attributes.set(ATTR_int);
         s->attributes.set(ATTR_int);

         node->attributes = get_attr(s, true);
         delete s;
         return sym;
         break;
      }
      case TOK_CHAR:
      case TOK_INTCON: {
         sym = createSymbol (node);
         sym->attributes.set(ATTR_int);
         sym->attributes.set(ATTR_const);

         node->attributes = get_attr (sym, true);
         break;
      }
      case TOK_NULL: {
         sym = createSymbol (node);
         sym->attributes.set(ATTR_null);
         sym->attributes.set(ATTR_const);

         node->attributes = get_attr (sym, true);
         break;
      }
      case TOK_NEWSTRING:
         // new only has 1 child
         sym = scan (node->children[0]);
         sym->attributes.set(ATTR_vreg);
         sym->attributes.set(ATTR_string);

         node->attributes = get_attr (sym, true);
         break;
      case TOK_NEW: {
         // new only has 1 child
         sym = scan (node->children[0]);
         sym->attributes.set(ATTR_vreg);

         node->attributes = get_attr (sym, true);
         break;
      }
      case TOK_NEWARRAY: {
         // In case the size isn't integer
         symbol *s = scan (node->children[1]);
         if (s == nullptr)
            errprintf ("Invalid type: %s is not INT\n", 
                       s->lexinfo->c_str());

         // Still return the NEWARRAY symbol
         sym = scan (node->children[0]);
         node->attributes = get_attr (sym, true);
         break;
      }
      case TOK_ARRAY: {
         sym = createSymbol (node);
         break;
      }
      case TOK_TYPEID: {
         symbol *s = createSymbol (node);
         // In case the NEW is calling this or STRUCT
         if (node->children.size() == 0) {
            sym = lookup (node->lexinfo);
            if (sym == nullptr) sym = createSymbol (node);
            if (*node->lexinfo == "string")
               sym->attributes.set(ATTR_string);
            else if (*node->lexinfo == "int")
               sym->attributes.set(ATTR_int);
         }
         else {
            sym = scan (node->children[0]);
            
            // Will always be struct type.
            sym->attributes.set(ATTR_struct);
            if (node->children.size() == 2)
               sym->attributes.set(ATTR_array);
            sym->type_name = node->lexinfo;
         }

         if (sym)
            node->attributes = get_attr (sym, true);
         delete s;

         return sym;
      }
      case ';':
      case TOK_FIELD:
      case TOK_DECLID: {
         // This is the lowest level
         sym = createSymbol (node);
         break;
      }
      case TOK_IDENT: {
         // This is for declared variables
         // This will lookup globally AND struct_table
         sym = lookup (node->lexinfo, true);
         if (sym == nullptr) {
            sym = lookup (node->lexinfo);
            if (sym == nullptr) {
               eprintf ("Cannot find variable: %s\n", 
                  node->lexinfo->c_str());
               return nullptr;
            }
         }

         sym->attributes.set(ATTR_vreg);

         node->attributes = get_attr(sym, true, true);
         node->block_nr = sym->block_nr;
         return sym;
      }
      case TOK_WHILE:
      case TOK_IF: {
         // Just create TOK_WHILE/TOK_IF to get numbers
         symbol *s = createSymbol (node);
         delete s;

         scan (node->children[0]);

         scan (node->children[1]);
         return nullptr;
         break;
      }
      case TOK_IFELSE: {
         scan (node->children[0]);

         // Go to block 1
         scan (node->children[1]);

         // Subtract block number
         next_block++;
         scan (node->children[2]);

         return nullptr;
         break;
      }
      case TOK_CALL: {
         symbol *s = createSymbol (node);
         delete s;

         sym = scan (node->children[0]);
         if (sym == nullptr) return nullptr;
         node->attributes = get_attr (sym, true);

         break;
      }
      case TOK_INDEX: {
         symbol *s = createSymbol (node);
         symbol *child1 = scan (node->children[0]);
         symbol *child2 = scan (node->children[1]);

         if (child2 && get_type_attr(child2) != ATTR_int) {
            errprintf ("Invalid type index\n");
            return nullptr;
         }

         if (child1) {
            if (get_type_attr(child1) == ATTR_string)
               s->attributes.set(ATTR_int);
            else
               s->attributes.set(get_type_attr(child1));
         }

         s->attributes.set(ATTR_lval);
         s->attributes.set(ATTR_vaddr);

         node->attributes = get_attr (s, true);
         return s;
         break;   
      }
      case TOK_RETURN: {
         symbol *s = createSymbol (node);
         symbol *child = scan (node->children[0]);
         
         if (child) s->attributes.set(get_type_attr(child));

         s->attributes.set(ATTR_vreg);

         node->attributes = get_attr (s, true);
         return s;
         break;
      }
      case TOK_STRUCT: {
         // Struct has to have at least 1 children
         // Getting the typeid node and setting its attributes
         // up
         symbol *typeid_sym = createSymbol (node->children[0]);
         typeid_sym->attributes.set (ATTR_struct);
         typeid_sym->type_name = typeid_sym->lexinfo;

         // Create a new entry in struct table
         struct_table.insert ({typeid_sym->lexinfo, typeid_sym});

         // Dump the struct symbol
         dump_symbol (typeid_sym);

         // Every struct has their fields
         typeid_sym->fields = new symbol_table();

         // Skip the first child
         bool first = true;
         for (astree *child: node->children) {
            // Skip the first child
            if (first) {
               first = false;
               continue;
            }

            // In case structure is used, but not defined
            if (child->symbol == TOK_TYPEID &&
                struct_table.find(child->lexinfo) == struct_table.end())
                  struct_table.insert({child->lexinfo, nullptr});

            // Making symbols for fields
            symbol *f = scan (child);
            // If it's a nullptr, there's something wrong.
            if (f == nullptr) return nullptr;

            // Set attributes for field
            f->attributes.set(ATTR_field);
            // Keep track of what this field is for
            f->field_of = typeid_sym->lexinfo;

            // Dump the field
            dump_symbol (f, 1);

            // Add the field into field table
            typeid_sym->fields->insert ({f->lexinfo, f});
         }

         // Don't try to scan the children!!!!
         return nullptr;
         break;
      }
      case TOK_PROTOTYPE: {
         // Get the name of the function
         symbol *proto = scan (node->children[0]);
         proto->attributes.set(ATTR_function);

         // Dump function symbol first
         dump_symbol (proto);
         
         // Add it into table
         symbol_table *table = getTable(); 
         table->insert ({proto->lexinfo, proto});

         // Handle parameters
         proto->parameters = new vector<symbol *>;
         for (astree *child: node->children[1]->children) {
            symbol *param = scan (child);
            param->attributes.set(ATTR_param);
            param->attributes.set(ATTR_lval);
            param->attributes.set(ATTR_variable);

            child->attributes = get_attr(param, true);

            proto->parameters->push_back (param);
            dump_symbol(param, 1);
         }

         node->attributes = get_attr (proto, true);

         return nullptr;
         break;
      }
      case TOK_FUNCTION: {
         symbol *fun = lookup (node->children[0]->children[0]->lexinfo);

         if (fun) {
            // Prototype?
            if (fun->attributes.test(ATTR_function)) {
               errprintf ("Duplicate function\n");
               return nullptr;
            }

            fun->attributes.set (ATTR_function);
         }
         else {
            fun = scan (node->children[0]);
            fun->attributes.set(ATTR_function);

            // Dump function symbol first
            dump_symbol (fun);
            
            // Add it into table
            symbol_table *table = getTable(); 
            table->insert ({fun->lexinfo, fun});

            // Each function has their own block
            enterBlock();

            // New table in another block
            symbol_table *new_table = getTable();

            // Handle parameters
            fun->parameters = new vector<symbol *>;
            for (astree *child: node->children[1]->children) {
               symbol *param = scan (child);
               param->attributes.set(ATTR_param);
               param->attributes.set(ATTR_lval);
               param->attributes.set(ATTR_variable);

               node->attributes = get_attr(param, true);

               fun->parameters->push_back (param);
               new_table->insert ({param->lexinfo, param});
               dump_symbol(param, 1);
            }

            fprintf (sym_file, "\n");
         }
         // Since the block is entered, don't need to process the block
         // again
         for (astree *child: node->children[2]->children) {
            // NOTE: don't need to check the symbol,
            // it should be nullptr
            scan (child);
            if (child->symbol == TOK_IFELSE) next_block--;
         }

         node->attributes = get_attr (fun, true);

         // Make sure to leave block
         leaveBlock();
         return nullptr;
         break;
      }
      case TOK_VARDECL: {
         symbol *var = nullptr;
         if (node->children[0]->symbol != '.') {
            astree *tmp = node->children[0]->children[0];
            const string *name = tmp->lexinfo;
            var = lookup (name, true);
            // If found locally, don't do anything, error it out
            if (var != nullptr) {
               errprintf ("%s already declared\n", name->c_str()); 
               return nullptr;
            }
         }

         if (var == nullptr) {
            var = scan (node->children[0]);
            symbol *var2 = scan (node->children[1]);

            // Type check
            if (var2 != nullptr && 
                get_type_attr(var) != get_type_attr(var2)) {
               errprintf ("Not the same type: %s and %s\n",
                          var->lexinfo->c_str(), 
                          var2->lexinfo->c_str());
               delete var2;
               return nullptr;
            }

            if (var2 == nullptr) delete var2;

            symbol *vardec = createSymbol (node);
            vardec->attributes.set(ATTR_vreg);
            vardec->attributes.set(get_type_attr(var));
            node->attributes = get_attr (vardec, true);
            delete vardec;

            // If null, there's something wrong
            if (var == nullptr) return nullptr;

            var->attributes.set(ATTR_lval);
            var->attributes.set(ATTR_variable);

            node->children[0]->attributes = get_attr(var, true);

            // Add it into table
            symbol_table *table = getTable();
            table->insert ({var->lexinfo, var});
            
            //dump_table();
            dump_symbol (var, getScope());
         }
         return nullptr;
         break;
      }
      case TOK_BLOCK: {
         symbol *s = createSymbol (node);
         delete s;
         enterBlock();

         // Scan the children
         for (astree *child: node->children) {
            scan (child);
            if (child->symbol == TOK_IFELSE)
               next_block--;
         }

         leaveBlock();
         return nullptr;
         break;
      }

   }

   // Keep going through all the children of the node
   for (astree *child: node->children)
      scan (child);

   return sym;
}

void SymbolStack::dump_symbol (symbol* sym, int depth) {
   if (sym->attributes.test (ATTR_field))
      fprintf (sym_file, "%*s%s (%zd.%zd.%zd) %s\n", 
        depth*3, "", sym->lexinfo->c_str(), sym->filenr,
         sym->linenr, sym->offset, get_attr (sym).c_str());
   else
      fprintf (sym_file, "%*s%s (%zd.%zd.%zd) {%zd} %s\n", 
        depth*3, "", sym->lexinfo->c_str(), sym->filenr,
        sym->linenr, sym->offset, sym->block_nr, 
        get_attr (sym).c_str());
}

void SymbolStack::dump_table () {
   printf("Stack size: %zd\n", symbol_stack.size());
   printf ("Table dump: ");
   for (symbol_table *table: symbol_stack) {
      printf("%p ", table);
      if (table == nullptr) continue;
      printf ("(");
      for (auto it = table->begin(); it != table->end(); ++it)
         printf ("%s ", it->first->c_str());
      printf (")");
   }
   printf ("\n");
}

string SymbolStack::get_attr (symbol* sym, bool tree, bool ident) {
   string attrs = "";
   if (sym->attributes.test(ATTR_field)) 
      attrs = attrs + "field {" + sym->field_of->c_str() + "} ";
   if (sym->attributes.test(ATTR_void))     attrs += "void ";
   if (sym->attributes.test(ATTR_int))      attrs += "int ";
   if (sym->attributes.test(ATTR_null))     attrs += "null ";
   if (sym->attributes.test(ATTR_string))   attrs += "string ";
   
   if (sym->attributes.test(ATTR_struct)) {
      if (tree == true)
            attrs = attrs + "struct ";
      else
         if (sym->type_name != nullptr)
            attrs = attrs + "struct \"" + 
                    sym->type_name->c_str() + "\" "; 
   }

   if (sym->attributes.test(ATTR_array))    attrs += "array ";
   if (sym->attributes.test(ATTR_function)) attrs += "function ";
   if (sym->attributes.test(ATTR_prototype))    attrs += "prototype ";
   if (sym->attributes.test(ATTR_variable)) attrs += "variable ";
   if (sym->attributes.test(ATTR_lval))     attrs += "lval ";
   if (sym->attributes.test(ATTR_param))    attrs += "param ";
   if (sym->attributes.test(ATTR_const))    attrs += "const ";
   if (sym->attributes.test(ATTR_vreg))     attrs += "vreg ";
   if (sym->attributes.test(ATTR_vaddr))    attrs += "vaddr ";

   if (ident == true) {
      attrs = attrs + "(" + to_string(sym->filenr) + "." +
              to_string(sym->linenr) + "." + to_string(sym->offset)
              + ")";
   }
   return attrs.c_str();
}

size_t SymbolStack::get_type_attr (symbol *sym) {
   if (sym->attributes.test(ATTR_void))     return ATTR_void;
   if (sym->attributes.test(ATTR_struct))     return ATTR_struct;
   if (sym->attributes.test(ATTR_int))     return ATTR_int;
   if (sym->attributes.test(ATTR_string))     return ATTR_string;
   return ATTR_null;
}

symbol* SymbolStack::lookup (const string *str, bool local) {
   //printf ("Looking up: %s\n", str->c_str());
   symbol *sym = nullptr;
   //Search only the local table since local hides global variable
   if (local) {
     symbol_table *table = symbol_stack.back();
     if (table == nullptr) return sym;
     auto found = table->find (str);
     if (found != table->end()) {
       sym = found->second;
     }
   } 
   else {
      //Iterate through the symbol_stack
      for (symbol_table *table: symbol_stack) {
         // Skip those that are nullptrs
         if (table == nullptr) continue;
         // Perform look up
         auto found = table->find (str);
         if (found != table->end()) {
            sym = found->second;
            break;
         }
      }

      // Look in the struct table too!
      auto it = struct_table.find (str);
      if (it != struct_table.end())
         sym = it->second;
   }

   return sym;
}

symbol_table* SymbolStack::getTable () {
   //dump_table();
   if (symbol_stack.back() == nullptr) {
      symbol_stack.pop_back();
      symbol_stack.push_back(new symbol_table);
   }

   return symbol_stack.back();
}

void SymbolStack::enterBlock () {
   //printf("Enter block: %zd\n", next_block);
   //dump_table();
   symbol_stack.push_back (nullptr);
   next_block++;
}

void SymbolStack::leaveBlock () {
   //printf("Leaving block: %zd\n", next_block-1);
   symbol_stack.pop_back();
   next_block--;
   //dump_table();
}

int SymbolStack::getScope () {
   size_t scope = -1;
   for (symbol_table *table: symbol_stack) {
      if (table != nullptr) scope++;
   }
   //printf("Scope: %zd\n", scope);
   return scope;
}
