/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_KHTMLXPATHYY_PARSER_TAB_H_INCLUDED
# define YY_KHTMLXPATHYY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int khtmlxpathyydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    MULOP = 258,
    RELOP = 259,
    EQOP = 260,
    PLUS = 261,
    MINUS = 262,
    OR = 263,
    AND = 264,
    AXISNAME = 265,
    NODETYPE = 266,
    PI = 267,
    FUNCTIONNAME = 268,
    LITERAL = 269,
    VARIABLEREFERENCE = 270,
    NUMBER = 271,
    DOTDOT = 272,
    SLASHSLASH = 273,
    NAMETEST = 274,
    ERROR = 275
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 41 "parser.y" /* yacc.c:1909  */

	khtml::XPath::Step::AxisType axisType;
	int        num;
	DOM::DOMString *str; // we use this and not DOMStringImpl*, so the
	                     // memory management for this is entirely manual,
	                     // and not an RC/manual hybrid
	khtml::XPath::Expression *expr;
	QList<khtml::XPath::Predicate *> *predList;
	QList<khtml::XPath::Expression *> *argList;
	khtml::XPath::Step *step;
	khtml::XPath::LocationPath *locationPath;

#line 88 "parser.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE khtmlxpathyylval;

int khtmlxpathyyparse (void);

#endif /* !YY_KHTMLXPATHYY_PARSER_TAB_H_INCLUDED  */
