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

#ifndef YY_CSSYY_PARSER_TAB_H_INCLUDED
# define YY_CSSYY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cssyydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    REDUCE = 258,
    S = 259,
    SGML_CD = 260,
    INCLUDES = 261,
    DASHMATCH = 262,
    BEGINSWITH = 263,
    ENDSWITH = 264,
    CONTAINS = 265,
    STRING = 266,
    IDENT = 267,
    NTH = 268,
    HASH = 269,
    HEXCOLOR = 270,
    IMPORT_SYM = 271,
    PAGE_SYM = 272,
    MEDIA_SYM = 273,
    FONT_FACE_SYM = 274,
    CHARSET_SYM = 275,
    NAMESPACE_SYM = 276,
    KHTML_RULE_SYM = 277,
    KHTML_DECLS_SYM = 278,
    KHTML_VALUE_SYM = 279,
    KHTML_MEDIAQUERY_SYM = 280,
    KHTML_SELECTORS_SYM = 281,
    IMPORTANT_SYM = 282,
    MEDIA_ONLY = 283,
    MEDIA_NOT = 284,
    MEDIA_AND = 285,
    QEMS = 286,
    EMS = 287,
    EXS = 288,
    CHS = 289,
    REMS = 290,
    PXS = 291,
    CMS = 292,
    MMS = 293,
    INS = 294,
    PTS = 295,
    PCS = 296,
    DEGS = 297,
    RADS = 298,
    GRADS = 299,
    MSECS = 300,
    SECS = 301,
    HERZ = 302,
    KHERZ = 303,
    DPI = 304,
    DPCM = 305,
    DIMEN = 306,
    PERCENTAGE = 307,
    FLOAT = 308,
    INTEGER = 309,
    URI = 310,
    FUNCTION = 311,
    NOTFUNCTION = 312,
    UNICODERANGE = 313
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


    CSSRuleImpl *rule;
    CSSSelector *selector;
    QList<CSSSelector*> *selectorList;
    bool ok;
    MediaListImpl *mediaList;
    CSSMediaRuleImpl *mediaRule;
    CSSRuleListImpl *ruleList;
    ParseString string;
    double val;
    int prop_id;
    unsigned int attribute;
    unsigned int element;
    CSSSelector::Relation relation;
    CSSSelector::Match match;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;

    khtml::MediaQuery* mediaQuery;
    khtml::MediaQueryExp* mediaQueryExp;
    QList<khtml::MediaQueryExp*>* mediaQueryExpList;
    khtml::MediaQuery::Restrictor mediaQueryRestrictor;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int cssyyparse (DOM::CSSParser *parser);

#endif /* !YY_CSSYY_PARSER_TAB_H_INCLUDED  */
