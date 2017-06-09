// Implementation of Emit class

#include "emit.h"

const char *INDENT = "    ";
vector<astree *> global_dec;
string currentStruct = "";

Emit::Emit (FILE* out) {
   oil_file = out;
}

void Emit::emit (astree *root) {
   // Should recursively run the node
   // NOTE: Before that, it has to write all the:
   //    1. Struct definitions
   //    2. String constants
   //    3. Global variables
   //    4. Functions
   //    5. void __ocmain (void)

   // Process struct definitions
   emit_struct (root);   

   // Process string definitions
   emit_string (root);   

   // Process variables definitions
   emit_variables (root);   

   // Process functions
   emit_function (root);

   // Print the ocmain() function
   emit_print ("void __ocmain (void) \n{\n");
   // Print all the declarations of global variables
   for (astree *node: global_dec) {
      // Now save a string for right-hand side of the VARDECL

      // Make sure to handle array []
      astree* target = node->children[0];
      if (target->children.size() == 2) target = target->children[1];
      else target = target->children[0];

      emit_print ("%s%s = %s;\n", INDENT, emit_rec(target).c_str(), 
                          emit_rec(node->children[1], true).c_str()); 
   }

   // Process normal statements, recursively
   emit_rec (root);

   emit_print ("}\n");

   return;
}

// Emit structs
void Emit::emit_struct (astree *root) {
   // Process all the structs
   for (astree *child: root->children) {
      if (child->symbol != TOK_STRUCT) continue;

      // Output the name of the struct
      emit_print ("struct s_%s {", 
                  emit_rec(child->children[0]).c_str());

      // Now output the fields if any
      bool first = true;
      for (astree *field: child->children) {
         // Make sure to skip the first children
         if (first) {
            first = false;
            continue;
         }

         currentStruct = *child->children[0]->lexinfo;

         // These are the fields
         emit_print ("\n%s%s %s;", INDENT, gettype(field).c_str(), 
                     emit_rec(field).c_str());
      }
      emit_print ("\n};\n");
   }
   return;
}

// Emit strings
void Emit::emit_string (astree *root) {
   // Process all the VARDECLS first
   for (astree *child: root->children) {
      if (child->symbol != TOK_VARDECL) continue;

      // Only print out the string
      if (child->children[0]->symbol != TOK_STRING) continue;

      // Print out the stuffs inside the string
      emit_print ("\n%s %s;", gettype(child->children[0]).c_str(), 
                  emit_rec(child->children[0]).c_str());

      global_dec.push_back(child);
   }

   return;
}

// Emit variables
void Emit::emit_variables (astree *root) {
   // Process all the VARDECLS except TOK_STRING
   bool exist = false;
   for (astree *child: root->children) {
      if (child->symbol != TOK_VARDECL) continue;

      // Don't print out the strings
      if (child->children[0]->symbol == TOK_STRING) continue;

      emit_print ("\n%s %s;", gettype(child).c_str(), 
                  emit_rec(child->children[0]).c_str());

      global_dec.push_back(child);

      exist = true;
   }

   if (exist) emit_print ("\n\n");

   return;
}

// Emit functions
void Emit::emit_function (astree *root) {
   for (astree *child: root->children) {
      if (child->symbol != TOK_FUNCTION &&
          child->symbol != TOK_PROTOTYPE) continue;

      // Function/prototype head
      emit_print ("%s %s (", gettype (child).c_str(),
                     emit_rec (child->children[0]).c_str());
      // Process the parameters
      bool first = true;
      for (astree *par: child->children[1]->children) {
         if (!first) emit_print (", ");

         if (child->symbol == TOK_FUNCTION)
            emit_print ("\n%s%s %s", INDENT, gettype(par).c_str(),
                        emit_rec(par->children[0]).c_str());
         else
            emit_print ("%s %s", gettype(par).c_str(),
                        emit_rec(par->children[0]).c_str());

         first = false;
      }

      emit_print (")%s\n", (child->symbol == TOK_FUNCTION) ? "" : ";");

      if (child->symbol == TOK_FUNCTION) {
         emit_print ("{\n");

         // Process inner statements
         for (astree *node: child->children[2]->children)
            emit_rec (node);

         emit_print ("}\n");
      }
   }
   emit_print("\n");
}

// Recursively emit the current node recursively
// NOTE: This method will be used by everything.
string Emit::emit_rec (astree *node, bool right) {
   string result = "";
   switch (node->symbol) {
      // Skip over VARDECL, FUNCTIONS, STRUCTS, PROTOTYPES,
      // because they're all processed.
      case TOK_FUNCTION:
      case TOK_STRUCT:
      case TOK_PROTOTYPE:
         return "";
      case '=':
      case TOK_VARDECL: {
         if (node->block_nr == 0) return "";

         string leftv = emit_rec (node->children[0]);
         string rightv = emit_rec (node->children[1], true);

         emit_print ("%s%s = %s;\n", INDENT,
                     leftv.c_str(), rightv.c_str());

         return "";
      } 
      case TOK_IDENT:
      case TOK_DECLID:
         if (node->block_nr == 0) 
            result += string ("__");
         else 
            result += string ("_") + to_string (node->block_nr) + 
                      string("_");

         result += string (node->lexinfo->c_str());
         return result;
      case TOK_FIELD:
         result += string("f_") + currentStruct + string ("_") +
                  string (node->lexinfo->c_str());
         return result;
      case TOK_TYPEID:
         // These are basically the structs
         // There are 2 types, one that doesn't have DECLID
         // and one that does have DECLID
         // struct name;
         if (node->children.size() == 1)
            result += emit_rec(node->children[0]);
         // struct[] name;
         else if (node->children.size() == 2)
            result += emit_rec(node->children[1]);
         else
            result += *node->lexinfo;

         return result;
      case TOK_INT:
      case TOK_STRING:
         // Return the string of a particular line (string/int)
         if (node->children.size() == 2)
            result += emit_rec (node->children[1]);
         else
            result += emit_rec (node->children[0]);
         return result;
      case TOK_VOID:
         result += emit_rec (node->children[0]);
         return result;
      case TOK_NULL:
      case TOK_STRINGCON:
      case TOK_INTCON: {
         result += *node->lexinfo;
         return result;
      }
      case TOK_INDEX: {
         if (right) result = "&";
         result += emit_rec(node->children[0]) + "[" + 
                   emit_rec(node->children[1]) + "]";
         return result;
      }

      // Requires special names
      case TOK_WHILE: {
         emit_print ("while_%zd_%zd_%zd:;\n", node->lloc.filenr, 
                     node->lloc.linenr, node->lloc.offset);

         // Conditions
         string target = emit_rec (node->children[0]);

         emit_print ("%sif (!%s) goto break_%zd_%zd_%zd;\n",
                     INDENT, target.c_str(), node->lloc.filenr, 
                     node->lloc.linenr, node->lloc.offset);

         // Recursively do the TOK_BLOCK
         emit_rec (node->children[1]);

         // Emit goto while to loop
         emit_print ("%sgoto while_%zd_%zd_%zd;\n", INDENT, 
                     node->lloc.filenr, node->lloc.linenr, 
                     node->lloc.offset);

         // Emit the break point
         emit_print ("break_%zd_%zd_%zd:;\n", node->lloc.filenr,
                     node->lloc.linenr, node->lloc.offset);
         return "";
      }
      // Requires special names
      case TOK_IF: {
         // First expression
         string target = emit_rec (node->children[0]);
         // Condition
         emit_print ("%sif (!%s) goto fi_%zd_%zd_%zd;\n",
                     INDENT, target.c_str(), node->lloc.filenr, 
                     node->lloc.linenr, node->lloc.offset);

         // Recursively do the TOK_BLOCK
         emit_rec (node->children[1]);

         // Emit the break point
         emit_print ("fi_%zd_%zd_%zd:;\n", node->lloc.filenr,
                     node->lloc.linenr, node->lloc.offset);
         return "";
      }
         break;
      case TOK_IFELSE: {
         // First expression
         string target = emit_rec (node->children[0]);
         // Condition
         emit_print ("%sif (!%s) goto else_%zd_%zd_%zd;\n",
                     INDENT, target.c_str(), node->lloc.filenr, 
                     node->lloc.linenr, node->lloc.offset);

         // Recursively do the TOK_BLOCK
         emit_rec (node->children[1]);

         // goto fi
         emit_print ("%sgoto fi_%zd_%zd_%zd;\n", INDENT,
                     node->lloc.filenr, node->lloc.linenr,
                     node->lloc.offset);

         emit_print ("else_%zd_%zd_%zd;\n", node->lloc.filenr, 
                     node->lloc.linenr, node->lloc.offset);

         // Emit third child
         emit_rec (node->children[2]);

         // Emit the break point
         emit_print ("fi_%zd_%zd_%zd:;\n", node->lloc.filenr,
                     node->lloc.linenr, node->lloc.offset);
         return "";
      }
         break;

      case TOK_BLOCK:
         break;
      case TOK_EQ:
      case TOK_NE:
      case TOK_GT:
      case TOK_GE:
      case TOK_LT:
      case TOK_LE: {
         string op;
         switch (node->symbol) {
            case TOK_EQ:
               op = "==";
               break;
            case TOK_NE:
               op = "!=";
               break;
            case TOK_GT:
               op = ">";
               break;
            case TOK_GE:
               op = ">=";
               break;
            case TOK_LT:
               op = "<";
               break;
            case TOK_LE:
               op = "<=";
               break;
         }

         string leftv = emit_rec (node->children[0], true);
         string rightv = emit_rec (node->children[1], true);
         string target = vreg (node);

         emit_print ("%s%s %s = %s %s %s;\n", INDENT,
                      gettype(node).c_str(), target.c_str(), 
                      leftv.c_str(), op.c_str(), rightv.c_str());
                     
         return target;
      }
      case '.': {
         string leftv = emit_rec (node->children[0]);
         currentStruct = node->children[1]->field_of;
         string rightv = emit_rec (node->children[1], true);
         string target = vreg (node);

         emit_print ("%s%s %s = &%s.%s;\n", INDENT,
                      gettype(node).c_str(), target.c_str(), 
                      leftv.c_str(), rightv.c_str());

         return target;
         break;
      }
      case '+':
      case '-':
      case '/':
      case '*':
      case '%': {
         string leftv = emit_rec (node->children[0], true);
         string rightv = emit_rec (node->children[1], true);
         string target = vreg (node);

         emit_print ("%s%s %s = %s %c %s;\n", INDENT,
                     gettype(node).c_str(), target.c_str(), 
                     leftv.c_str(), (char)(node->symbol), 
                     rightv.c_str());
         return target;
      }
      case TOK_POS: {
         string target = vreg (node);
         string arg = emit_rec(node->children[0]);

         emit_print ("%s%s %s = %s;\n", INDENT,
                     gettype(node).c_str(), target.c_str(), 
                     arg.c_str());

         return target;
      }
      case TOK_NEG: {
         string target = vreg (node);
         string arg = emit_rec(node->children[0]);

         emit_print ("%s%s %s = -%s;\n", INDENT,
                     gettype(node).c_str(), target.c_str(), 
                     arg.c_str());

         return target;
      }
      case '!': {
         string target = vreg (node);
         string arg = emit_rec(node->children[0]);

         emit_print ("%s%s %s = !%s;\n", INDENT,
                     gettype(node).c_str(), target.c_str(), 
                     arg.c_str());

         return target;
      }

      case TOK_NEWSTRING: {
         string target = vreg (node);
         string type = gettype(node);
         
         emit_print ("%s%s %s = xcalloc (%s, sizeof (%s));\n",
                     INDENT, type.c_str(), target.c_str(),
                     emit_rec (node->children[0]).c_str(), 
                     type.substr(0, type.size() -1).c_str()); 
         return target;
      }
      case TOK_NEW: {
         string target = vreg (node);
         string type = gettype(node);
         
         emit_print ("%s%s %s = xcalloc (%s, sizeof (%s));\n",
                     INDENT, type.c_str(), target.c_str(),
                     "1", type.substr(0, type.size() -1).c_str()); 
         return target;
      }
      case TOK_NEWARRAY: {
         string target = vreg (node);
         
         emit_print ("%s%s* %s = xcalloc (%s, sizeof (%s));\n",
                     INDENT, gettype(node).c_str(), target.c_str(),
                     emit_rec (node->children[1]).c_str(), 
                     gettype(node).c_str());
         return target;
      }
      case TOK_CALL: {
         vector<string> params;
         // Process the parameters
         bool first = true;
         for (astree *child: node->children) {
            if (first) {
               first = false;
               continue;
            }

            params.push_back(emit_rec (child));
         }

         string ident = emit_rec (node->children[0]);
         string target = vreg (node);
         if (node->attributes.find ("void") == string::npos) {
            emit_print ("%s%s %s = %s(", INDENT, gettype(node).c_str(),
                        target.c_str(), ident.c_str());
         }
         else {
            emit_print ("%s%s(", INDENT, ident.c_str());
         }

         // Emit the parameters
         first = true;
         for (string par: params) {
            if (!first) emit_print(", ");
            emit_print("%s", par.c_str());
            first = false;
         }
         emit_print (");\n");

         return target;
      }
      case TOK_RETURN: {
         string reg = emit_rec (node->children[0]);

         emit_print ("%sreturn %s;\n", INDENT, reg.c_str());
         return reg;
      }
   }

   for (astree* child: node->children)
      emit_rec (child);
   return result;
}

int counter = 0;
string Emit::vreg (astree* node) {
   char typechar;
   // For int
   if (node->attributes.find("int") != string::npos)
      typechar = 'i';
   // For string constant
   if (node->attributes.find("string") != string::npos &&
       node->attributes.find("const") != string::npos)
      typechar = 's';

   if (gettype(node).find("*") != string::npos &&
       node->attributes.find("vaddr") == string::npos)
      typechar = 'p';

   // For addresses
   if (node->attributes.find("vaddr") != string::npos)
      typechar = 'a';

   return string ("") +  typechar + to_string (++counter);
}

string Emit::gettype (astree* node) {
   string type = "";
   if (node->attributes.find("int") != string::npos)
      type = "int";
   if (node->attributes.find("char") != string::npos)
      type = "char";
   if (node->attributes.find("string") != string::npos)
      type = "char*";
   if (node->attributes.find("struct") != string::npos) {
      type = "struct s_" + currentStruct + "*";
   }
   if (node->attributes.find("void") != string::npos)
      type = "void"; 

   if (node->attributes.find("array") != string::npos ||
       node->symbol == '.')
      type += "*";

   return type;
}

// Replacement for fprintf/printf
void Emit::emit_print (const char* format, ...) {
   va_list args;
   va_start (args, format);
   // Change this to vfprintf (oil_file, format, args) if want
   // to print to file
   vfprintf (oil_file, format, args);
   //vprintf (format, args);
   va_end (args);

   return;
}
