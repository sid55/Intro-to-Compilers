
%{

#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }

int yylval_token (int symbol) {
   yylval = new astree (symbol, lexer::lloc, yytext);
   astree::print(tok_file, yylval, 0);
   return symbol;
}

%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option warn
%option verbose

LETTER          [A-Za-z_]
DIGIT           [0-9]
MANTISSA        ({DIGIT}+\.?{DIGIT}*|\.{DIGIT}+)
EXPONENT        ([Ee][+-]?{DIGIT}+)
NUMBER          ({MANTISSA}{EXPONENT}?)
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
BADIDENT        ({DIGIT}+{IDENT})
STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")
BADSTRING       (\"([^\\"\n]|\\[\\'"0nt])*)
CHAR            ('([^\\'\n]|\\[\\'"0nt])')
BADCHAR         ('([^\\'\n]|\\[\\'"0nt]))

%%

"#".*           { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }

"char"          { return yylval_token (TOK_CHAR); }
"else"          { return yylval_token (TOK_ELSE); }
"if"            { return yylval_token (TOK_IF); }
"int"           { return yylval_token (TOK_INT); }
"new"           { return yylval_token (TOK_NEW); }
"null"          { return yylval_token (TOK_NULL); }
"return"        { return yylval_token (TOK_RETURN); }
"string"        { return yylval_token (TOK_STRING); }
"struct"        { return yylval_token (TOK_STRUCT); }
"void"          { return yylval_token (TOK_VOID); }
"while"         { return yylval_token (TOK_WHILE); }

{NUMBER}        { return yylval_token (TOK_INTCON); }
{IDENT}         { return yylval_token (TOK_IDENT); }
"="             { return yylval_token ('='); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"^"             { return yylval_token ('^'); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
";"             { return yylval_token (';'); } 
"!"             { return yylval_token ('!'); }
"%"             { return yylval_token ('%'); }
","             { return yylval_token (','); }
"."             { return yylval_token ('.'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }

"!="            { return yylval_token (TOK_NE); }
"<="            { return yylval_token (TOK_LE); }
"<"             { return yylval_token (TOK_LT); }
"=="            { return yylval_token (TOK_EQ); }
">="            { return yylval_token (TOK_GE); }
">"             { return yylval_token (TOK_GT); }
"[]"            { return yylval_token (TOK_ARRAY); }

{NOTNUMBER}     { lexer::badtoken (yytext);
                  return yylval_token (TOK_INTCON); }

{BADIDENT}      { lexer::badtoken (yytext);
                  return yylval_token (TOK_IDENT); }

{STRING}        { return yylval_token (TOK_STRINGCON); }
{BADSTRING}     { lexer::badtoken (yytext);
                  return yylval_token (TOK_STRINGCON); }

{CHAR}          { return yylval_token (TOK_CHARCON); }
{BADCHAR}       { lexer::badtoken (yytext);
                  return yylval_token (TOK_CHARCON); }

.               { lexer::badchar (*yytext); }

%%

