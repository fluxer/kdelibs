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

#ifndef YY_KIOTRADER_YACC_H_INCLUDED
# define YY_KIOTRADER_YACC_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int kiotraderdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NOT = 258,
    EQ = 259,
    EQI = 260,
    NEQ = 261,
    NEQI = 262,
    LEQ = 263,
    GEQ = 264,
    LE = 265,
    GR = 266,
    OR = 267,
    AND = 268,
    TOKEN_IN = 269,
    TOKEN_IN_SUBSTRING = 270,
    MATCH_INSENSITIVE = 271,
    TOKEN_IN_INSENSITIVE = 272,
    TOKEN_IN_SUBSTRING_INSENSITIVE = 273,
    EXIST = 274,
    MAX = 275,
    MIN = 276,
    VAL_BOOL = 277,
    VAL_STRING = 278,
    VAL_ID = 279,
    VAL_NUM = 280,
    VAL_FLOAT = 281
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 20 "yacc.y" /* yacc.c:1909  */

     char valb;
     int vali;
     double vald;
     char *name;
     void *ptr;

#line 89 "yacc.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int kiotraderparse (void *_scanner);

#endif /* !YY_KIOTRADER_YACC_H_INCLUDED  */
