%{
// Dummy parser for scanner project.

#include <cassert>

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%token TOK_DECLID TOK_INDEX TOK_PARAMLIST TOK_PROTOTYPE
%token TOK_FUNCTION TOK_VARDECL TOK_RETURNVOID TOK_NEWSTRING

%start start

%right TOK_IF TOK_ELSE
%right '='
%left  TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left  '+' '-'
%left  '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_NEW
%left  '[' '.' TOK_FUNCTION
%nonassoc '('

%%

start       : program                   { parser::root = $1;         }
            ;

program     : program structdef         { $$ = $1->adopt ($2, NULL); }
            | program function          { $$ = $1->adopt ($2, NULL); }
            | program statement         { $$ = $1->adopt ($2, NULL); }
            | program error '}'         { $$ = $1;                   }
            | program error ';'         { $$ = $1;                   }
            |                           { $$ = new_parseroot ();     }
            ;

struct_head : struct_head fielddecl ';' { 
                destroy ($3); 
                $$ = $1->adopt ($2); }
            | TOK_STRUCT TOK_IDENT '{' fielddecl ';' { 
                destroy ($3, $5); 
                $2 = $2->adopt_sym (NULL, TOK_TYPEID);
                $$ = $1->adopt ($2, $4); }
            ;

structdef   : TOK_STRUCT TOK_IDENT '{' '}'  { destroy ($3, $4); 
                $2 = $2->adopt_sym (NULL, TOK_TYPEID); 
                $$ = $1->adopt ($2); }
            | struct_head '}' { destroy ($2); $$ = $1; }
            ;

fielddecl   : basetype TOK_ARRAY TOK_IDENT { 
                $3 = $3->adopt_sym (NULL, TOK_FIELD); 
                $$ = $1->adopt ($2, $3); }
            | basetype TOK_IDENT { 
                $2 = $2->adopt_sym (NULL, TOK_FIELD);
                $$ = $1->adopt ($2); }
            ;

basetype    : TOK_VOID    { $$ = $1; }
            | TOK_INT     { $$ = $1; }
            | TOK_STRING  { $$ = $1; }
            | TOK_CHAR    { $$ = $1; }
            | TOK_IDENT   { $$ = $1->adopt_sym (NULL, TOK_TYPEID); }
            ;

func_head   : '(' identdecl { 
                $$ = $1->adopt_sym ($2, TOK_PARAMLIST);}
            | '(' { 
                $$ = $1->adopt_sym (NULL, TOK_PARAMLIST); }
            | func_head ',' identdecl { 
                destroy ($2);
                $$ = $1->adopt ($3); }
            ;

function    : identdecl func_head ')' block { 
                destroy ($3);
                int sym = *($4->lexinfo) == ";"
                            ? TOK_PROTOTYPE : TOK_FUNCTION;
                astree *func = new astree(sym, $1->lloc, "");
                func = func->adopt ($1);
                // If function, it has 3 children
                if (sym == TOK_FUNCTION) $$ = func->adopt ($2, $4); 
                // If prototype, it has 2 children
                // Make sure to destroy ';'
                else {
                    destroy ($4);
                    $$ = func->adopt($2); 
                }
              }
            ;

identdecl   : basetype TOK_IDENT      { 
                $2 = $2->adopt_sym (NULL, TOK_DECLID); 
                $$ = $1->adopt ($2); }
            | basetype TOK_ARRAY TOK_IDENT { 
                $3 = $3->adopt_sym (NULL, TOK_DECLID); 
                $$ = $1->adopt ($2, $3); }
            ;

block_head  : block_head statement { 
                $$ = $1->adopt ($2); }
            | '{' statement { 
                $$ = $1->adopt_sym($2, TOK_BLOCK);}
            ;

block       : '{' '}' { 
                destroy ($2); 
                $$ = $1->adopt_sym (NULL, TOK_BLOCK); }
            | block_head '}'          { destroy ($2); $$ = $1; }
            | ';'                     { $$ = $1; }
            ;

statement   : block                   { $$ = $1; }
            | vardecl                 { $$ = $1; }
            | while                   { $$ = $1; }
            | ifelse                  { $$ = $1; }
            | return                  { $$ = $1; }
            | expr ';'                { destroy ($2); $$ = $1; }
            ;

vardecl     : identdecl '=' expr ';'  { 
                destroy ($4); 
                $2 = $2->adopt_sym (NULL, TOK_VARDECL); 
                $$ = $2->adopt ($1, $3); }
            ;

while       : TOK_WHILE '(' expr ')' statement { 
                destroy ($2, $4); 
                $$ = $1->adopt ($3, $5); }
            ;

ifelse      : TOK_IF '(' expr ')' statement %prec TOK_ELSE { 
                destroy ($2, $4); 
                $$ = $1->adopt ($3, $5); }
            | TOK_IF '(' expr ')' statement TOK_ELSE statement  { 
                destroy ($2, $4); 
                destroy ($6);
                $1 = $1->adopt ($3, $5); 
                $$ = $1->adopt_sym ($7, TOK_IFELSE); }
            ;

return      : TOK_RETURN ';' { 
                destroy ($2);
                $$ = $1->adopt_sym (NULL, TOK_RETURNVOID); }
            | TOK_RETURN expr ';' { 
                destroy ($3); 
                $$ = $1->adopt ($2); }
            ;

expr        : expr '=' expr      { $$ = $2->adopt ($1, $3); }
            | expr '+' expr      { $$ = $2->adopt ($1, $3); }
            | expr '-' expr      { $$ = $2->adopt ($1, $3); }
            | expr '*' expr      { $$ = $2->adopt ($1, $3); }
            | expr '/' expr      { $$ = $2->adopt ($1, $3); }
            | expr '%' expr      { $$ = $2->adopt ($1, $3); }
            | expr TOK_EQ expr   { $$ = $2->adopt ($1, $3); }
            | expr TOK_NE expr   { $$ = $2->adopt ($1, $3); }
            | expr TOK_LT expr   { $$ = $2->adopt ($1, $3); }
            | expr TOK_LE expr   { $$ = $2->adopt ($1, $3); }
            | expr TOK_GT expr   { $$ = $2->adopt ($1, $3); }
            | expr TOK_GE expr   { $$ = $2->adopt ($1, $3); }
            | '+' expr %prec TOK_POS { 
                $$ = $1->adopt_sym ($2, TOK_POS); }
            | '-' expr %prec TOK_NEG { 
                $$ = $1->adopt_sym ($2, TOK_NEG); }
            | '!' expr           { $$ = $1->adopt ($2); }
            | allocator          { $$ = $1; }
            | call               { $$ = $1; }
            | '(' expr ')'       { destroy ($1, $3); $$ = $2; }
            | variable           { $$ = $1; }
            | constant           { $$ = $1; }
            ;

allocator   : TOK_NEW TOK_IDENT '(' ')' { 
                destroy ($3, $4); 
                $2 = $2->adopt_sym (NULL, TOK_TYPEID);
                $$ = $1->adopt ($2); }
            | TOK_NEW TOK_STRING '(' expr ')' { 
                destroy ($3, $5); 
                destroy ($2);
                $$ = $1->adopt_sym ($4, TOK_NEWSTRING); }
            | TOK_NEW basetype '[' expr ']' { 
                destroy ($3, $5); 
                $1 = $1->adopt_sym (NULL, TOK_NEWARRAY); 
                $2 = $2->adopt_sym (NULL, TOK_TYPEID);
                $$ = $1->adopt ($2, $4);}
            ;


call_head   : call_head ',' expr {
               destroy($2); 
               $$ = $1->adopt($3);}
            | TOK_IDENT '(' expr {
               $2->adopt_sym(NULL, TOK_CALL); 
               $$ =$2->adopt($1, $3);}
            ;

call        : TOK_IDENT '(' ')' {
               destroy($3); 
               $$ = $2->adopt_sym($1, TOK_CALL);}
            | call_head ')'      {
               destroy($2); 
               $$ = $1;}
            ;

variable    : TOK_IDENT            { $$ = $1; }
            | expr '[' expr ']'    { 
                destroy ($4); 
                $2 = $2->adopt_sym (NULL, TOK_INDEX); 
                $$ = $2->adopt ($1, $3); }
            | expr '.' TOK_IDENT   { 
                $3 = $3->adopt_sym (NULL, TOK_FIELD);
                $$ = $2->adopt ($1, $3); }
            ;

constant    : TOK_INTCON           { $$ = $1; }
            | TOK_CHARCON          { $$ = $1; }
            | TOK_STRINGCON        { $$ = $1; }
            | TOK_NULL             { $$ = $1; }
            ;
%%


const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

/*
static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != nullptr);
   return result;
}
*/

