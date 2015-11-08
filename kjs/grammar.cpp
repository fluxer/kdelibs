/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         kjsyyparse
#define yylex           kjsyylex
#define yyerror         kjsyyerror
#define yydebug         kjsyydebug
#define yynerrs         kjsyynerrs

#define yylval          kjsyylval
#define yychar          kjsyychar
#define yylloc          kjsyylloc

/* Copy the first part of user declarations.  */
#line 1 "grammar.y" /* yacc.c:339  */


/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <config-kjs.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "nodes.h"
#include "makenodes.h"
#include "lexer.h"
#include "internal.h"

// Not sure why, but yacc doesn't add this define along with the others.
#define yylloc kjsyylloc

/* default values for bison */
#define YYDEBUG 0 // Set to 1 to debug a parse error.
#define kjsyydebug 0 // Set to 1 to debug a parse error.
#if !PLATFORM(DARWIN)
    // avoid triggering warnings in older bison
#define YYERROR_VERBOSE
#endif

extern int kjsyylex();
int kjsyyerror(const char *);
static bool allowAutomaticSemicolon();

#define AUTO_SEMICOLON do { if (!allowAutomaticSemicolon()) YYABORT; } while (0)
#define DBG(l, s, e) (l)->setLoc((s).first_line, (e).last_line)

#ifndef __GNUC__
#   define  __attribute__(x)
#endif

using namespace KJS;


#line 138 "grammar.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "grammar.tab.h".  */
#ifndef YY_KJSYY_GRAMMAR_TAB_H_INCLUDED
# define YY_KJSYY_GRAMMAR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int kjsyydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NULLTOKEN = 258,
    TRUETOKEN = 259,
    FALSETOKEN = 260,
    BREAK = 261,
    CASE = 262,
    DEFAULT = 263,
    FOR = 264,
    NEW = 265,
    VAR = 266,
    CONSTTOKEN = 267,
    CONTINUE = 268,
    FUNCTION = 269,
    RETURN = 270,
    VOIDTOKEN = 271,
    DELETETOKEN = 272,
    IF = 273,
    THISTOKEN = 274,
    DO = 275,
    WHILE = 276,
    INTOKEN = 277,
    INSTANCEOF = 278,
    TYPEOF = 279,
    SWITCH = 280,
    WITH = 281,
    RESERVED = 282,
    THROW = 283,
    TRY = 284,
    CATCH = 285,
    FINALLY = 286,
    DEBUGGER = 287,
    IMPORT = 288,
    IF_WITHOUT_ELSE = 289,
    ELSE = 290,
    EQEQ = 291,
    NE = 292,
    STREQ = 293,
    STRNEQ = 294,
    LE = 295,
    GE = 296,
    OR = 297,
    AND = 298,
    PLUSPLUS = 299,
    MINUSMINUS = 300,
    LSHIFT = 301,
    RSHIFT = 302,
    URSHIFT = 303,
    PLUSEQUAL = 304,
    MINUSEQUAL = 305,
    MULTEQUAL = 306,
    DIVEQUAL = 307,
    LSHIFTEQUAL = 308,
    RSHIFTEQUAL = 309,
    URSHIFTEQUAL = 310,
    ANDEQUAL = 311,
    MODEQUAL = 312,
    XOREQUAL = 313,
    OREQUAL = 314,
    NUMBER = 315,
    STRING = 316,
    IDENT = 317,
    AUTOPLUSPLUS = 318,
    AUTOMINUSMINUS = 319
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 64 "grammar.y" /* yacc.c:355  */

  int                 ival;
  double              dval;
  UString             *ustr;
  Identifier          *ident;
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FunctionBodyNode    *body;
  FuncDeclNode        *func;
  FuncExprNode        *funcExpr;
  ProgramNode         *prog;
  AssignExprNode      *init;
  SourceElementsNode  *srcs;
  ArgumentsNode       *args;
  ArgumentListNode    *alist;
  VarDeclNode         *decl;
  VarDeclListNode     *vlist;
  CaseBlockNode       *cblk;
  ClauseListNode      *clist;
  CaseClauseNode      *ccl;
  ElementNode         *elm;
  Operator            op;
  PropertyListNode   *plist;
  PropertyNode       *pnode;
  PropertyNameNode   *pname;
  PackageNameNode     *pkgn;

#line 272 "grammar.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE kjsyylval;
extern YYLTYPE kjsyylloc;
int kjsyyparse (void);

#endif /* !YY_KJSYY_GRAMMAR_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 303 "grammar.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  212
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1718

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  114
/* YYNRULES -- Number of rules.  */
#define YYNRULES  352
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  589

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    78,     2,     2,     2,    80,    83,     2,
      67,    68,    79,    75,    69,    76,    74,    65,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    66,    88,
      81,    87,    82,    86,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    72,     2,    73,    84,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,    85,    71,    77,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   192,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   226,   227,   231,   232,   233,   234,   235,
     236,   242,   251,   252,   253,   257,   258,   258,   262,   262,
     269,   270,   274,   275,   276,   278,   282,   283,   284,   285,
     286,   291,   292,   293,   297,   298,   303,   304,   308,   309,
     313,   314,   315,   316,   317,   321,   322,   323,   324,   328,
     329,   333,   334,   338,   339,   340,   341,   345,   346,   347,
     348,   352,   353,   357,   358,   362,   363,   367,   368,   372,
     373,   374,   378,   379,   380,   384,   385,   386,   387,   388,
     389,   390,   391,   392,   393,   394,   397,   398,   402,   403,
     407,   408,   409,   410,   414,   415,   417,   419,   424,   425,
     426,   430,   431,   433,   438,   439,   440,   441,   445,   446,
     447,   448,   452,   453,   454,   455,   456,   457,   458,   462,
     463,   464,   465,   466,   467,   472,   473,   474,   475,   476,
     477,   479,   483,   484,   485,   486,   487,   491,   492,   494,
     496,   498,   503,   504,   506,   507,   509,   514,   515,   519,
     520,   525,   526,   530,   531,   535,   536,   541,   542,   547,
     548,   552,   553,   558,   559,   564,   565,   569,   570,   575,
     576,   581,   582,   586,   587,   592,   593,   597,   598,   603,
     604,   609,   610,   615,   616,   621,   622,   627,   628,   633,
     634,   635,   636,   637,   638,   639,   640,   641,   642,   643,
     644,   648,   649,   653,   654,   658,   659,   663,   664,   665,
     666,   667,   668,   669,   670,   671,   672,   673,   674,   675,
     676,   677,   678,   679,   683,   684,   688,   689,   693,   694,
     699,   700,   705,   706,   710,   711,   715,   716,   721,   722,
     727,   728,   732,   736,   740,   744,   745,   749,   751,   756,
     757,   758,   759,   761,   763,   771,   773,   778,   779,   783,
     784,   788,   789,   790,   791,   795,   796,   797,   798,   802,
     803,   804,   805,   809,   813,   817,   818,   823,   824,   828,
     829,   833,   834,   838,   839,   843,   847,   848,   852,   853,
     854,   859,   860,   864,   865,   869,   871,   873,   875,   877,
     879,   884,   884,   885,   885,   890,   890,   893,   893,   896,
     896,   897,   897,   903,   904,   908,   909,   913,   914,   918,
     919,   923,   924
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NULLTOKEN", "TRUETOKEN", "FALSETOKEN",
  "BREAK", "CASE", "DEFAULT", "FOR", "NEW", "VAR", "CONSTTOKEN",
  "CONTINUE", "FUNCTION", "RETURN", "VOIDTOKEN", "DELETETOKEN", "IF",
  "THISTOKEN", "DO", "WHILE", "INTOKEN", "INSTANCEOF", "TYPEOF", "SWITCH",
  "WITH", "RESERVED", "THROW", "TRY", "CATCH", "FINALLY", "DEBUGGER",
  "IMPORT", "IF_WITHOUT_ELSE", "ELSE", "EQEQ", "NE", "STREQ", "STRNEQ",
  "LE", "GE", "OR", "AND", "PLUSPLUS", "MINUSMINUS", "LSHIFT", "RSHIFT",
  "URSHIFT", "PLUSEQUAL", "MINUSEQUAL", "MULTEQUAL", "DIVEQUAL",
  "LSHIFTEQUAL", "RSHIFTEQUAL", "URSHIFTEQUAL", "ANDEQUAL", "MODEQUAL",
  "XOREQUAL", "OREQUAL", "NUMBER", "STRING", "IDENT", "AUTOPLUSPLUS",
  "AUTOMINUSMINUS", "'/'", "':'", "'('", "')'", "','", "'{'", "'}'", "'['",
  "']'", "'.'", "'+'", "'-'", "'~'", "'!'", "'*'", "'%'", "'<'", "'>'",
  "'&'", "'^'", "'|'", "'?'", "'='", "';'", "$accept", "Keywords",
  "IdentifierName", "Literal", "PropertyName", "Property", "$@1", "$@2",
  "PropertyList", "PrimaryExpr", "PrimaryExprNoBrace", "ArrayLiteral",
  "ElementList", "ElisionOpt", "Elision", "MemberExpr", "MemberExprNoBF",
  "NewExpr", "NewExprNoBF", "CallExpr", "CallExprNoBF", "Arguments",
  "ArgumentList", "LeftHandSideExpr", "LeftHandSideExprNoBF",
  "PostfixExpr", "PostfixExprNoBF", "UnaryExprCommon", "UnaryExpr",
  "UnaryExprNoBF", "MultiplicativeExpr", "MultiplicativeExprNoBF",
  "AdditiveExpr", "AdditiveExprNoBF", "ShiftExpr", "ShiftExprNoBF",
  "RelationalExpr", "RelationalExprNoIn", "RelationalExprNoBF",
  "EqualityExpr", "EqualityExprNoIn", "EqualityExprNoBF", "BitwiseANDExpr",
  "BitwiseANDExprNoIn", "BitwiseANDExprNoBF", "BitwiseXORExpr",
  "BitwiseXORExprNoIn", "BitwiseXORExprNoBF", "BitwiseORExpr",
  "BitwiseORExprNoIn", "BitwiseORExprNoBF", "LogicalANDExpr",
  "LogicalANDExprNoIn", "LogicalANDExprNoBF", "LogicalORExpr",
  "LogicalORExprNoIn", "LogicalORExprNoBF", "ConditionalExpr",
  "ConditionalExprNoIn", "ConditionalExprNoBF", "AssignmentExpr",
  "AssignmentExprNoIn", "AssignmentExprNoBF", "AssignmentOperator", "Expr",
  "ExprNoIn", "ExprNoBF", "Statement", "Block", "VariableStatement",
  "VariableDeclarationList", "VariableDeclarationListNoIn",
  "VariableDeclaration", "VariableDeclarationNoIn", "ConstStatement",
  "ConstDeclarationList", "ConstDeclaration", "Initializer",
  "InitializerNoIn", "EmptyStatement", "ExprStatement", "IfStatement",
  "IterationStatement", "ExprOpt", "ExprNoInOpt", "ContinueStatement",
  "BreakStatement", "ReturnStatement", "WithStatement", "SwitchStatement",
  "CaseBlock", "CaseClausesOpt", "CaseClauses", "CaseClause",
  "DefaultClause", "LabelledStatement", "ThrowStatement", "TryStatement",
  "DebuggerStatement", "PackageName", "ImportStatement",
  "FunctionDeclaration", "$@3", "$@4", "FunctionExpr", "$@5", "$@6", "$@7",
  "$@8", "FormalParameterList", "FunctionBody", "Program",
  "SourceElements", "SourceElement", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,    47,    58,    40,    41,    44,
     123,   125,    91,    93,    46,    43,    45,   126,    33,    42,
      37,    60,    62,    38,    94,   124,    63,    61,    59
};
# endif

#define YYPACT_NINF -424

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-424)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1118,  -424,  -424,  -424,     7,    15,   320,    17,    44,    25,
      58,   718,  1502,  1502,    37,  -424,  1198,    64,  1502,   116,
     137,  1502,   152,    67,   145,  1502,  1502,  -424,  -424,  -424,
     175,  1502,  1502,  -424,  1502,   798,   189,  1502,  1502,  1502,
    1502,  -424,  -424,  -424,  -424,    40,  -424,   117,   555,  -424,
    -424,  -424,   115,    71,    46,   165,   217,   161,   193,   209,
     242,     5,  -424,  -424,    39,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,   295,  1118,  -424,  -424,    74,  -424,
     226,   320,    -4,  -424,  1578,  -424,  -424,   149,  -424,  -424,
     210,    63,  -424,   210,    69,  -424,  -424,    75,  -424,   232,
    -424,  -424,   149,  -424,   153,   699,  -424,  -424,  -424,   135,
     157,   233,   170,   228,   224,   216,   241,   268,    13,  -424,
    -424,    87,   204,  -424,  -424,  1502,   301,  1502,  -424,  1502,
    1502,    89,   231,  -424,  -424,   246,    27,  -424,  -424,  1198,
    -424,  -424,   200,  -424,   878,  -424,    36,  1274,   259,  -424,
    -424,  -424,  -424,  1350,  1502,   656,  -424,  1502,   656,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  1502,  1502,  1502,  1502,  1502,  1502,
    1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,
    1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  -424,
    1502,  -424,  -424,  -424,  -424,  -424,   286,   362,   233,   178,
     236,   271,   280,   273,   322,    14,  -424,  -424,   297,   279,
     149,  -424,   302,    16,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,   656,  -424,  -424,
    -424,   304,  -424,    90,  1502,   656,  -424,  1502,  -424,  -424,
      17,  -424,  -424,  -424,    44,  -424,  -424,  -424,    35,  -424,
    1502,   656,  -424,  -424,  -424,  1502,  1502,  1502,  1502,  1502,
    1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,
    1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,
    -424,  1502,  -424,   215,   306,   237,   247,   267,  -424,  -424,
     307,   152,   309,  -424,     4,  -424,  -424,  -424,  -424,   189,
    -424,  -424,  -424,  -424,  -424,   269,  -424,   112,  -424,  -424,
     113,  -424,  -424,  -424,  -424,  -424,   135,   135,   157,   157,
     157,   233,   233,   233,   233,   233,   233,   170,   170,   170,
     170,   228,   224,   216,   241,   268,   312,  -424,    23,   -15,
    -424,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,
    1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,  1502,
    -424,    55,  -424,  -424,   272,   308,  1502,  1647,  -424,   155,
    -424,  -424,  -424,  -424,  -424,   274,   166,  -424,  -424,  -424,
    -424,  -424,   135,   135,   157,   157,   157,   233,   233,   233,
     233,   233,   233,   170,   170,   170,   170,   228,   224,   216,
     241,   268,   313,  -424,  1198,  1502,  1198,   298,  1198,   315,
    -424,  -424,    79,  -424,    76,  1426,  -424,  1502,  -424,  -424,
    1502,  1502,  1502,   364,   326,  1502,   276,   699,  -424,   233,
     233,   233,   233,   233,   178,   178,   178,   178,   236,   271,
     280,   273,   322,   323,  -424,   324,   303,  -424,   278,   325,
    -424,   332,   134,  -424,  -424,  -424,  -424,   325,  -424,  -424,
    1502,   361,   282,  -424,   390,  -424,  -424,   330,  -424,   337,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,   284,  -424,  1502,
     316,  -424,   317,  1198,  1502,  1502,   325,  -424,   958,  -424,
     325,  -424,  -424,   292,  -424,   325,  -424,  1198,    80,  1502,
      45,   390,  -424,   152,  1198,   294,  -424,  1502,  -424,  -424,
     336,  -424,   325,  -424,  1038,  -424,   325,  -424,  -424,  -424,
    -424,  -424,    56,   342,  -424,   390,  -424,   378,  -424,  1198,
     354,  1198,  -424,  -424,  -424,   325,  1118,  1118,   339,   152,
    -424,  1198,  -424,  -424,  1118,  1118,  -424,  -424,  -424
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     347,    35,    36,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    56,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    41,    38,    39,
      59,     0,     0,    40,     0,     0,    66,     0,     0,     0,
       0,   274,    57,    75,    58,    81,    97,    98,   102,   118,
     119,   124,   131,   138,   155,   172,   181,   187,   193,   199,
     205,   211,   217,   235,     0,   352,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   351,     0,   348,   349,   296,     0,   295,
     289,     0,     0,    59,     0,    70,    52,    79,    82,    71,
     262,     0,   258,   270,     0,   268,   292,     0,   291,     0,
     300,   299,    79,    95,    96,    99,   116,   117,   120,   128,
     134,   142,   162,   177,   183,   189,   195,   201,   207,   213,
     231,     0,    99,   106,   105,     0,     0,     0,   107,     0,
       0,     0,     0,   322,   321,   323,     0,   108,   110,     0,
     109,   111,     0,   254,     0,    68,     0,     0,    67,   112,
     113,   114,   115,     0,     0,     0,    87,     0,     0,    88,
     103,   104,   220,   221,   222,   223,   224,   225,   226,   227,
     230,   228,   229,   219,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   276,
       0,   275,     1,   350,   298,   297,     0,    99,   149,   167,
     179,   185,   191,   197,   203,   209,   215,   233,   290,     0,
      79,    80,     0,     0,    29,    30,    31,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    32,    44,    43,    33,    53,    34,
      42,     0,    50,     0,     0,     0,    78,     0,   263,   257,
       0,   256,   271,   267,     0,   266,   294,   293,     0,    83,
       0,     0,    84,   100,   101,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     302,     0,   301,     0,     0,     0,     0,     0,   317,   316,
       0,     0,     0,   328,     0,   327,   315,    60,   255,    66,
      62,    61,    64,    69,    91,     0,    93,     0,    33,    77,
       0,    90,   218,   126,   125,   127,   132,   133,   139,   140,
     141,   161,   160,   158,   159,   156,   157,   173,   174,   175,
     176,   182,   188,   194,   200,   206,     0,   236,   264,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   287,
      74,     0,   343,   335,     0,     0,     0,     0,    54,     0,
      73,   272,   259,   269,   331,     0,     0,    86,   214,   122,
     121,   123,   129,   130,   135,   136,   137,   148,   147,   145,
     146,   143,   144,   163,   164,   165,   166,   178,   184,   190,
     196,   202,     0,   232,     0,     0,     0,     0,     0,     0,
     318,   323,     0,   324,     0,     0,    92,     0,    76,    89,
       0,     0,     0,   265,     0,   287,     0,    99,   216,   154,
     152,   153,   150,   151,   168,   169,   170,   171,   180,   186,
     192,   198,   204,     0,   234,   288,     0,   339,     0,     0,
     337,     0,     0,    45,    55,    51,    72,     0,   333,    85,
       0,   277,     0,   281,   307,   304,   303,     0,   330,     0,
     329,   326,   325,    63,    65,    94,   212,     0,   273,     0,
     264,   261,     0,     0,     0,   287,     0,   341,     0,   336,
       0,   344,    46,     0,   332,     0,   208,     0,     0,     0,
       0,   308,   309,     0,     0,     0,   265,   287,   284,   210,
       0,   340,     0,   345,     0,   338,     0,    48,   334,   278,
     280,   279,     0,     0,   305,   307,   310,   319,   285,     0,
       0,     0,   342,   346,    47,     0,   311,   313,     0,     0,
     286,     0,   282,    49,   312,   314,   306,   320,   283
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -424,  -424,  -156,  -424,  -424,    18,  -424,  -424,  -424,  -424,
       0,  -424,  -424,    84,  -424,    42,  -424,    83,  -424,  -424,
    -424,   -40,  -424,   192,  -424,  -424,  -424,    11,    12,  -424,
    -155,  -424,  -173,  -424,   311,  -424,  -140,   -70,  -424,  -193,
      32,  -424,  -189,    33,  -424,  -176,    34,  -424,  -174,    41,
    -424,  -168,    31,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -143,  -376,  -424,   -17,     2,  -424,  -424,   -13,   -18,  -424,
    -424,  -424,   150,   -35,  -424,  -424,   148,   331,   -82,  -424,
    -424,  -424,  -424,  -423,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -128,  -424,  -102,  -424,  -424,  -424,  -424,  -424,   108,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -283,
    -318,  -424,   -34,   -83
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   269,   270,    42,   271,   272,   556,   575,   273,    95,
      96,    44,   156,   157,   158,   112,    45,   113,    46,   114,
      47,   166,   345,   132,    48,   116,    49,   117,   118,    51,
     119,    52,   120,    53,   121,    54,   122,   219,    55,   123,
     220,    56,   124,   221,    57,   125,   222,    58,   126,   223,
      59,   127,   224,    60,   128,   225,    61,   129,   226,    62,
     130,   227,    63,   382,   485,   228,    64,    65,    66,    67,
     101,   379,   102,   380,    68,   104,   105,   278,   463,    69,
      70,    71,    72,   486,   229,    73,    74,    75,    76,    77,
     505,   540,   541,   542,   565,    78,    79,    80,    81,   146,
      82,    83,   497,   535,    99,   489,   530,   526,   552,   404,
     529,    84,    85,    86
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      43,   154,   213,   136,   142,   415,   468,   169,    87,   349,
     371,    50,   351,   131,   342,   372,    43,   358,   359,   360,
     346,   483,   484,   141,   133,   134,   106,    50,   333,   373,
     138,   184,   374,   356,   357,    43,   152,   147,   148,   375,
     209,   352,   522,   150,   151,   461,    50,   207,    97,   159,
     160,   161,   162,   563,   464,   318,   396,   276,   232,   367,
     368,   369,   370,   233,   279,   376,   453,   377,   143,    88,
     283,   213,   289,   465,   292,   214,   286,   511,   402,   100,
     508,   560,    90,   454,   403,    43,   518,   107,   320,    98,
     328,   208,   190,   191,   192,    89,    50,   402,   295,   319,
     397,   334,   550,   414,   135,   339,   103,   163,   210,   340,
     462,   405,   164,   108,   165,   335,   564,   402,   488,   410,
     109,   437,   576,   487,   570,   321,   438,   211,   424,   425,
     426,   137,   280,   230,   411,   417,   336,   323,   284,   325,
     439,   326,   327,   440,   422,   423,   188,   189,   549,    43,
     441,   281,   418,   509,    43,   144,   321,   285,   321,   407,
      50,   408,   215,   287,   512,    50,   347,   510,   561,   350,
     433,   434,   435,   436,   231,   322,   442,   329,   443,   534,
     185,   321,   321,   139,   163,   458,   459,   193,   194,   167,
     400,   168,   304,   305,   186,   187,   402,   353,   354,   355,
     296,   383,   532,   115,   140,   195,   196,   145,   551,   533,
     306,   307,   555,   115,   297,   298,   163,   558,   384,   385,
     163,   274,    35,   275,   321,   290,   115,   291,   496,     1,
       2,     3,   299,   300,   572,   321,    91,   216,   574,   499,
      92,   149,    12,    13,   203,    15,   197,   198,   293,   294,
      18,   308,   309,   199,   200,   201,   202,   583,   155,   386,
     387,   330,   331,   493,   310,   311,   312,   313,   337,   321,
      25,    26,   388,   389,   390,   391,   409,   204,    27,   301,
     302,   303,   217,   444,   321,   206,    28,    29,    93,    31,
      32,    33,   416,    34,   205,   212,    94,   277,    36,   288,
     315,    37,    38,    39,    40,   446,   321,   314,   419,   420,
     421,   317,   514,   450,   515,   447,   321,   516,   474,   475,
     476,   477,   324,     1,     2,     3,   316,   115,   343,   115,
      91,   115,   115,   332,    92,   448,   321,   456,   457,    15,
     490,   491,   498,   491,   523,   321,   527,   491,   378,   115,
     538,   321,   544,   321,   392,   115,   115,   536,   394,   115,
     557,   491,   569,   321,   393,   395,   398,   399,   504,   401,
     406,   451,    27,   445,   449,   492,   115,   507,   460,   500,
      28,    29,    93,   466,   381,    33,   519,    34,   520,   524,
      94,   525,    36,   321,   531,   528,   537,   539,   543,   453,
     115,   218,   115,   462,   571,   547,   293,   294,   577,   579,
     586,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   581,   455,   478,   495,   479,   482,   480,   521,
     412,   501,   413,   503,   282,   506,   481,   578,   546,   566,
     452,     0,     0,     0,    43,     0,    43,   502,    43,   183,
       0,     0,     0,     0,     0,    50,     0,    50,     0,    50,
       0,     0,     0,   517,     0,     0,   115,     0,     0,   115,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   115,     0,     0,     0,     0,   115,     0,     0,
       0,     0,     0,     0,   554,     0,     0,     0,     0,     0,
       0,   213,   213,     0,   361,   362,   363,   364,   365,   366,
     548,   115,     0,   115,     0,     0,     0,     0,     0,     0,
       0,   545,     0,    43,   559,   567,     0,     0,    43,     0,
       0,   568,     0,     0,    50,     0,     0,    43,     0,    50,
       0,   562,   584,   585,    43,     0,     0,     0,    50,     0,
       0,     0,     0,     0,    43,    50,   580,     0,   582,     0,
       0,   587,     0,     0,     0,    50,     0,     0,   588,    43,
       0,    43,     0,   115,   467,     0,    43,    43,     0,     0,
      50,    43,    50,     0,    43,    43,     0,    50,    50,   467,
     467,   115,    50,     0,     0,    50,    50,     0,   115,   170,
     171,     0,     0,     0,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   427,   428,   429,   430,   431,
     432,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   115,     0,     0,
       0,     0,   183,     0,     0,     0,     0,   115,     0,   115,
       0,     0,   115,   115,   467,     0,     0,   115,     0,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,     0,   258,   259,   260,   261,   262,   263,
       0,   264,   115,   218,   469,   470,   471,   472,   473,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
       0,   115,     0,     0,     0,     0,   467,   115,   348,   110,
       0,     1,     2,     3,     0,     0,     0,     0,    91,     0,
       0,   115,    92,     0,    12,    13,     0,    15,     0,   115,
       0,     0,    18,   293,   294,     0,     0,     0,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,   218,     0,     0,     0,     0,    28,    29,
      93,    31,    32,    33,     0,    34,   183,     0,    94,     0,
      36,     0,     0,    37,    38,    39,    40,     0,     0,     0,
       0,     1,     2,     3,     4,     0,   111,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      23,    24,     0,     0,     0,   218,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    28,    29,
      30,    31,    32,    33,     0,    34,     0,     0,    35,   153,
      36,     0,     0,    37,    38,    39,    40,     0,     0,     0,
       0,     1,     2,     3,     4,     0,    41,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      23,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    28,    29,
      30,    31,    32,    33,     0,    34,     0,     0,    35,   338,
      36,     0,     0,    37,    38,    39,    40,     0,     0,     0,
       0,     1,     2,     3,     4,     0,    41,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      23,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    28,    29,
      30,    31,    32,    33,     0,    34,     0,     0,    35,   553,
      36,     0,     0,    37,    38,    39,    40,     0,     0,     0,
       0,     1,     2,     3,     4,     0,    41,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      23,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    28,    29,
      30,    31,    32,    33,     0,    34,     0,     0,    35,   573,
      36,     0,     0,    37,    38,    39,    40,     0,     0,     0,
       0,     1,     2,     3,     4,     0,    41,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      23,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    28,    29,
      30,    31,    32,    33,     0,    34,     0,     0,    35,     0,
      36,     0,     0,    37,    38,    39,    40,     0,     0,     0,
       0,     1,     2,     3,     4,     0,    41,     5,     6,     7,
       8,     9,     0,    11,    12,    13,    14,    15,    16,    17,
       0,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      23,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    28,    29,
      30,    31,    32,    33,     0,    34,     0,     0,    35,     0,
      36,     0,     0,    37,    38,    39,    40,     1,     2,     3,
       0,     0,     0,     0,    91,     0,    41,     0,    92,     0,
      12,    13,     0,    15,     0,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    26,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,     0,    28,    29,    93,    31,    32,    33,
       0,    34,     0,     0,    94,     0,    36,   341,     0,    37,
      38,    39,    40,     1,     2,     3,     0,     0,     0,     0,
      91,     0,     0,     0,    92,     0,    12,    13,     0,    15,
       0,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    93,    31,    32,    33,     0,    34,   344,     0,
      94,     0,    36,     0,     0,    37,    38,    39,    40,     1,
       2,     3,     0,     0,     0,     0,    91,     0,     0,     0,
      92,     0,    12,    13,     0,    15,     0,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    93,    31,
      32,    33,     0,    34,     0,     0,    94,     0,    36,   513,
       0,    37,    38,    39,    40,     1,     2,     3,     0,     0,
       0,     0,    91,     0,     0,     0,    92,     0,    12,    13,
       0,    15,     0,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    25,    26,     0,     0,
       0,     0,     0,     0,    27,     0,     0,     0,     0,     0,
       0,     0,    28,    29,    93,    31,    32,    33,     0,    34,
       0,     0,    94,     0,    36,     0,     0,    37,    38,    39,
      40,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,     0,   258,   259,   260,   261,
     262,   263,     0,   264,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   265,   266,
     267,     0,     0,     0,     0,     0,     0,     0,     0,   268,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,     0,   258,   259,   260,   261,   262,
     263,     0,   264,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   265,   266,   267,
       0,     0,     0,     0,     0,     0,     0,     0,   494
};

static const yytype_int16 yycheck[] =
{
       0,    35,    85,    16,    22,   288,   382,    47,     1,   165,
     203,     0,   168,    11,   157,   204,    16,   190,   191,   192,
     163,   397,   398,    21,    12,    13,     1,    16,     1,   205,
      18,    48,   206,   188,   189,    35,    34,    25,    26,   207,
       1,   184,   465,    31,    32,    22,    35,    42,     6,    37,
      38,    39,    40,     8,    69,    42,    42,    97,    62,   199,
     200,   201,   202,    67,     1,   208,    62,   210,     1,    62,
       1,   154,   112,    88,   114,     1,     1,     1,    62,    62,
       1,     1,    67,    79,    68,    85,   462,    62,     1,     6,
       1,    86,    46,    47,    48,    88,    85,    62,   115,    86,
      86,    74,   525,    68,    67,    69,    62,    67,    69,    73,
      87,   267,    72,    88,    74,    88,    71,    62,   401,   275,
      62,   314,    66,    68,   547,    69,   315,    88,   301,   302,
     303,    67,    69,    91,   277,   291,   149,   135,    69,   137,
     316,   139,   140,   317,   299,   300,    75,    76,   524,   149,
     318,    88,   295,    74,   154,    88,    69,    88,    69,    69,
     149,    71,    88,    88,    88,   154,   164,    88,    88,   167,
     310,   311,   312,   313,    91,    88,   319,    88,   321,   497,
      65,    69,    69,    67,    67,    73,    73,    22,    23,    72,
     230,    74,    22,    23,    79,    80,    62,   185,   186,   187,
      65,    23,    68,    11,    67,    40,    41,    62,   526,   492,
      40,    41,   530,    21,    79,    80,    67,   535,    40,    41,
      67,    72,    70,    74,    69,    72,    34,    74,    73,     3,
       4,     5,    75,    76,   552,    69,    10,    11,   556,    73,
      14,    66,    16,    17,    83,    19,    81,    82,    44,    45,
      24,    81,    82,    36,    37,    38,    39,   575,    69,    81,
      82,    30,    31,   406,    36,    37,    38,    39,    68,    69,
      44,    45,    36,    37,    38,    39,   274,    84,    52,    46,
      47,    48,    90,    68,    69,    43,    60,    61,    62,    63,
      64,    65,   290,    67,    85,     0,    70,    87,    72,    67,
      84,    75,    76,    77,    78,    68,    69,    83,   296,   297,
     298,    43,   455,   331,   457,    68,    69,   460,   388,   389,
     390,   391,    21,     3,     4,     5,    85,   135,    69,   137,
      10,   139,   140,    87,    14,    68,    69,    68,    69,    19,
      68,    69,    68,    69,    68,    69,    68,    69,    62,   157,
      68,    69,    68,    69,    83,   163,   164,   500,    85,   167,
      68,    69,    68,    69,    84,    43,    69,    88,    70,    67,
      66,    62,    52,    67,    67,    67,   184,    62,    66,    66,
      60,    61,    62,   381,    22,    65,    22,    67,    62,    66,
      70,    88,    72,    69,    62,    70,    35,     7,    68,    62,
     208,    90,   210,    87,    68,    88,    44,    45,    66,    31,
      71,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    68,   339,   392,   407,   393,   396,   394,   464,
     280,   444,   284,   446,   103,   448,   395,   565,   520,   541,
     332,    -1,    -1,    -1,   444,    -1,   446,   445,   448,    87,
      -1,    -1,    -1,    -1,    -1,   444,    -1,   446,    -1,   448,
      -1,    -1,    -1,   461,    -1,    -1,   274,    -1,    -1,   277,
      -1,   554,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   290,    -1,    -1,    -1,    -1,   295,    -1,    -1,
      -1,    -1,    -1,    -1,   528,    -1,    -1,    -1,    -1,    -1,
      -1,   584,   585,    -1,   193,   194,   195,   196,   197,   198,
     523,   319,    -1,   321,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   519,    -1,   523,   537,   543,    -1,    -1,   528,    -1,
      -1,   544,    -1,    -1,   523,    -1,    -1,   537,    -1,   528,
      -1,   539,   576,   577,   544,    -1,    -1,    -1,   537,    -1,
      -1,    -1,    -1,    -1,   554,   544,   569,    -1,   571,    -1,
      -1,   579,    -1,    -1,    -1,   554,    -1,    -1,   581,   569,
      -1,   571,    -1,   381,   382,    -1,   576,   577,    -1,    -1,
     569,   581,   571,    -1,   584,   585,    -1,   576,   577,   397,
     398,   399,   581,    -1,    -1,   584,   585,    -1,   406,    44,
      45,    -1,    -1,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,   304,   305,   306,   307,   308,
     309,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   445,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,   455,    -1,   457,
      -1,    -1,   460,   461,   462,    -1,    -1,   465,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    28,    29,    30,    31,    32,    33,
      -1,    35,   500,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,   392,   393,   394,   395,   396,   397,   398,
      -1,   519,    -1,    -1,    -1,    -1,   524,   525,    62,     1,
      -1,     3,     4,     5,    -1,    -1,    -1,    -1,    10,    -1,
      -1,   539,    14,    -1,    16,    17,    -1,    19,    -1,   547,
      -1,    -1,    24,    44,    45,    -1,    -1,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,   462,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    -1,    67,    87,    -1,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    88,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,
      32,    33,    -1,    -1,    -1,   524,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    70,    71,
      72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    88,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    70,    71,
      72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    88,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    70,    71,
      72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    88,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    70,    71,
      72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    88,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    88,     9,    10,    11,
      12,    13,    -1,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    70,    -1,
      72,    -1,    -1,    75,    76,    77,    78,     3,     4,     5,
      -1,    -1,    -1,    -1,    10,    -1,    88,    -1,    14,    -1,
      16,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,    65,
      -1,    67,    -1,    -1,    70,    -1,    72,    73,    -1,    75,
      76,    77,    78,     3,     4,     5,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    14,    -1,    16,    17,    -1,    19,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    -1,
      70,    -1,    72,    -1,    -1,    75,    76,    77,    78,     3,
       4,     5,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      14,    -1,    16,    17,    -1,    19,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    70,    -1,    72,    73,
      -1,    75,    76,    77,    78,     3,     4,     5,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    14,    -1,    16,    17,
      -1,    19,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62,    63,    64,    65,    -1,    67,
      -1,    -1,    70,    -1,    72,    -1,    -1,    75,    76,    77,
      78,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    28,    29,    30,    31,
      32,    33,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    28,    29,    30,    31,    32,
      33,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    24,    25,
      26,    28,    29,    32,    33,    44,    45,    52,    60,    61,
      62,    63,    64,    65,    67,    70,    72,    75,    76,    77,
      78,    88,    92,    99,   100,   105,   107,   109,   113,   115,
     116,   118,   120,   122,   124,   127,   130,   133,   136,   139,
     142,   145,   148,   151,   155,   156,   157,   158,   163,   168,
     169,   170,   171,   174,   175,   176,   177,   178,   184,   185,
     186,   187,   189,   190,   200,   201,   202,     1,    62,    88,
      67,    10,    14,    62,    70,    98,    99,   104,   106,   193,
      62,   159,   161,    62,   164,   165,     1,    62,    88,    62,
       1,    88,   104,   106,   108,   112,   114,   116,   117,   119,
     121,   123,   125,   128,   131,   134,   137,   140,   143,   146,
     149,   153,   112,   117,   117,    67,   156,    67,   117,    67,
      67,   153,   157,     1,    88,    62,   188,   117,   117,    66,
     117,   117,   153,    71,   201,    69,   101,   102,   103,   117,
     117,   117,   117,    67,    72,    74,   110,    72,    74,   110,
      44,    45,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    87,   152,    65,    79,    80,    75,    76,
      46,    47,    48,    22,    23,    40,    41,    81,    82,    36,
      37,    38,    39,    83,    84,    85,    43,    42,    86,     1,
      69,    88,     0,   202,     1,    88,    11,   112,   123,   126,
     129,   132,   135,   138,   141,   144,   147,   150,   154,   173,
     104,   106,    62,    67,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    28,    29,
      30,    31,    32,    33,    35,    60,    61,    62,    71,    90,
      91,    93,    94,    97,    72,    74,   110,    87,   166,     1,
      69,    88,   166,     1,    69,    88,     1,    88,    67,   110,
      72,    74,   110,    44,    45,   152,    65,    79,    80,    75,
      76,    46,    47,    48,    22,    23,    40,    41,    81,    82,
      36,    37,    38,    39,    83,    84,    85,    43,    42,    86,
       1,    69,    88,   153,    21,   153,   153,   153,     1,    88,
      30,    31,    87,     1,    74,    88,   156,    68,    71,    69,
      73,    73,   149,    69,    68,   111,   149,   153,    62,    91,
     153,    91,   149,   117,   117,   117,   119,   119,   121,   121,
     121,   123,   123,   123,   123,   123,   123,   125,   125,   125,
     125,   128,   131,   134,   137,   140,   149,   149,    62,   160,
     162,    22,   152,    23,    40,    41,    81,    82,    36,    37,
      38,    39,    83,    84,    85,    43,    42,    86,    69,    88,
     110,    67,    62,    68,   198,    91,    66,    69,    71,   153,
      91,   149,   161,   165,    68,   198,   153,    91,   149,   117,
     117,   117,   119,   119,   121,   121,   121,   123,   123,   123,
     123,   123,   123,   125,   125,   125,   125,   128,   131,   134,
     137,   140,   149,   149,    68,    67,    68,    68,    68,    67,
     157,    62,   188,    62,    79,   102,    68,    69,    73,    73,
      66,    22,    87,   167,    69,    88,   153,   112,   150,   123,
     123,   123,   123,   123,   126,   126,   126,   126,   129,   132,
     135,   138,   141,   150,   150,   153,   172,    68,   198,   194,
      68,    69,    67,   149,    71,    94,    73,   191,    68,    73,
      66,   156,   153,   156,    70,   179,   156,    62,     1,    74,
      88,     1,    88,    73,   149,   149,   149,   153,   150,    22,
      62,   162,   172,    68,    66,    88,   196,    68,    70,   199,
     195,    62,    68,   198,   199,   192,   149,    35,    68,     7,
     180,   181,   182,    68,    68,   153,   167,    88,   156,   150,
     172,   199,   197,    71,   201,   199,    95,    68,   199,   156,
       1,    88,   153,     8,    71,   183,   182,   157,   156,    68,
     172,    68,   199,    71,   199,    96,    66,    66,   180,    31,
     156,    68,   156,   199,   201,   201,    71,   157,   156
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    91,    91,    92,    92,    92,    92,    92,
      92,    92,    93,    93,    93,    94,    95,    94,    96,    94,
      97,    97,    98,    98,    98,    98,    99,    99,    99,    99,
      99,   100,   100,   100,   101,   101,   102,   102,   103,   103,
     104,   104,   104,   104,   104,   105,   105,   105,   105,   106,
     106,   107,   107,   108,   108,   108,   108,   109,   109,   109,
     109,   110,   110,   111,   111,   112,   112,   113,   113,   114,
     114,   114,   115,   115,   115,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   117,   117,   118,   118,
     119,   119,   119,   119,   120,   120,   120,   120,   121,   121,
     121,   122,   122,   122,   123,   123,   123,   123,   124,   124,
     124,   124,   125,   125,   125,   125,   125,   125,   125,   126,
     126,   126,   126,   126,   126,   127,   127,   127,   127,   127,
     127,   127,   128,   128,   128,   128,   128,   129,   129,   129,
     129,   129,   130,   130,   130,   130,   130,   131,   131,   132,
     132,   133,   133,   134,   134,   135,   135,   136,   136,   137,
     137,   138,   138,   139,   139,   140,   140,   141,   141,   142,
     142,   143,   143,   144,   144,   145,   145,   146,   146,   147,
     147,   148,   148,   149,   149,   150,   150,   151,   151,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   153,   153,   154,   154,   155,   155,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   157,   157,   158,   158,   159,   159,
     160,   160,   161,   161,   162,   162,   163,   163,   164,   164,
     165,   165,   166,   167,   168,   169,   169,   170,   170,   171,
     171,   171,   171,   171,   171,   171,   171,   172,   172,   173,
     173,   174,   174,   174,   174,   175,   175,   175,   175,   176,
     176,   176,   176,   177,   178,   179,   179,   180,   180,   181,
     181,   182,   182,   183,   183,   184,   185,   185,   186,   186,
     186,   187,   187,   188,   188,   189,   189,   189,   189,   189,
     189,   191,   190,   192,   190,   194,   193,   195,   193,   196,
     193,   197,   193,   198,   198,   199,   199,   200,   200,   201,
     201,   202,   202
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     0,     6,     0,     7,
       1,     3,     1,     2,     3,     4,     1,     1,     1,     1,
       3,     3,     3,     5,     2,     4,     0,     1,     1,     2,
       1,     1,     4,     3,     3,     1,     4,     3,     3,     1,
       2,     1,     2,     2,     2,     4,     3,     2,     2,     4,
       3,     2,     3,     1,     3,     1,     1,     1,     1,     1,
       2,     2,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     3,     3,     3,     1,     3,     3,     3,     1,     3,
       3,     1,     3,     3,     1,     3,     3,     3,     1,     3,
       3,     3,     1,     3,     3,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     3,     1,     3,     3,     3,     3,
       3,     3,     1,     3,     3,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     3,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     5,     1,
       5,     1,     5,     1,     3,     1,     3,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     3,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     3,     3,     1,     3,
       1,     3,     1,     2,     1,     2,     3,     3,     1,     3,
       1,     2,     2,     2,     1,     2,     2,     5,     7,     7,
       7,     5,     9,    10,     7,     8,     9,     0,     1,     0,
       1,     2,     2,     3,     3,     2,     2,     3,     3,     2,
       2,     3,     3,     5,     5,     3,     5,     0,     1,     1,
       2,     3,     4,     2,     3,     3,     3,     3,     4,     7,
       9,     2,     2,     1,     3,     5,     5,     3,     3,     5,
       5,     0,     6,     0,     7,     0,     5,     0,     6,     0,
       6,     0,     7,     1,     3,     2,     3,     0,     1,     1,
       2,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 192 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("break"); }
#line 2161 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 193 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("case"); }
#line 2167 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 194 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("default"); }
#line 2173 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 195 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("for"); }
#line 2179 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 196 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("new"); }
#line 2185 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 197 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("var"); }
#line 2191 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 198 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("const"); }
#line 2197 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 199 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("continue"); }
#line 2203 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 200 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("function"); }
#line 2209 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 201 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("return"); }
#line 2215 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 202 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("void"); }
#line 2221 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 203 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("delete"); }
#line 2227 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 204 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("if"); }
#line 2233 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 205 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("this"); }
#line 2239 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 206 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("do"); }
#line 2245 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 207 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("while"); }
#line 2251 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 208 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("in"); }
#line 2257 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 209 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("instanceof"); }
#line 2263 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 210 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("typeof"); }
#line 2269 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 211 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("switch"); }
#line 2275 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 212 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("with"); }
#line 2281 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 213 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("throw"); }
#line 2287 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 214 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("try"); }
#line 2293 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 215 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("catch"); }
#line 2299 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 216 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("finally"); }
#line 2305 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 217 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("debugger"); }
#line 2311 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 218 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("import"); }
#line 2317 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 219 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("null"); }
#line 2323 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 220 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("true"); }
#line 2329 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 221 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("false"); }
#line 2335 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 222 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = new Identifier("else"); }
#line 2341 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 226 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = (yyvsp[0].ident); }
#line 2347 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 227 "grammar.y" /* yacc.c:1646  */
    { (yyval.ident) = (yyvsp[0].ident); }
#line 2353 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 231 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new NullNode(); }
#line 2359 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 232 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new BooleanNode(true); }
#line 2365 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 233 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new BooleanNode(false); }
#line 2371 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 234 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new NumberNode((yyvsp[0].dval)); }
#line 2377 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 235 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new StringNode((yyvsp[0].ustr)); }
#line 2383 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 236 "grammar.y" /* yacc.c:1646  */
    {
                                            Lexer& l = lexer();
                                            if (!l.scanRegExp())
                                                YYABORT;
                                            (yyval.node) = new RegExpNode(l.pattern(), l.flags());
                                        }
#line 2394 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 242 "grammar.y" /* yacc.c:1646  */
    {
                                            Lexer& l = lexer();
                                            if (!l.scanRegExp())
                                                YYABORT;
                                            (yyval.node) = new RegExpNode("=" + l.pattern(), l.flags());
                                        }
#line 2405 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 251 "grammar.y" /* yacc.c:1646  */
    { (yyval.pname) = new PropertyNameNode(*(yyvsp[0].ident)); }
#line 2411 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 252 "grammar.y" /* yacc.c:1646  */
    { (yyval.pname) = new PropertyNameNode(Identifier(*(yyvsp[0].ustr))); }
#line 2417 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 253 "grammar.y" /* yacc.c:1646  */
    { (yyval.pname) = new PropertyNameNode(Identifier(UString::from((yyvsp[0].dval)))); }
#line 2423 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 257 "grammar.y" /* yacc.c:1646  */
    { (yyval.pnode) = new PropertyNode((yyvsp[-2].pname), (yyvsp[0].node), PropertyNode::Constant); }
#line 2429 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 258 "grammar.y" /* yacc.c:1646  */
    {inFuncExpr();}
#line 2435 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 258 "grammar.y" /* yacc.c:1646  */
    {
          if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[-5].ident), *(yyvsp[-4].ident), 0, (yyvsp[0].body)))
            YYABORT;
        }
#line 2444 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 262 "grammar.y" /* yacc.c:1646  */
    {inFuncExpr();}
#line 2450 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 262 "grammar.y" /* yacc.c:1646  */
    {
          if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[-6].ident), *(yyvsp[-5].ident), (yyvsp[-3].param), (yyvsp[0].body)))
            YYABORT;
        }
#line 2459 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 269 "grammar.y" /* yacc.c:1646  */
    { (yyval.plist) = new PropertyListNode((yyvsp[0].pnode)); }
#line 2465 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 270 "grammar.y" /* yacc.c:1646  */
    { (yyval.plist) = new PropertyListNode((yyvsp[0].pnode), (yyvsp[-2].plist)); }
#line 2471 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 275 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new ObjectLiteralNode(); }
#line 2477 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 276 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-1].plist)); }
#line 2483 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 278 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-2].plist)); }
#line 2489 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 282 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new ThisNode(); }
#line 2495 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 285 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new VarAccessNode(*(yyvsp[0].ident)); }
#line 2501 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 286 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeGroupNode((yyvsp[-1].node)); }
#line 2507 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 291 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival)); }
#line 2513 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 292 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new ArrayNode((yyvsp[-1].elm)); }
#line 2519 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 293 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival), (yyvsp[-3].elm)); }
#line 2525 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 297 "grammar.y" /* yacc.c:1646  */
    { (yyval.elm) = new ElementNode((yyvsp[-1].ival), (yyvsp[0].node)); }
#line 2531 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 299 "grammar.y" /* yacc.c:1646  */
    { (yyval.elm) = new ElementNode((yyvsp[-3].elm), (yyvsp[-1].ival), (yyvsp[0].node)); }
#line 2537 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 303 "grammar.y" /* yacc.c:1646  */
    { (yyval.ival) = 0; }
#line 2543 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 308 "grammar.y" /* yacc.c:1646  */
    { (yyval.ival) = 1; }
#line 2549 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 309 "grammar.y" /* yacc.c:1646  */
    { (yyval.ival) = (yyvsp[-1].ival) + 1; }
#line 2555 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 314 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].funcExpr); }
#line 2561 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 315 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2567 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 316 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); }
#line 2573 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 317 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new NewExprNode((yyvsp[-1].node), (yyvsp[0].args)); }
#line 2579 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 322 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2585 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 323 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); }
#line 2591 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 324 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new NewExprNode((yyvsp[-1].node), (yyvsp[0].args)); }
#line 2597 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 329 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new NewExprNode((yyvsp[0].node)); }
#line 2603 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 334 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new NewExprNode((yyvsp[0].node)); }
#line 2609 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 338 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); }
#line 2615 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 339 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); }
#line 2621 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 340 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2627 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 341 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); }
#line 2633 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 345 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); }
#line 2639 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 346 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); }
#line 2645 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 347 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 2651 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 348 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); }
#line 2657 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 352 "grammar.y" /* yacc.c:1646  */
    { (yyval.args) = new ArgumentsNode(); }
#line 2663 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 353 "grammar.y" /* yacc.c:1646  */
    { (yyval.args) = new ArgumentsNode((yyvsp[-1].alist)); }
#line 2669 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 357 "grammar.y" /* yacc.c:1646  */
    { (yyval.alist) = new ArgumentListNode((yyvsp[0].node)); }
#line 2675 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 358 "grammar.y" /* yacc.c:1646  */
    { (yyval.alist) = new ArgumentListNode((yyvsp[-2].alist), (yyvsp[0].node)); }
#line 2681 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 373 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makePostfixNode((yyvsp[-1].node), OpPlusPlus); }
#line 2687 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 374 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makePostfixNode((yyvsp[-1].node), OpMinusMinus); }
#line 2693 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 379 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makePostfixNode((yyvsp[-1].node), OpPlusPlus); }
#line 2699 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 380 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makePostfixNode((yyvsp[-1].node), OpMinusMinus); }
#line 2705 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 384 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeDeleteNode((yyvsp[0].node)); }
#line 2711 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 385 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new VoidNode((yyvsp[0].node)); }
#line 2717 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 386 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeTypeOfNode((yyvsp[0].node)); }
#line 2723 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 387 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makePrefixNode((yyvsp[0].node), OpPlusPlus); }
#line 2729 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 388 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makePrefixNode((yyvsp[0].node), OpPlusPlus); }
#line 2735 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 389 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makePrefixNode((yyvsp[0].node), OpMinusMinus); }
#line 2741 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 390 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makePrefixNode((yyvsp[0].node), OpMinusMinus); }
#line 2747 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 391 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeUnaryPlusNode((yyvsp[0].node)); }
#line 2753 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 392 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeNegateNode((yyvsp[0].node)); }
#line 2759 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 393 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitwiseNotNode((yyvsp[0].node)); }
#line 2765 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 394 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeLogicalNotNode((yyvsp[0].node)); }
#line 2771 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 408 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeMultNode((yyvsp[-2].node), (yyvsp[0].node), OpMult); }
#line 2777 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 409 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeMultNode((yyvsp[-2].node), (yyvsp[0].node), OpDiv); }
#line 2783 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 410 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeMultNode((yyvsp[-2].node), (yyvsp[0].node), OpMod); }
#line 2789 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 416 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeMultNode((yyvsp[-2].node), (yyvsp[0].node), OpMult); }
#line 2795 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 418 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeMultNode((yyvsp[-2].node), (yyvsp[0].node), OpDiv); }
#line 2801 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 420 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeMultNode((yyvsp[-2].node), (yyvsp[0].node), OpMod); }
#line 2807 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 425 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeAddNode((yyvsp[-2].node), (yyvsp[0].node), OpPlus); }
#line 2813 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 426 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeAddNode((yyvsp[-2].node), (yyvsp[0].node), OpMinus); }
#line 2819 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 432 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeAddNode((yyvsp[-2].node), (yyvsp[0].node), OpPlus); }
#line 2825 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 434 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeAddNode((yyvsp[-2].node), (yyvsp[0].node), OpMinus); }
#line 2831 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 439 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeShiftNode((yyvsp[-2].node), (yyvsp[0].node), OpLShift); }
#line 2837 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 440 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeShiftNode((yyvsp[-2].node), (yyvsp[0].node), OpRShift); }
#line 2843 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 441 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeShiftNode((yyvsp[-2].node), (yyvsp[0].node), OpURShift); }
#line 2849 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 446 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeShiftNode((yyvsp[-2].node), (yyvsp[0].node), OpLShift); }
#line 2855 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 447 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeShiftNode((yyvsp[-2].node), (yyvsp[0].node), OpRShift); }
#line 2861 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 448 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeShiftNode((yyvsp[-2].node), (yyvsp[0].node), OpURShift); }
#line 2867 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 453 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); }
#line 2873 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 454 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); }
#line 2879 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 455 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); }
#line 2885 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 456 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); }
#line 2891 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 457 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); }
#line 2897 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 458 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpIn, (yyvsp[0].node)); }
#line 2903 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 463 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); }
#line 2909 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 464 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); }
#line 2915 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 465 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); }
#line 2921 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 466 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); }
#line 2927 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 468 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); }
#line 2933 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 473 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); }
#line 2939 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 474 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); }
#line 2945 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 475 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); }
#line 2951 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 476 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); }
#line 2957 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 478 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); }
#line 2963 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 479 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeRelationalNode((yyvsp[-2].node), OpIn, (yyvsp[0].node)); }
#line 2969 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 484 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); }
#line 2975 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 485 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); }
#line 2981 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 486 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); }
#line 2987 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 487 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));}
#line 2993 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 493 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); }
#line 2999 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 495 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); }
#line 3005 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 497 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); }
#line 3011 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 499 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));}
#line 3017 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 505 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); }
#line 3023 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 506 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); }
#line 3029 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 508 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); }
#line 3035 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 510 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeEqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));}
#line 3041 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 515 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); }
#line 3047 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 521 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); }
#line 3053 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 526 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); }
#line 3059 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 531 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); }
#line 3065 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 537 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); }
#line 3071 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 543 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); }
#line 3077 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 548 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); }
#line 3083 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 554 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); }
#line 3089 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 560 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); }
#line 3095 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 565 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); }
#line 3101 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 571 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); }
#line 3107 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 577 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); }
#line 3113 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 582 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); }
#line 3119 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 588 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); }
#line 3125 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 593 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); }
#line 3131 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 599 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3137 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 605 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3143 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 611 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 3149 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 617 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeAssignNode((yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node)); }
#line 3155 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 623 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeAssignNode((yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node)); }
#line 3161 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 629 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = makeAssignNode((yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node)); }
#line 3167 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 633 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpEqual; }
#line 3173 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 634 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpPlusEq; }
#line 3179 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 635 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpMinusEq; }
#line 3185 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 636 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpMultEq; }
#line 3191 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 637 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpDivEq; }
#line 3197 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 638 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpLShift; }
#line 3203 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 639 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpRShift; }
#line 3209 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 640 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpURShift; }
#line 3215 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 641 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpAndEq; }
#line 3221 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 642 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpXOrEq; }
#line 3227 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 643 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpOrEq; }
#line 3233 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 644 "grammar.y" /* yacc.c:1646  */
    { (yyval.op) = OpModEq; }
#line 3239 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 649 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3245 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 654 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3251 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 659 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 3257 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 683 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); }
#line 3263 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 684 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new BlockNode((yyvsp[-1].srcs)); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); }
#line 3269 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 688 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); }
#line 3275 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 689 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3281 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 693 "grammar.y" /* yacc.c:1646  */
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); }
#line 3287 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 695 "grammar.y" /* yacc.c:1646  */
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); }
#line 3293 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 699 "grammar.y" /* yacc.c:1646  */
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); }
#line 3299 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 701 "grammar.y" /* yacc.c:1646  */
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); }
#line 3305 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 705 "grammar.y" /* yacc.c:1646  */
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Variable); }
#line 3311 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 706 "grammar.y" /* yacc.c:1646  */
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Variable); }
#line 3317 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 710 "grammar.y" /* yacc.c:1646  */
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Variable); }
#line 3323 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 711 "grammar.y" /* yacc.c:1646  */
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Variable); }
#line 3329 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 715 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); }
#line 3335 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 717 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3341 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 721 "grammar.y" /* yacc.c:1646  */
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); }
#line 3347 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 723 "grammar.y" /* yacc.c:1646  */
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); }
#line 3353 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 727 "grammar.y" /* yacc.c:1646  */
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Constant); }
#line 3359 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 728 "grammar.y" /* yacc.c:1646  */
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Constant); }
#line 3365 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 732 "grammar.y" /* yacc.c:1646  */
    { (yyval.init) = new AssignExprNode((yyvsp[0].node)); }
#line 3371 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 736 "grammar.y" /* yacc.c:1646  */
    { (yyval.init) = new AssignExprNode((yyvsp[0].node)); }
#line 3377 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 740 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new EmptyStatementNode(); }
#line 3383 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 744 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ExprStatementNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); }
#line 3389 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 745 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ExprStatementNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3395 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 750 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeIfNode((yyvsp[-2].node), (yyvsp[0].stat), 0); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); }
#line 3401 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 752 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeIfNode((yyvsp[-4].node), (yyvsp[-2].stat), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-6]), (yylsp[-3])); }
#line 3407 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 756 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new DoWhileNode((yyvsp[-5].stat), (yyvsp[-2].node)); DBG((yyval.stat), (yylsp[-6]), (yylsp[-4]));}
#line 3413 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 757 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new DoWhileNode((yyvsp[-5].stat), (yyvsp[-2].node)); DBG((yyval.stat), (yylsp[-6]), (yylsp[-4])); AUTO_SEMICOLON; }
#line 3419 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 758 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new WhileNode((yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); }
#line 3425 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 760 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ForNode((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-1])); }
#line 3431 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 762 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ForNode((yyvsp[-6].vlist), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-9]), (yylsp[-1])); }
#line 3437 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 764 "grammar.y" /* yacc.c:1646  */
    {
                                            Node *n = (yyvsp[-4].node)->nodeInsideAllParens();
                                            if (!n->isLocation())
                                                YYABORT;
                                            (yyval.stat) = new ForInNode(n, (yyvsp[-2].node), (yyvsp[0].stat));
                                            DBG((yyval.stat), (yylsp[-6]), (yylsp[-1]));
                                        }
#line 3449 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 772 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ForInNode(*(yyvsp[-4].ident), 0, (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-7]), (yylsp[-1])); }
#line 3455 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 774 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ForInNode(*(yyvsp[-5].ident), (yyvsp[-4].init), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-1])); }
#line 3461 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 778 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = 0; }
#line 3467 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 783 "grammar.y" /* yacc.c:1646  */
    { (yyval.node) = 0; }
#line 3473 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 788 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); }
#line 3479 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 292:
#line 789 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3485 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 293:
#line 790 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); }
#line 3491 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 294:
#line 791 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3497 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 795 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); }
#line 3503 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 296:
#line 796 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3509 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 297:
#line 797 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); }
#line 3515 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 298:
#line 798 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3521 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 299:
#line 802 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); }
#line 3527 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 300:
#line 803 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3533 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 301:
#line 804 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); }
#line 3539 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 302:
#line 805 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3545 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 303:
#line 809 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new WithNode((yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); }
#line 3551 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 304:
#line 813 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new SwitchNode((yyvsp[-2].node), (yyvsp[0].cblk)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); }
#line 3557 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 305:
#line 817 "grammar.y" /* yacc.c:1646  */
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-1].clist), 0, 0); }
#line 3563 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 306:
#line 819 "grammar.y" /* yacc.c:1646  */
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-3].clist), (yyvsp[-2].ccl), (yyvsp[-1].clist)); }
#line 3569 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 307:
#line 823 "grammar.y" /* yacc.c:1646  */
    { (yyval.clist) = 0; }
#line 3575 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 309:
#line 828 "grammar.y" /* yacc.c:1646  */
    { (yyval.clist) = new ClauseListNode((yyvsp[0].ccl)); }
#line 3581 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 310:
#line 829 "grammar.y" /* yacc.c:1646  */
    { (yyval.clist) = new ClauseListNode((yyvsp[-1].clist), (yyvsp[0].ccl)); }
#line 3587 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 311:
#line 833 "grammar.y" /* yacc.c:1646  */
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-1].node)); }
#line 3593 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 312:
#line 834 "grammar.y" /* yacc.c:1646  */
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-2].node), (yyvsp[0].srcs)); }
#line 3599 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 313:
#line 838 "grammar.y" /* yacc.c:1646  */
    { (yyval.ccl) = new CaseClauseNode(0); }
#line 3605 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 314:
#line 839 "grammar.y" /* yacc.c:1646  */
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[0].srcs)); }
#line 3611 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 315:
#line 843 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeLabelNode(*(yyvsp[-2].ident), (yyvsp[0].stat)); }
#line 3617 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 316:
#line 847 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); }
#line 3623 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 317:
#line 848 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3629 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 318:
#line 852 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new TryNode((yyvsp[-2].stat), CommonIdentifiers::shared()->nullIdentifier, 0, (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-3]), (yylsp[-2])); }
#line 3635 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 319:
#line 853 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new TryNode((yyvsp[-5].stat), *(yyvsp[-2].ident), (yyvsp[0].stat), 0); DBG((yyval.stat), (yylsp[-6]), (yylsp[-5])); }
#line 3641 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 320:
#line 855 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new TryNode((yyvsp[-7].stat), *(yyvsp[-4].ident), (yyvsp[-2].stat), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-7])); }
#line 3647 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 321:
#line 859 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); }
#line 3653 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 322:
#line 860 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; }
#line 3659 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 323:
#line 864 "grammar.y" /* yacc.c:1646  */
    { (yyval.pkgn) = new PackageNameNode(*(yyvsp[0].ident)); }
#line 3665 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 324:
#line 865 "grammar.y" /* yacc.c:1646  */
    { (yyval.pkgn) = new PackageNameNode((yyvsp[-2].pkgn), *(yyvsp[0].ident)); }
#line 3671 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 325:
#line 869 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeImportNode((yyvsp[-3].pkgn), true, 0);
                                          DBG((yyval.stat), (yylsp[-4]), (yylsp[0])); }
#line 3678 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 326:
#line 871 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeImportNode((yyvsp[-3].pkgn), true, 0);
                                          DBG((yyval.stat), (yylsp[-4]), (yylsp[0])); AUTO_SEMICOLON; }
#line 3685 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 327:
#line 873 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeImportNode((yyvsp[-1].pkgn), false, 0);
                                          DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); }
#line 3692 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 328:
#line 875 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeImportNode((yyvsp[-1].pkgn), false, 0);
                                          DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); AUTO_SEMICOLON; }
#line 3699 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 329:
#line 877 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeImportNode((yyvsp[-1].pkgn), false, *(yyvsp[-3].ident));
                                          DBG((yyval.stat), (yylsp[-4]), (yylsp[0])); }
#line 3706 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 330:
#line 879 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = makeImportNode((yyvsp[-1].pkgn), false, *(yyvsp[-3].ident));
                                          DBG((yyval.stat), (yylsp[-4]), (yylsp[0])); AUTO_SEMICOLON; }
#line 3713 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 331:
#line 884 "grammar.y" /* yacc.c:1646  */
    {inFuncDecl();}
#line 3719 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 332:
#line 884 "grammar.y" /* yacc.c:1646  */
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-4].ident), (yyvsp[0].body)); }
#line 3725 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 333:
#line 885 "grammar.y" /* yacc.c:1646  */
    {inFuncDecl();}
#line 3731 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 334:
#line 886 "grammar.y" /* yacc.c:1646  */
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-5].ident), (yyvsp[-3].param), (yyvsp[0].body)); }
#line 3737 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 335:
#line 890 "grammar.y" /* yacc.c:1646  */
    {inFuncExpr();}
#line 3743 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 336:
#line 890 "grammar.y" /* yacc.c:1646  */
    {
      (yyval.funcExpr) = new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, (yyvsp[0].body));
    }
#line 3751 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 337:
#line 893 "grammar.y" /* yacc.c:1646  */
    {inFuncExpr();}
#line 3757 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 338:
#line 893 "grammar.y" /* yacc.c:1646  */
    {
      (yyval.funcExpr) = new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, (yyvsp[0].body), (yyvsp[-3].param));
    }
#line 3765 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 339:
#line 896 "grammar.y" /* yacc.c:1646  */
    {inFuncExpr();}
#line 3771 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 340:
#line 896 "grammar.y" /* yacc.c:1646  */
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[-4].ident), (yyvsp[0].body)); }
#line 3777 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 341:
#line 897 "grammar.y" /* yacc.c:1646  */
    {inFuncExpr();}
#line 3783 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 342:
#line 897 "grammar.y" /* yacc.c:1646  */
    {
      (yyval.funcExpr) = new FuncExprNode(*(yyvsp[-5].ident), (yyvsp[0].body), (yyvsp[-3].param));
    }
#line 3791 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 343:
#line 903 "grammar.y" /* yacc.c:1646  */
    { (yyval.param) = new ParameterNode(*(yyvsp[0].ident)); }
#line 3797 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 344:
#line 904 "grammar.y" /* yacc.c:1646  */
    { (yyval.param) = new ParameterNode((yyvsp[-2].param), *(yyvsp[0].ident)); }
#line 3803 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 345:
#line 908 "grammar.y" /* yacc.c:1646  */
    { (yyval.body) = new FunctionBodyNode(0); DBG((yyval.body), (yylsp[-1]), (yylsp[0])); }
#line 3809 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 346:
#line 909 "grammar.y" /* yacc.c:1646  */
    { (yyval.body) = new FunctionBodyNode((yyvsp[-1].srcs)); DBG((yyval.body), (yylsp[-2]), (yylsp[0])); }
#line 3815 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 347:
#line 913 "grammar.y" /* yacc.c:1646  */
    { parser().didFinishParsing(new ProgramNode(0)); }
#line 3821 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 348:
#line 914 "grammar.y" /* yacc.c:1646  */
    { parser().didFinishParsing(new ProgramNode((yyvsp[0].srcs))); }
#line 3827 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 349:
#line 918 "grammar.y" /* yacc.c:1646  */
    { (yyval.srcs) = new SourceElementsNode((yyvsp[0].stat)); }
#line 3833 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 350:
#line 919 "grammar.y" /* yacc.c:1646  */
    { (yyval.srcs) = new SourceElementsNode((yyvsp[-1].srcs), (yyvsp[0].stat)); }
#line 3839 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 351:
#line 923 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = (yyvsp[0].func); }
#line 3845 "grammar.tab.c" /* yacc.c:1646  */
    break;

  case 352:
#line 924 "grammar.y" /* yacc.c:1646  */
    { (yyval.stat) = (yyvsp[0].stat); }
#line 3851 "grammar.tab.c" /* yacc.c:1646  */
    break;


#line 3855 "grammar.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 927 "grammar.y" /* yacc.c:1906  */


/* called by yyparse on error */
int yyerror(const char *)
{
// fprintf(stderr, "ERROR: %s at line %d\n", s, KJS::Lexer::curr()->lineNo());
    return 1;
}

/* may we automatically insert a semicolon ? */
static bool allowAutomaticSemicolon()
{
    return yychar == '}' || yychar == 0 || lexer().prevTerminator();
}

// kate: indent-width 2; replace-tabs on; tab-width 4; space-indent on;
