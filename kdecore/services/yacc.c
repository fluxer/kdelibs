/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         kiotraderparse
#define yylex           kiotraderlex
#define yyerror         kiotradererror
#define yydebug         kiotraderdebug
#define yynerrs         kiotradernerrs

/* First part of user prologue.  */
#line 1 "yacc.y"

#include <stdlib.h>
#include <stdio.h>
#include "ktraderparse_p.h"
#include "yacc.h"

#define YYLTYPE_IS_TRIVIAL 0
#define YYENABLE_NLS 0
typedef void* yyscan_t;
void yyerror(void *_scanner, const char *s);
int kiotraderlex(YYSTYPE * yylval, yyscan_t scanner);
int kiotraderlex_init (yyscan_t* scanner);
int kiotraderlex_destroy(yyscan_t scanner);

void KTraderParse_initFlex( const char *s, yyscan_t _scanner );


#line 94 "yacc.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yacc.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NOT = 3,                        /* NOT  */
  YYSYMBOL_EQ = 4,                         /* EQ  */
  YYSYMBOL_EQI = 5,                        /* EQI  */
  YYSYMBOL_NEQ = 6,                        /* NEQ  */
  YYSYMBOL_NEQI = 7,                       /* NEQI  */
  YYSYMBOL_LEQ = 8,                        /* LEQ  */
  YYSYMBOL_GEQ = 9,                        /* GEQ  */
  YYSYMBOL_LE = 10,                        /* LE  */
  YYSYMBOL_GR = 11,                        /* GR  */
  YYSYMBOL_OR = 12,                        /* OR  */
  YYSYMBOL_AND = 13,                       /* AND  */
  YYSYMBOL_TOKEN_IN = 14,                  /* TOKEN_IN  */
  YYSYMBOL_TOKEN_IN_SUBSTRING = 15,        /* TOKEN_IN_SUBSTRING  */
  YYSYMBOL_MATCH_INSENSITIVE = 16,         /* MATCH_INSENSITIVE  */
  YYSYMBOL_TOKEN_IN_INSENSITIVE = 17,      /* TOKEN_IN_INSENSITIVE  */
  YYSYMBOL_TOKEN_IN_SUBSTRING_INSENSITIVE = 18, /* TOKEN_IN_SUBSTRING_INSENSITIVE  */
  YYSYMBOL_EXIST = 19,                     /* EXIST  */
  YYSYMBOL_MAX = 20,                       /* MAX  */
  YYSYMBOL_MIN = 21,                       /* MIN  */
  YYSYMBOL_VAL_BOOL = 22,                  /* VAL_BOOL  */
  YYSYMBOL_VAL_STRING = 23,                /* VAL_STRING  */
  YYSYMBOL_VAL_ID = 24,                    /* VAL_ID  */
  YYSYMBOL_VAL_NUM = 25,                   /* VAL_NUM  */
  YYSYMBOL_VAL_FLOAT = 26,                 /* VAL_FLOAT  */
  YYSYMBOL_27_ = 27,                       /* '~'  */
  YYSYMBOL_28_ = 28,                       /* '+'  */
  YYSYMBOL_29_ = 29,                       /* '-'  */
  YYSYMBOL_30_ = 30,                       /* '*'  */
  YYSYMBOL_31_ = 31,                       /* '/'  */
  YYSYMBOL_32_ = 32,                       /* '('  */
  YYSYMBOL_33_ = 33,                       /* ')'  */
  YYSYMBOL_YYACCEPT = 34,                  /* $accept  */
  YYSYMBOL_constraint = 35,                /* constraint  */
  YYSYMBOL_bool = 36,                      /* bool  */
  YYSYMBOL_bool_or = 37,                   /* bool_or  */
  YYSYMBOL_bool_and = 38,                  /* bool_and  */
  YYSYMBOL_bool_compare = 39,              /* bool_compare  */
  YYSYMBOL_expr_in = 40,                   /* expr_in  */
  YYSYMBOL_expr_twiddle = 41,              /* expr_twiddle  */
  YYSYMBOL_expr = 42,                      /* expr  */
  YYSYMBOL_term = 43,                      /* term  */
  YYSYMBOL_factor_non = 44,                /* factor_non  */
  YYSYMBOL_factor = 45                     /* factor  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  27
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   67

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  34
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  42
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  69

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   281


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      32,    33,    30,    28,     2,    29,     2,    31,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    27,     2,     2,     2,
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
      25,    26
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    84,    84,    85,    88,    91,    92,    95,    96,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   110,   111,
     112,   113,   114,   117,   118,   119,   122,   123,   124,   127,
     128,   129,   132,   133,   136,   137,   138,   139,   140,   141,
     142,   143,   144
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NOT", "EQ", "EQI",
  "NEQ", "NEQI", "LEQ", "GEQ", "LE", "GR", "OR", "AND", "TOKEN_IN",
  "TOKEN_IN_SUBSTRING", "MATCH_INSENSITIVE", "TOKEN_IN_INSENSITIVE",
  "TOKEN_IN_SUBSTRING_INSENSITIVE", "EXIST", "MAX", "MIN", "VAL_BOOL",
  "VAL_STRING", "VAL_ID", "VAL_NUM", "VAL_FLOAT", "'~'", "'+'", "'-'",
  "'*'", "'/'", "'('", "')'", "$accept", "constraint", "bool", "bool_or",
  "bool_and", "bool_compare", "expr_in", "expr_twiddle", "expr", "term",
  "factor_non", "factor", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-16)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -3,    11,    -9,    18,    32,   -16,   -16,   -16,   -16,   -16,
      -3,    38,   -16,   -16,    45,    48,     3,   -13,    12,    22,
     -16,   -16,   -16,   -16,   -16,   -16,    25,   -16,    -3,    -3,
      -3,    -3,    -3,    -3,    -3,    -3,    -3,    -3,    39,    40,
      -3,    41,    42,    -3,    -3,    -3,    -3,    -3,   -16,   -16,
     -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,
     -16,    -2,   -16,   -16,    -2,    22,    22,   -16,   -16
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     0,     0,     0,    40,    39,    36,    37,    38,
       0,     0,     3,     4,     6,     8,    17,    22,    25,    28,
      31,    33,    32,    35,    41,    42,     0,     1,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    34,     5,
       7,     9,    10,    11,    12,    14,    13,    15,    16,    18,
      20,    24,    19,    21,    23,    26,    27,    29,    30
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -16,   -16,   -16,    -4,    33,   -16,    14,   -16,   -15,    10,
      13,    66
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
       1,    38,    39,    40,    41,    42,    26,    30,    31,    32,
      33,    34,    35,    36,    37,    23,     2,     3,     4,     5,
       6,     7,     8,     9,    49,    61,    44,    45,    64,    10,
       2,     3,     4,     5,     6,     7,     8,     9,    27,    43,
      44,    45,    24,    10,    51,    52,    53,    54,    55,    56,
      57,    58,    46,    47,    65,    66,    25,    28,    48,    67,
      68,    29,    50,    59,    60,    62,    63,    22
};

static const yytype_int8 yycheck[] =
{
       3,    14,    15,    16,    17,    18,    10,     4,     5,     6,
       7,     8,     9,    10,    11,    24,    19,    20,    21,    22,
      23,    24,    25,    26,    28,    40,    28,    29,    43,    32,
      19,    20,    21,    22,    23,    24,    25,    26,     0,    27,
      28,    29,    24,    32,    30,    31,    32,    33,    34,    35,
      36,    37,    30,    31,    44,    45,    24,    12,    33,    46,
      47,    13,    29,    24,    24,    24,    24,     1
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,    19,    20,    21,    22,    23,    24,    25,    26,
      32,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    45,    24,    24,    24,    37,     0,    12,    13,
       4,     5,     6,     7,     8,     9,    10,    11,    14,    15,
      16,    17,    18,    27,    28,    29,    30,    31,    33,    37,
      38,    40,    40,    40,    40,    40,    40,    40,    40,    24,
      24,    42,    24,    24,    42,    43,    43,    44,    44
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    34,    35,    35,    36,    37,    37,    38,    38,    39,
      39,    39,    39,    39,    39,    39,    39,    39,    40,    40,
      40,    40,    40,    41,    41,    41,    42,    42,    42,    43,
      43,    43,    44,    44,    45,    45,    45,    45,    45,    45,
      45,    45,    45
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     1,     3,     1,     3,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     1,     3,
       3,     1,     2,     1,     3,     2,     1,     1,     1,     1,
       1,     2,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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
        yyerror (_scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, _scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *_scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (_scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *_scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, _scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *_scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], _scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, _scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *_scanner)
{
  YY_USE (yyvaluep);
  YY_USE (_scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_bool_or: /* bool_or  */
#line 68 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 907 "yacc.c"
        break;

    case YYSYMBOL_bool_and: /* bool_and  */
#line 69 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 913 "yacc.c"
        break;

    case YYSYMBOL_bool_compare: /* bool_compare  */
#line 70 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 919 "yacc.c"
        break;

    case YYSYMBOL_expr_in: /* expr_in  */
#line 71 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 925 "yacc.c"
        break;

    case YYSYMBOL_expr_twiddle: /* expr_twiddle  */
#line 72 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 931 "yacc.c"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 73 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 937 "yacc.c"
        break;

    case YYSYMBOL_term: /* term  */
#line 74 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 943 "yacc.c"
        break;

    case YYSYMBOL_factor_non: /* factor_non  */
#line 75 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 949 "yacc.c"
        break;

    case YYSYMBOL_factor: /* factor  */
#line 76 "yacc.y"
            { KTraderParse_destroy( ((*yyvaluep).ptr) ); }
#line 955 "yacc.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *_scanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, _scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* constraint: %empty  */
#line 84 "yacc.y"
                        { KTraderParse_setParseTree( 0L ); }
#line 1231 "yacc.c"
    break;

  case 3: /* constraint: bool  */
#line 85 "yacc.y"
                 { KTraderParse_setParseTree( (yyvsp[0].ptr) ); }
#line 1237 "yacc.c"
    break;

  case 4: /* bool: bool_or  */
#line 88 "yacc.y"
              { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1243 "yacc.c"
    break;

  case 5: /* bool_or: bool_and OR bool_or  */
#line 91 "yacc.y"
                             { (yyval.ptr) = KTraderParse_newOR( (yyvsp[-2].ptr), (yyvsp[0].ptr) ); }
#line 1249 "yacc.c"
    break;

  case 6: /* bool_or: bool_and  */
#line 92 "yacc.y"
                  { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1255 "yacc.c"
    break;

  case 7: /* bool_and: bool_compare AND bool_and  */
#line 95 "yacc.y"
                                    { (yyval.ptr) = KTraderParse_newAND( (yyvsp[-2].ptr), (yyvsp[0].ptr) ); }
#line 1261 "yacc.c"
    break;

  case 8: /* bool_and: bool_compare  */
#line 96 "yacc.y"
                       { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1267 "yacc.c"
    break;

  case 9: /* bool_compare: expr_in EQ expr_in  */
#line 99 "yacc.y"
                                 { (yyval.ptr) = KTraderParse_newCMP( (yyvsp[-2].ptr), (yyvsp[0].ptr), 1 ); }
#line 1273 "yacc.c"
    break;

  case 10: /* bool_compare: expr_in EQI expr_in  */
#line 100 "yacc.y"
                                  { (yyval.ptr) = KTraderParse_newCMP( (yyvsp[-2].ptr), (yyvsp[0].ptr), 7 ); }
#line 1279 "yacc.c"
    break;

  case 11: /* bool_compare: expr_in NEQ expr_in  */
#line 101 "yacc.y"
                                  { (yyval.ptr) = KTraderParse_newCMP( (yyvsp[-2].ptr), (yyvsp[0].ptr), 2 ); }
#line 1285 "yacc.c"
    break;

  case 12: /* bool_compare: expr_in NEQI expr_in  */
#line 102 "yacc.y"
                                   { (yyval.ptr) = KTraderParse_newCMP( (yyvsp[-2].ptr), (yyvsp[0].ptr), 8 ); }
#line 1291 "yacc.c"
    break;

  case 13: /* bool_compare: expr_in GEQ expr_in  */
#line 103 "yacc.y"
                                  { (yyval.ptr) = KTraderParse_newCMP( (yyvsp[-2].ptr), (yyvsp[0].ptr), 3 ); }
#line 1297 "yacc.c"
    break;

  case 14: /* bool_compare: expr_in LEQ expr_in  */
#line 104 "yacc.y"
                                  { (yyval.ptr) = KTraderParse_newCMP( (yyvsp[-2].ptr), (yyvsp[0].ptr), 4 ); }
#line 1303 "yacc.c"
    break;

  case 15: /* bool_compare: expr_in LE expr_in  */
#line 105 "yacc.y"
                                 { (yyval.ptr) = KTraderParse_newCMP( (yyvsp[-2].ptr), (yyvsp[0].ptr), 5 ); }
#line 1309 "yacc.c"
    break;

  case 16: /* bool_compare: expr_in GR expr_in  */
#line 106 "yacc.y"
                                 { (yyval.ptr) = KTraderParse_newCMP( (yyvsp[-2].ptr), (yyvsp[0].ptr), 6 ); }
#line 1315 "yacc.c"
    break;

  case 17: /* bool_compare: expr_in  */
#line 107 "yacc.y"
                      { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1321 "yacc.c"
    break;

  case 18: /* expr_in: expr_twiddle TOKEN_IN VAL_ID  */
#line 110 "yacc.y"
                                      { (yyval.ptr) = KTraderParse_newIN( (yyvsp[-2].ptr), KTraderParse_newID( (yyvsp[0].name) ), 1 ); }
#line 1327 "yacc.c"
    break;

  case 19: /* expr_in: expr_twiddle TOKEN_IN_INSENSITIVE VAL_ID  */
#line 111 "yacc.y"
                                                  { (yyval.ptr) = KTraderParse_newIN( (yyvsp[-2].ptr), KTraderParse_newID( (yyvsp[0].name) ), 0 ); }
#line 1333 "yacc.c"
    break;

  case 20: /* expr_in: expr_twiddle TOKEN_IN_SUBSTRING VAL_ID  */
#line 112 "yacc.y"
                                                { (yyval.ptr) = KTraderParse_newSubstringIN( (yyvsp[-2].ptr), KTraderParse_newID( (yyvsp[0].name) ), 1 ); }
#line 1339 "yacc.c"
    break;

  case 21: /* expr_in: expr_twiddle TOKEN_IN_SUBSTRING_INSENSITIVE VAL_ID  */
#line 113 "yacc.y"
                                                            { (yyval.ptr) = KTraderParse_newSubstringIN( (yyvsp[-2].ptr), KTraderParse_newID( (yyvsp[0].name) ), 0 ); }
#line 1345 "yacc.c"
    break;

  case 22: /* expr_in: expr_twiddle  */
#line 114 "yacc.y"
                      { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1351 "yacc.c"
    break;

  case 23: /* expr_twiddle: expr '~' expr  */
#line 117 "yacc.y"
                            { (yyval.ptr) = KTraderParse_newMATCH( (yyvsp[-2].ptr), (yyvsp[0].ptr), 1 ); }
#line 1357 "yacc.c"
    break;

  case 24: /* expr_twiddle: expr_twiddle MATCH_INSENSITIVE expr  */
#line 118 "yacc.y"
                                                  { (yyval.ptr) = KTraderParse_newMATCH( (yyvsp[-2].ptr), (yyvsp[0].ptr), 0 ); }
#line 1363 "yacc.c"
    break;

  case 25: /* expr_twiddle: expr  */
#line 119 "yacc.y"
                   { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1369 "yacc.c"
    break;

  case 26: /* expr: expr '+' term  */
#line 122 "yacc.y"
                    { (yyval.ptr) = KTraderParse_newCALC( (yyvsp[-2].ptr), (yyvsp[0].ptr), 1 ); }
#line 1375 "yacc.c"
    break;

  case 27: /* expr: expr '-' term  */
#line 123 "yacc.y"
                    { (yyval.ptr) = KTraderParse_newCALC( (yyvsp[-2].ptr), (yyvsp[0].ptr), 2 ); }
#line 1381 "yacc.c"
    break;

  case 28: /* expr: term  */
#line 124 "yacc.y"
           { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1387 "yacc.c"
    break;

  case 29: /* term: term '*' factor_non  */
#line 127 "yacc.y"
                          { (yyval.ptr) = KTraderParse_newCALC( (yyvsp[-2].ptr), (yyvsp[0].ptr), 3 ); }
#line 1393 "yacc.c"
    break;

  case 30: /* term: term '/' factor_non  */
#line 128 "yacc.y"
                          { (yyval.ptr) = KTraderParse_newCALC( (yyvsp[-2].ptr), (yyvsp[0].ptr), 4 ); }
#line 1399 "yacc.c"
    break;

  case 31: /* term: factor_non  */
#line 129 "yacc.y"
                 { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1405 "yacc.c"
    break;

  case 32: /* factor_non: NOT factor  */
#line 132 "yacc.y"
                       { (yyval.ptr) = KTraderParse_newNOT( (yyvsp[0].ptr) ); }
#line 1411 "yacc.c"
    break;

  case 33: /* factor_non: factor  */
#line 133 "yacc.y"
                   { (yyval.ptr) = (yyvsp[0].ptr); }
#line 1417 "yacc.c"
    break;

  case 34: /* factor: '(' bool_or ')'  */
#line 136 "yacc.y"
                        { (yyval.ptr) = KTraderParse_newBRACKETS( (yyvsp[-1].ptr) ); }
#line 1423 "yacc.c"
    break;

  case 35: /* factor: EXIST VAL_ID  */
#line 137 "yacc.y"
                     { (yyval.ptr) = KTraderParse_newEXIST( (yyvsp[0].name) ); }
#line 1429 "yacc.c"
    break;

  case 36: /* factor: VAL_ID  */
#line 138 "yacc.y"
               { (yyval.ptr) = KTraderParse_newID( (yyvsp[0].name) ); }
#line 1435 "yacc.c"
    break;

  case 37: /* factor: VAL_NUM  */
#line 139 "yacc.y"
                { (yyval.ptr) = KTraderParse_newNUM( (yyvsp[0].vali) ); }
#line 1441 "yacc.c"
    break;

  case 38: /* factor: VAL_FLOAT  */
#line 140 "yacc.y"
                  { (yyval.ptr) = KTraderParse_newFLOAT( (yyvsp[0].vald) ); }
#line 1447 "yacc.c"
    break;

  case 39: /* factor: VAL_STRING  */
#line 141 "yacc.y"
                   { (yyval.ptr) = KTraderParse_newSTRING( (yyvsp[0].name) ); }
#line 1453 "yacc.c"
    break;

  case 40: /* factor: VAL_BOOL  */
#line 142 "yacc.y"
                 { (yyval.ptr) = KTraderParse_newBOOL( (yyvsp[0].valb) ); }
#line 1459 "yacc.c"
    break;

  case 41: /* factor: MAX VAL_ID  */
#line 143 "yacc.y"
                   { (yyval.ptr) = KTraderParse_newMAX2( (yyvsp[0].name) ); }
#line 1465 "yacc.c"
    break;

  case 42: /* factor: MIN VAL_ID  */
#line 144 "yacc.y"
                   { (yyval.ptr) = KTraderParse_newMIN2( (yyvsp[0].name) ); }
#line 1471 "yacc.c"
    break;


#line 1475 "yacc.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (_scanner, YY_("syntax error"));
    }

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
                      yytoken, &yylval, _scanner);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, _scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (_scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, _scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, _scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 149 "yacc.y"


void yyerror ( void *_scanner, const char *s )  /* Called by yyparse on error */
{
    KTraderParse_error( s );
}

void KTraderParse_mainParse( const char *_code )
{
    yyscan_t scanner;
    kiotraderlex_init(&scanner);
    KTraderParse_initFlex(_code, scanner);
    kiotraderparse(scanner);
    kiotraderlex_destroy(scanner);
}
