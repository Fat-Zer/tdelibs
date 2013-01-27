/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/* This file is mostly data generated by flex. Unfortunately flex
   can't handle 16bit strings directly, so we just copy the part of
   the code we need and modify it to our needs.

   Most of the defines below are to make sure we can easily use the
   flex generated code, using as little editing as possible.

   The flex syntax to generate the lexer are more or less directly
   copied from the CSS2.1 specs, with some fixes for comments and
   the important symbol.

   To regenerate, run flex on tokenizer.flex. After this, copy the
   data tables and the YY_DECL method over to this file. Remove the
   init code from YY_DECL and change the YY_END_OF_BUFFER to only call
   yyterminate().

*/

// --------- begin generated code -------------------
#define YY_NUM_RULES 51
#define YY_END_OF_BUFFER 52
static yyconst short int yy_accept[331] =
    {   0,
        0,    0,   52,   50,    2,    2,   50,   50,   50,   50,
       50,   50,   50,   50,   50,   42,   50,   50,   50,   50,
       11,   11,   11,   50,   50,    2,    0,    0,    0,   10,
        0,   13,    0,    8,    0,    0,    9,    0,    0,    0,
       11,   11,   43,    0,   41,    0,    0,   42,    0,   40,
       40,   40,   40,   40,   40,   40,   40,   40,   40,   12,
       40,   40,   37,    0,    0,    0,    0,    0,    0,    0,
        0,   11,   11,    7,   47,   11,    0,    0,   11,   11,
        0,   11,    6,    5,    0,    0,    0,   10,    0,    0,
       13,   13,    0,    0,   10,    0,    0,    4,   12,    0,

        0,   40,   40,   40,    0,   40,   28,   40,   24,   26,
       40,   38,   30,   40,   29,   36,   40,   32,   31,   27,
       40,    0,    0,    0,    0,    0,    0,    0,    0,   11,
       11,   11,   12,   11,   11,   48,   48,   11,    0,    0,
        0,   13,    0,    0,    0,    1,   40,   40,   40,   40,
       33,   40,   39,   12,   34,    3,    0,    0,    0,    0,
        0,    0,    0,   11,   11,   44,    0,   48,   48,   48,
       47,    0,    0,   13,    0,    0,    0,   40,   40,   40,
       35,    0,    0,    0,    0,    0,    0,   15,   11,   11,
       49,   48,   48,   48,   48,    0,    0,    0,    0,   46,

        0,    0,    0,   13,    0,   40,   40,   25,    0,    0,
        0,    0,   16,    0,   11,   11,   49,   48,   48,   48,
       48,   48,    0,    0,    0,    0,    0,    0,    0,    0,
        0,   46,    0,    0,    0,    0,   13,    0,   40,   40,
        0,    0,    0,   14,    0,   11,   11,   49,   48,   48,
       48,   48,   48,   48,    0,   45,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,   13,
        0,   40,   40,    0,   18,    0,    0,   11,   49,   48,
       48,   48,   48,   48,   48,   48,    0,   45,    0,    0,
        0,   45,    0,    0,    0,    0,   40,    0,    0,    0,

        0,    0,   49,    0,    0,    0,   23,    0,    0,    0,
       17,   19,   49,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,   20,    0,    0,    0,   21,   22,    0
    } ;

static yyconst int yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    4,    5,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    6,    7,    8,    9,   10,   11,   12,   13,   14,
       15,   16,   17,   12,   18,   19,   20,   21,   21,   21,
       21,   21,   21,   21,   21,   21,   21,   12,   12,   22,
       23,   24,   25,   26,   30,   31,   32,   33,   34,   35,
       36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
       46,   47,   48,   49,   50,   51,   39,   52,   39,   53,
       12,   27,   12,   28,   29,   12,   30,   31,   32,   33,

       34,   35,   36,   37,   38,   39,   40,   41,   42,   43,
       44,   45,   46,   47,   48,   49,   50,   51,   39,   52,
       39,   53,   12,   54,   12,   55,    1,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,

       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56
    } ;

static yyconst int yy_meta[57] =
    {   0,
        1,    2,    3,    3,    3,    4,    4,    4,    4,    4,
        4,    4,    4,    5,    4,    4,    4,    6,    4,    4,
        6,    4,    4,    4,    7,    4,    8,    4,    8,    9,
        9,    9,    9,    9,    9,    8,    8,    8,    8,    8,
        8,    8,    8,    8,    8,    8,    8,    8,    8,    8,
        8,    8,    8,    4,    4,    8
    } ;

static yyconst short int yy_base[359] =
    {   0,
        0,    0,  807, 1638,   55,   60,   65,   64,  779,  781,
       60,  780,   56,  781,  785,   93,  793,   63,  126,  773,
       61,   75,  136,  772,  771,  162,  167,  751,   70, 1638,
      204,  764,  154, 1638,   63,  238, 1638,  760,   64,  160,
       82,  183,  133,  767, 1638,  755,  760,    0,  183,   53,
      753,   52,   83,  169,  135,  121,   57,  192,  205,  206,
      225,   86,  745,  752,  729,  731,  723,  716,  723,  726,
      725,  231,  276, 1638, 1638,  234,  254,  733,  235,  249,
      291,  277, 1638, 1638,  701,  195,  172,  219,  325,  359,
      717,  393,  217,  237,  286,  427,  461, 1638,  160,  727,

      135,  715,  495,  714,  344,  256,  700,  265,  699,  698,
       88,  697,  696,  175,  695,  694,  233,  693,  679,  678,
      267,  684,  664,  670,  656,  640,  651,  622,  627,  446,
      305,  529,  639,  320,  321,  318,  634,  323,  614,  287,
      521,  536,  326,  544,  641, 1638,  551,  621,  585,  311,
      619,  339,  618,  360,  617, 1638,  594,  582,  556,  559,
      564,  567,  566,  577,  592, 1638,  600,  324,  574,  572,
      634,  549,  620,  635,  641,  579,  324,  656,  662,  278,
      567,  551,  544,  562,  526,  528,  501, 1638,  677,  683,
      698,  382,  523,  522,  521,  732,  758,  288,  341, 1638,

      784,  496,  718,  744,  791,  799,  806,  517,  502,  489,
      486,  471, 1638,  474,  814,  821,  829,  383,  493,  485,
      484,  481,  400,  343,  407,  863,  356,  897,  931,  957,
      983, 1009, 1035, 1069,  475,  848,  882,  917,  943,  969,
      486,  453,  460, 1638,  445,  995, 1076, 1084,  416,  448,
      447,  444,  443,  426,  440, 1638,  444,  408,  509, 1118,
     1152,  601,  358, 1131, 1186, 1220, 1205,  407, 1227, 1235,
     1242,  481, 1250,  456, 1638,  417,  406,  535, 1257,  403,
     1638, 1638, 1638, 1638, 1638, 1638, 1049,  409,  410, 1265,
     1299,  426,  443, 1285, 1300,  370,  676,  354,  334,  352,

      339,  283, 1306, 1321, 1327, 1342, 1638,  261,  226,  225,
     1638, 1638, 1638, 1348, 1363, 1369,  214,  192,  129, 1384,
     1390, 1405,   73, 1638,   52, 1411, 1426, 1638, 1638, 1638,
     1460, 1464, 1472, 1476, 1482, 1487, 1495, 1501, 1509, 1518,
     1520, 1526, 1530, 1536, 1545, 1551, 1555, 1564, 1568, 1576,
     1580, 1588, 1596, 1604, 1608, 1616, 1624, 1628
    } ;

static yyconst short int yy_def[359] =
    {   0,
      330,    1,  330,  330,  330,  330,  330,  331,  332,  330,
      333,  330,  334,  330,  330,  330,  330,  330,  335,  330,
      336,  336,  336,  330,  330,  330,  330,  330,  331,  330,
      337,  332,  338,  330,  333,  339,  330,  330,  330,  335,
      336,  336,   16,  340,  330,  341,  330,   16,  342,  343,
      343,  343,  343,  343,  343,  343,  343,  343,  343,  343,
      343,  343,  343,  330,  330,  330,  330,  330,  330,  330,
      330,  336,  336,  330,  330,  336,  344,  330,  336,  336,
      330,  336,  330,  330,  330,  331,  331,  331,  331,  337,
      332,  332,  333,  333,  333,  333,  339,  330,  330,  340,

      345,  343,  343,  343,  346,  343,  343,  343,  343,  343,
      343,  343,  343,  343,  343,  343,  343,  343,  343,  343,
      343,  330,  330,  330,  330,  330,  330,  330,  330,   73,
      336,   73,  330,  336,  336,  347,  330,  336,  330,  331,
       89,   92,  333,   96,  348,  330,  103,  343,  103,  343,
      343,  343,  343,  343,  343,  330,  330,  330,  330,  330,
      330,  330,  330,   73,  132,  330,  330,  349,  330,  330,
      350,  330,   89,   92,   96,  348,  345,  103,  149,  343,
      343,  330,  330,  330,  330,  330,  330,  330,   73,  132,
      330,  351,  330,  330,  330,  350,  350,  352,  353,  330,

      354,  330,   89,   92,   96,  103,  149,  343,  330,  330,
      330,  330,  330,  330,   73,  132,  330,  355,  330,  330,
      330,  330,  330,  352,  330,  356,  353,  357,  350,  350,
      350,  350,  350,  354,  330,   89,   92,   96,  103,  149,
      330,  330,  330,  330,  330,   73,  132,  330,  358,  330,
      330,  330,  330,  330,  330,  330,  352,  352,  352,  352,
      356,  353,  353,  353,  353,  357,  233,  330,   89,   92,
       96,  343,  149,  330,  330,  330,  330,  246,  330,  330,
      330,  330,  330,  330,  330,  330,  352,  352,  352,  260,
      353,  353,  353,  265,  233,  330,  343,  330,  330,  330,

      330,  330,  330,  260,  265,  233,  330,  330,  330,  330,
      330,  330,  330,  260,  265,  233,  330,  330,  330,  260,
      265,  233,  330,  330,  330,  260,  265,  330,  330,    0,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330
    } ;

static yyconst short int yy_nxt[1695] =
    {   0,
        4,    5,    6,    5,    5,    5,    7,    8,    9,   10,
        4,    4,   11,    4,    4,   12,    4,   13,   14,   15,
       16,   17,    4,    4,    4,   18,   19,   20,   21,   21,
       21,   21,   21,   21,   21,   21,   21,   21,   21,   21,
       21,   21,   22,   21,   21,   21,   21,   21,   21,   23,
       21,   21,   21,   24,   25,   21,   26,   26,   26,   26,
       26,   26,   26,   26,   26,   26,   27,   27,   27,   27,
       27,   30,   30,   38,   75,   30,   39,   30,  105,  105,
       65,  106,   40,  105,   39,  329,   36,   77,   75,   36,
       31,   78,   79,  107,   66,   75,   31,   67,   42,  113,

       68,   77,   28,   45,   69,   70,   99,   71,   77,  105,
       46,   47,  105,   48,  105,  121,  108,  152,   80,   49,
      328,   50,   51,   51,   52,   53,   54,   51,   55,   56,
       57,   51,   58,   51,   59,   60,   51,   61,   51,   62,
       63,   51,   51,   51,   51,   51,   73,  105,   51,   75,
      101,  330,   81,   43,  146,   73,   73,   73,   73,   73,
       73,  105,   77,   26,   26,   26,   26,   26,   27,   27,
       27,   27,   27,  112,   92,   51,   78,   78,  325,   30,
       73,  111,   82,   92,   92,   92,   92,   92,   92,   73,
       73,   73,   73,   73,   73,  105,   75,   29,   31,   78,

       79,  105,   30,  103,   28,   29,   29,   29,   86,   77,
      109,   88,  103,  103,  103,  103,  103,  103,  105,   35,
      110,   31,   78,  117,   89,  324,   30,  153,  114,   30,
       90,  105,  105,   89,   89,   89,   89,   89,   89,   35,
       35,   35,   93,   36,   75,   31,  115,   75,   75,   30,
       95,  105,  116,  154,  323,  134,  118,   77,   96,  105,
       77,   77,   75,   36,   97,  319,  318,   96,   96,   96,
       96,   96,   96,  119,  132,   77,  120,   72,   72,   72,
       72,   72,  105,  132,  132,  132,  132,  132,  132,   75,
       75,  105,  317,  105,   30,  225,  130,  135,   30,  155,

      151,  150,   77,   77,  105,  130,  130,  130,  130,  130,
      130,  136,   36,   31,  226,  137,  312,  138,   75,  208,
      136,  136,  136,  136,  136,  136,   87,  140,  140,  140,
       87,   77,   30,   75,  166,  167,  171,  105,   30,  177,
      134,  167,  169,  146,  180,  141,   77,   77,  193,   77,
      225,   31,   36,  225,  141,  141,  141,  141,  141,  141,
       29,   29,   29,   86,  149,  105,   88,  228,  225,  226,
      225,  181,  311,  149,  149,  149,  149,  149,  149,   89,
      154,  310,  228,  309,  228,   90,  105,  308,   89,   89,
       89,   89,   89,   89,   91,   91,   91,   91,   91,  167,

      167,  223,  223,  223,  223,  223,  219,  250,  255,  255,
      255,  255,  255,  142,  200,  225,  225,  225,  307,   33,
      167,  256,  142,  142,  142,  142,  142,  142,   94,  143,
      143,  143,   94,  167,  226,  226,  226,  302,  225,   30,
      281,  255,  255,  255,  255,  255,  224,  144,  301,  296,
      286,  225,  228,   36,  256,  225,  144,  144,  144,  144,
      144,  144,   35,   35,   35,   93,  164,  285,  284,  228,
      226,  283,  282,   95,  277,  164,  164,  164,  164,  164,
      164,   96,  102,  102,  102,  102,  102,   97,  298,  276,
       96,   96,   96,   96,   96,   96,  102,  102,  102,  102,

      102,  275,  299,  274,  268,  254,  300,  105,  253,  252,
      287,  255,  255,  255,  287,  147,  225,  251,  245,  244,
      243,  105,  242,  288,  147,  147,  147,  147,  147,  147,
      131,  131,  131,  131,  131,  226,  131,  131,  131,  131,
      131,  173,  241,  105,  235,  222,  221,  220,  214,  165,
      173,  173,  173,  173,  173,  173,  174,  213,  165,  165,
      165,  165,  165,  165,  175,  174,  174,  174,  174,  174,
      174,  178,  212,  175,  175,  175,  175,  175,  175,  211,
      178,  178,  178,  178,  178,  178,  148,  148,  148,  148,
      148,  210,  209,  105,  177,  202,  195,  189,  194,  188,

      187,  186,  185,  227,  184,  179,  189,  189,  189,  189,
      189,  189,  190,  225,  179,  179,  179,  179,  179,  179,
      191,  190,  190,  190,  190,  190,  190,  228,  183,  191,
      191,  191,  191,  191,  191,  196,  196,  196,  196,  196,
      203,  198,  182,  105,  105,  105,  199,  105,  200,  203,
      203,  203,  203,  203,  203,  204,  177,  172,  170,  133,
      201,  205,  163,  162,  204,  204,  204,  204,  204,  204,
      205,  205,  205,  205,  205,  205,  206,  148,  148,  148,
      148,  148,  207,  161,  160,  206,  206,  206,  206,  206,
      206,  207,  207,  207,  207,  207,  207,  215,  159,  158,

      157,  156,  105,  216,  105,  105,  215,  215,  215,  215,
      215,  215,  216,  216,  216,  216,  216,  216,  217,  105,
      105,  105,  105,  105,  105,  105,  105,  217,  217,  217,
      217,  217,  217,  196,  196,  196,  196,  196,  236,  198,
      105,  105,  101,   33,  199,  139,  200,  236,  236,  236,
      236,  236,  236,  133,  129,  128,  127,  126,  201,  223,
      223,  223,  223,  223,  237,  330,  125,  124,  123,  122,
      330,  105,  200,  237,  237,  237,  237,  237,  237,  105,
       43,   49,  101,   98,  201,  223,  223,  223,  223,  229,
       33,  231,   85,   84,   83,   74,  231,  231,  232,   64,

       44,   43,   37,   34,  233,   33,  330,  330,  330,  330,
      234,  238,  330,  233,  233,  233,  233,  233,  233,  239,
      238,  238,  238,  238,  238,  238,  240,  330,  239,  239,
      239,  239,  239,  239,  246,  240,  240,  240,  240,  240,
      240,  247,  330,  246,  246,  246,  246,  246,  246,  248,
      247,  247,  247,  247,  247,  247,  330,  330,  248,  248,
      248,  248,  248,  248,  224,  224,  224,  257,  269,  330,
      259,  330,  330,  330,  330,  330,  330,  269,  269,  269,
      269,  269,  269,  260,  330,  330,  330,  330,  330,  261,
      330,  330,  260,  260,  260,  260,  260,  260,  227,  227,

      227,  262,  270,  330,  330,  330,  330,  330,  330,  264,
      330,  270,  270,  270,  270,  270,  270,  265,  330,  330,
      330,  330,  330,  266,  330,  330,  265,  265,  265,  265,
      265,  265,  223,  223,  223,  223,  223,  271,  330,  330,
      330,  330,  330,  330,  330,  200,  271,  271,  271,  271,
      271,  271,  330,  330,  330,  330,  330,  201,  223,  223,
      223,  223,  223,  272,  330,  330,  330,  330,  330,  330,
      330,  200,  272,  272,  272,  272,  272,  272,  330,  330,
      330,  330,  330,  201,  223,  223,  223,  223,  223,  273,
      330,  330,  330,  330,  330,  330,  330,  200,  273,  273,

      273,  273,  273,  273,  330,  330,  330,  330,  330,  201,
      223,  223,  223,  223,  223,   76,  330,  330,  330,  330,
      330,  330,  330,  200,   76,   76,   76,   76,   76,   76,
      330,  330,  330,  330,  330,  201,  229,  229,  229,  229,
      229,  330,  330,  330,  330,  330,  330,  330,  330,  200,
      287,  255,  255,  255,  287,  267,  225,  330,  330,  330,
      330,  201,  330,  288,  267,  267,  267,  267,  267,  267,
      223,  223,  223,  223,  229,  226,  231,  330,  330,  330,
      330,  231,  231,  232,  330,  330,  330,  330,  330,  233,
      330,  330,  330,  330,  330,  234,  278,  330,  233,  233,

      233,  233,  233,  233,  279,  278,  278,  278,  278,  278,
      278,  330,  330,  279,  279,  279,  279,  279,  279,  258,
      289,  289,  289,  258,  330,  225,  330,  330,  330,  330,
      330,  330,  291,  255,  255,  255,  291,  330,  290,  330,
      330,  330,  330,  225,  226,  292,  330,  290,  290,  290,
      290,  290,  290,  224,  224,  224,  257,  228,  330,  259,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  260,  330,  330,  330,  330,  330,  261,  330,
      330,  260,  260,  260,  260,  260,  260,  263,  293,  293,
      293,  263,  330,  330,  330,  330,  330,  330,  225,  330,

      330,  330,  330,  330,  330,  330,  294,  330,  330,  330,
      330,  330,  228,  330,  330,  294,  294,  294,  294,  294,
      294,  227,  227,  227,  262,  295,  330,  330,  330,  330,
      330,  330,  264,  330,  295,  295,  295,  295,  295,  295,
      265,  330,  330,  330,  330,  330,  266,   29,  330,  265,
      265,  265,  265,  265,  265,   32,   29,   29,   29,   29,
       29,   29,   35,  330,   32,   32,   32,   32,   32,   32,
      297,   35,   35,   35,   35,   35,   35,  303,  330,  297,
      297,  297,  297,  297,  297,  304,  303,  303,  303,  303,
      303,  303,  330,  330,  304,  304,  304,  304,  304,  304,

      291,  255,  255,  255,  291,  305,  330,  330,  330,  330,
      330,  225,  330,  292,  305,  305,  305,  305,  305,  305,
      306,  330,  330,  330,  330,  228,  313,  330,  330,  306,
      306,  306,  306,  306,  306,  313,  313,  313,  313,  313,
      313,  314,  330,  330,  330,  330,  330,  315,  330,  330,
      314,  314,  314,  314,  314,  314,  315,  315,  315,  315,
      315,  315,  316,  330,  330,  330,  330,  330,  320,  330,
      330,  316,  316,  316,  316,  316,  316,  320,  320,  320,
      320,  320,  320,  321,  330,  330,  330,  330,  330,  322,
      330,  330,  321,  321,  321,  321,  321,  321,  322,  322,

      322,  322,  322,  322,  326,  330,  330,  330,  330,  330,
      327,  330,  330,  326,  326,  326,  326,  326,  326,  327,
      327,  327,  327,  327,  327,  197,  330,  330,  330,  330,
      330,  224,  330,  330,  197,  197,  197,  197,  197,  197,
      224,  224,  224,  224,  224,  224,  227,  330,  330,  330,
      330,  330,  330,  330,  330,  227,  227,  227,  227,  227,
      227,   29,  330,   29,   29,   29,   29,   29,   29,   32,
      330,   32,   32,   35,  330,   35,   35,   35,   35,   35,
       35,   41,  330,   41,   41,   72,   72,   72,   72,   72,
       72,   76,   76,  330,   76,   76,   87,   87,   87,   87,

       87,   87,   87,   87,   91,   91,   91,   91,   91,   91,
       94,   94,   94,   94,   94,   94,   94,   94,  100,  100,
      100,  100,  100,  100,  100,  100,  100,   51,   51,  102,
      102,  102,  102,  102,  102,  104,  330,  104,  104,  131,
      131,  131,  131,  131,  131,  145,  145,  145,  145,  145,
      145,  145,  145,  145,  148,  148,  148,  148,  148,  148,
      168,  168,  330,  168,  176,  176,  176,  176,  176,  176,
      176,  176,  176,  192,  192,  330,  192,  197,  197,  197,
      330,  197,  197,  197,  197,  218,  218,  330,  218,  224,
      330,  224,  224,  224,  224,  224,  224,  227,  330,  227,

      227,  227,  227,  227,  227,  230,  230,  230,  230,  230,
      230,  230,  230,  249,  249,  330,  249,  258,  258,  258,
      258,  258,  258,  258,  258,  263,  263,  263,  263,  263,
      263,  263,  263,  280,  280,  330,  280,    3,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330

    } ;

static yyconst short int yy_chk[1695] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    5,    5,    5,    5,
        5,    6,    6,    6,    6,    6,    7,    7,    7,    7,
        7,    8,   11,   13,   21,   35,   13,   29,   52,   50,
       18,   50,   13,   57,   39,  325,   11,   21,   22,   35,
        8,   22,   22,   52,   18,   41,   29,   18,   13,   57,

       18,   22,    7,   16,   18,   18,   39,   18,   41,   53,
       16,   16,   62,   16,  111,   62,   53,  111,   22,   16,
      323,   16,   16,   16,   16,   16,   16,   16,   16,   16,
       16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
       16,   16,   16,   16,   16,   16,   19,   56,   16,   23,
      101,   43,   23,   43,  101,   19,   19,   19,   19,   19,
       19,   55,   23,   26,   26,   26,   26,   26,   27,   27,
       27,   27,   27,   56,   33,   43,   99,   99,  319,   87,
       40,   55,   23,   33,   33,   33,   33,   33,   33,   40,
       40,   40,   40,   40,   40,   54,   42,   86,   87,   42,

       42,  114,   86,   49,   27,   31,   31,   31,   31,   42,
       54,   31,   49,   49,   49,   49,   49,   49,   58,   93,
       54,   86,   60,   60,   31,  318,   88,  114,   58,   93,
       31,   59,   60,   31,   31,   31,   31,   31,   31,   36,
       36,   36,   36,   93,   72,   88,   59,   76,   79,   94,
       36,   61,   59,  117,  317,   79,   61,   72,   36,  117,
       76,   79,   80,   94,   36,  310,  309,   36,   36,   36,
       36,   36,   36,   61,   77,   80,   61,   73,   73,   73,
       73,   73,  106,   77,   77,   77,   77,   77,   77,   73,
       82,  108,  308,  121,  140,  198,   73,   80,   95,  121,

      108,  106,   73,   82,  180,   73,   73,   73,   73,   73,
       73,   81,   95,  140,  198,   81,  302,   82,  131,  180,
       81,   81,   81,   81,   81,   81,   89,   89,   89,   89,
       89,  131,   89,  134,  135,  136,  138,  150,  143,  177,
      134,  168,  136,  177,  150,   89,  134,  135,  168,  138,
      224,   89,  143,  199,   89,   89,   89,   89,   89,   89,
       90,   90,   90,   90,  105,  152,   90,  199,  227,  224,
      263,  152,  301,  105,  105,  105,  105,  105,  105,   90,
      154,  300,  227,  299,  263,   90,  154,  298,   90,   90,
       90,   90,   90,   90,   92,   92,   92,   92,   92,  192,

      218,  223,  223,  223,  223,  223,  192,  218,  225,  225,
      225,  225,  225,   92,  223,  258,  288,  289,  296,   92,
      280,  225,   92,   92,   92,   92,   92,   92,   96,   96,
       96,   96,   96,  249,  258,  288,  289,  277,  292,   96,
      249,  255,  255,  255,  255,  255,  257,   96,  276,  268,
      254,  257,  292,   96,  255,  293,   96,   96,   96,   96,
       96,   96,   97,   97,   97,   97,  130,  253,  252,  293,
      257,  251,  250,   97,  245,  130,  130,  130,  130,  130,
      130,   97,  272,  272,  272,  272,  272,   97,  274,  243,
       97,   97,   97,   97,   97,   97,  103,  103,  103,  103,

      103,  242,  274,  241,  235,  222,  274,  272,  221,  220,
      259,  259,  259,  259,  259,  103,  259,  219,  214,  212,
      211,  103,  210,  259,  103,  103,  103,  103,  103,  103,
      132,  132,  132,  132,  132,  259,  278,  278,  278,  278,
      278,  141,  209,  208,  202,  195,  194,  193,  187,  132,
      141,  141,  141,  141,  141,  141,  142,  186,  132,  132,
      132,  132,  132,  132,  144,  142,  142,  142,  142,  142,
      142,  147,  185,  144,  144,  144,  144,  144,  144,  184,
      147,  147,  147,  147,  147,  147,  149,  149,  149,  149,
      149,  183,  182,  181,  176,  172,  170,  164,  169,  163,

      162,  161,  160,  262,  159,  149,  164,  164,  164,  164,
      164,  164,  165,  262,  149,  149,  149,  149,  149,  149,
      167,  165,  165,  165,  165,  165,  165,  262,  158,  167,
      167,  167,  167,  167,  167,  171,  171,  171,  171,  171,
      173,  171,  157,  155,  153,  151,  171,  148,  171,  173,
      173,  173,  173,  173,  173,  174,  145,  139,  137,  133,
      171,  175,  129,  128,  174,  174,  174,  174,  174,  174,
      175,  175,  175,  175,  175,  175,  178,  297,  297,  297,
      297,  297,  179,  127,  126,  178,  178,  178,  178,  178,
      178,  179,  179,  179,  179,  179,  179,  189,  125,  124,

      123,  122,  297,  190,  120,  119,  189,  189,  189,  189,
      189,  189,  190,  190,  190,  190,  190,  190,  191,  118,
      116,  115,  113,  112,  110,  109,  107,  191,  191,  191,
      191,  191,  191,  196,  196,  196,  196,  196,  203,  196,
      104,  102,  100,   91,  196,   85,  196,  203,  203,  203,
      203,  203,  203,   78,   71,   70,   69,   68,  196,  197,
      197,  197,  197,  197,  204,  197,   67,   66,   65,   64,
      197,   63,  197,  204,  204,  204,  204,  204,  204,   51,
       47,   46,   44,   38,  197,  201,  201,  201,  201,  201,
       32,  201,   28,   25,   24,   20,  201,  201,  201,   17,

       15,   14,   12,   10,  201,    9,    3,    0,    0,    0,
      201,  205,    0,  201,  201,  201,  201,  201,  201,  206,
      205,  205,  205,  205,  205,  205,  207,    0,  206,  206,
      206,  206,  206,  206,  215,  207,  207,  207,  207,  207,
      207,  216,    0,  215,  215,  215,  215,  215,  215,  217,
      216,  216,  216,  216,  216,  216,    0,    0,  217,  217,
      217,  217,  217,  217,  226,  226,  226,  226,  236,    0,
      226,    0,    0,    0,    0,    0,    0,  236,  236,  236,
      236,  236,  236,  226,    0,    0,    0,    0,    0,  226,
        0,    0,  226,  226,  226,  226,  226,  226,  228,  228,

      228,  228,  237,    0,    0,    0,    0,    0,    0,  228,
        0,  237,  237,  237,  237,  237,  237,  228,    0,    0,
        0,    0,    0,  228,    0,    0,  228,  228,  228,  228,
      228,  228,  229,  229,  229,  229,  229,  238,  229,    0,
        0,    0,    0,  229,    0,  229,  238,  238,  238,  238,
      238,  238,    0,    0,    0,    0,    0,  229,  230,  230,
      230,  230,  230,  239,  230,    0,    0,    0,    0,  230,
        0,  230,  239,  239,  239,  239,  239,  239,    0,    0,
        0,    0,    0,  230,  231,  231,  231,  231,  231,  240,
      231,    0,    0,    0,    0,  231,    0,  231,  240,  240,

      240,  240,  240,  240,    0,    0,    0,    0,    0,  231,
      232,  232,  232,  232,  232,  246,  232,    0,    0,    0,
        0,  232,    0,  232,  246,  246,  246,  246,  246,  246,
        0,    0,    0,    0,    0,  232,  233,  233,  233,  233,
      233,    0,  233,    0,    0,    0,    0,  233,    0,  233,
      287,  287,  287,  287,  287,  233,  287,    0,    0,    0,
        0,  233,    0,  287,  233,  233,  233,  233,  233,  233,
      234,  234,  234,  234,  234,  287,  234,    0,    0,    0,
        0,  234,  234,  234,    0,    0,    0,    0,    0,  234,
        0,    0,    0,    0,    0,  234,  247,    0,  234,  234,

      234,  234,  234,  234,  248,  247,  247,  247,  247,  247,
      247,    0,    0,  248,  248,  248,  248,  248,  248,  260,
      260,  260,  260,  260,    0,  260,    0,    0,    0,    0,
        0,    0,  264,  264,  264,  264,  264,    0,  260,    0,
        0,    0,    0,  264,  260,  264,    0,  260,  260,  260,
      260,  260,  260,  261,  261,  261,  261,  264,    0,  261,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,  261,    0,    0,    0,    0,    0,  261,    0,
        0,  261,  261,  261,  261,  261,  261,  265,  265,  265,
      265,  265,    0,    0,    0,    0,    0,    0,  265,    0,

        0,    0,    0,    0,    0,    0,  265,    0,    0,    0,
        0,    0,  265,    0,    0,  265,  265,  265,  265,  265,
      265,  266,  266,  266,  266,  267,    0,    0,    0,    0,
        0,    0,  266,    0,  267,  267,  267,  267,  267,  267,
      266,    0,    0,    0,    0,    0,  266,  269,    0,  266,
      266,  266,  266,  266,  266,  270,  269,  269,  269,  269,
      269,  269,  271,    0,  270,  270,  270,  270,  270,  270,
      273,  271,  271,  271,  271,  271,  271,  279,    0,  273,
      273,  273,  273,  273,  273,  290,  279,  279,  279,  279,
      279,  279,    0,    0,  290,  290,  290,  290,  290,  290,

      291,  291,  291,  291,  291,  294,    0,    0,    0,    0,
        0,  291,    0,  291,  294,  294,  294,  294,  294,  294,
      295,    0,    0,    0,    0,  291,  303,    0,    0,  295,
      295,  295,  295,  295,  295,  303,  303,  303,  303,  303,
      303,  304,    0,    0,    0,    0,    0,  305,    0,    0,
      304,  304,  304,  304,  304,  304,  305,  305,  305,  305,
      305,  305,  306,    0,    0,    0,    0,    0,  314,    0,
        0,  306,  306,  306,  306,  306,  306,  314,  314,  314,
      314,  314,  314,  315,    0,    0,    0,    0,    0,  316,
        0,    0,  315,  315,  315,  315,  315,  315,  316,  316,

      316,  316,  316,  316,  320,    0,    0,    0,    0,    0,
      321,    0,    0,  320,  320,  320,  320,  320,  320,  321,
      321,  321,  321,  321,  321,  322,    0,    0,    0,    0,
        0,  326,    0,    0,  322,  322,  322,  322,  322,  322,
      326,  326,  326,  326,  326,  326,  327,    0,    0,    0,
        0,    0,    0,    0,    0,  327,  327,  327,  327,  327,
      327,  331,    0,  331,  331,  331,  331,  331,  331,  332,
        0,  332,  332,  333,    0,  333,  333,  333,  333,  333,
      333,  334,    0,  334,  334,  335,  335,  335,  335,  335,
      335,  336,  336,    0,  336,  336,  337,  337,  337,  337,

      337,  337,  337,  337,  338,  338,  338,  338,  338,  338,
      339,  339,  339,  339,  339,  339,  339,  339,  340,  340,
      340,  340,  340,  340,  340,  340,  340,  341,  341,  342,
      342,  342,  342,  342,  342,  343,    0,  343,  343,  344,
      344,  344,  344,  344,  344,  345,  345,  345,  345,  345,
      345,  345,  345,  345,  346,  346,  346,  346,  346,  346,
      347,  347,    0,  347,  348,  348,  348,  348,  348,  348,
      348,  348,  348,  349,  349,    0,  349,  350,  350,  350,
        0,  350,  350,  350,  350,  351,  351,    0,  351,  352,
        0,  352,  352,  352,  352,  352,  352,  353,    0,  353,

      353,  353,  353,  353,  353,  354,  354,  354,  354,  354,
      354,  354,  354,  355,  355,    0,  355,  356,  356,  356,
      356,  356,  356,  356,  356,  357,  357,  357,  357,  357,
      357,  357,  357,  358,  358,    0,  358,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330,  330,  330,  330,  330,  330,  330,
      330,  330,  330,  330

    } ;

YY_DECL
	{
	register yy_state_type yy_current_state;
	register unsigned short *yy_cp, *yy_bp;
	register int yy_act;

#line 25 "tokenizer.flex"


#line 1009 "tok"

	while ( 1 )		/* loops until end-of-file is reached */
		{
		yy_cp = yy_c_buf_p;

		/* Support of yytext. */
		*yy_cp = yy_hold_char;

		/* yy_bp points to the position in yy_ch_buf of the start of
		 * the current run.
		 */
		yy_bp = yy_cp;

		yy_current_state = yy_start;
yy_match:
		do
			{
			register YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)];
			if ( yy_accept[yy_current_state] )
				{
				yy_last_accepting_state = yy_current_state;
				yy_last_accepting_cpos = yy_cp;
				}
			while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
				{
				yy_current_state = (int) yy_def[yy_current_state];
				if ( yy_current_state >= 331 )
					yy_c = yy_meta[(unsigned int) yy_c];
				}
			yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
			++yy_cp;
			}
		while ( yy_base[yy_current_state] != 1638 );

yy_find_action:
		yy_act = yy_accept[yy_current_state];
		if ( yy_act == 0 )
			{ /* have to back up */
			yy_cp = yy_last_accepting_cpos;
			yy_current_state = yy_last_accepting_state;
			yy_act = yy_accept[yy_current_state];
			}

		YY_DO_BEFORE_ACTION;


do_action:	/* This label is used only to access EOF actions. */


		switch ( yy_act )
	{ /* beginning of action switch */
			case 0: /* must back up */
			/* undo the effects of YY_DO_BEFORE_ACTION */
			*yy_cp = yy_hold_char;
			yy_cp = yy_last_accepting_cpos;
			yy_current_state = yy_last_accepting_state;
			goto yy_find_action;

case 1:
YY_RULE_SETUP
#line 27 "tokenizer.flex"
/* ignore comments */
	YY_BREAK
case 2:
YY_RULE_SETUP
#line 29 "tokenizer.flex"
{yyTok = S; return yyTok;}
	YY_BREAK
case 3:
YY_RULE_SETUP
#line 31 "tokenizer.flex"
{yyTok = SGML_CD; return yyTok;}
	YY_BREAK
case 4:
YY_RULE_SETUP
#line 32 "tokenizer.flex"
{yyTok = SGML_CD; return yyTok;}
	YY_BREAK
case 5:
YY_RULE_SETUP
#line 33 "tokenizer.flex"
{yyTok = INCLUDES; return yyTok;}
	YY_BREAK
case 6:
YY_RULE_SETUP
#line 34 "tokenizer.flex"
{yyTok = DASHMATCH; return yyTok;}
	YY_BREAK
case 7:
YY_RULE_SETUP
#line 35 "tokenizer.flex"
{yyTok = BEGINSWITH; return yyTok;}
	YY_BREAK
case 8:
YY_RULE_SETUP
#line 36 "tokenizer.flex"
{yyTok = ENDSWITH; return yyTok;}
	YY_BREAK
case 9:
YY_RULE_SETUP
#line 37 "tokenizer.flex"
{yyTok = CONTAINS; return yyTok;}
	YY_BREAK
case 10:
YY_RULE_SETUP
#line 39 "tokenizer.flex"
{yyTok = STRING; return yyTok;}
	YY_BREAK
case 11:
YY_RULE_SETUP
#line 41 "tokenizer.flex"
{yyTok = IDENT; return yyTok;}
	YY_BREAK
case 12:
YY_RULE_SETUP
#line 43 "tokenizer.flex"
{yyTok = NTH; return yyTok;}
	YY_BREAK
case 13:
YY_RULE_SETUP
#line 45 "tokenizer.flex"
{yyTok = HASH; return yyTok;}
	YY_BREAK
case 14:
YY_RULE_SETUP
#line 47 "tokenizer.flex"
{yyTok = IMPORT_SYM; return yyTok;}
	YY_BREAK
case 15:
YY_RULE_SETUP
#line 48 "tokenizer.flex"
{yyTok = PAGE_SYM; return yyTok;}
	YY_BREAK
case 16:
YY_RULE_SETUP
#line 49 "tokenizer.flex"
{yyTok = MEDIA_SYM; return yyTok;}
	YY_BREAK
case 17:
YY_RULE_SETUP
#line 50 "tokenizer.flex"
{yyTok = FONT_FACE_SYM; return yyTok;}
	YY_BREAK
case 18:
YY_RULE_SETUP
#line 51 "tokenizer.flex"
{yyTok = CHARSET_SYM; return yyTok;}
	YY_BREAK
case 19:
YY_RULE_SETUP
#line 52 "tokenizer.flex"
{yyTok = NAMESPACE_SYM; return yyTok; }
	YY_BREAK
case 20:
YY_RULE_SETUP
#line 53 "tokenizer.flex"
{yyTok = TDEHTML_RULE_SYM; return yyTok; }
	YY_BREAK
case 21:
YY_RULE_SETUP
#line 54 "tokenizer.flex"
{yyTok = TDEHTML_DECLS_SYM; return yyTok; }
	YY_BREAK
case 22:
YY_RULE_SETUP
#line 55 "tokenizer.flex"
{yyTok = TDEHTML_VALUE_SYM; return yyTok; }
	YY_BREAK
case 23:
YY_RULE_SETUP
#line 57 "tokenizer.flex"
{yyTok = IMPORTANT_SYM; return yyTok;}
	YY_BREAK
case 24:
YY_RULE_SETUP
#line 59 "tokenizer.flex"
{yyTok = EMS; return yyTok;}
	YY_BREAK
case 25:
YY_RULE_SETUP
#line 60 "tokenizer.flex"
{yyTok = QEMS; return yyTok;} /* quirky ems */
	YY_BREAK
case 26:
YY_RULE_SETUP
#line 61 "tokenizer.flex"
{yyTok = EXS; return yyTok;}
	YY_BREAK
case 27:
YY_RULE_SETUP
#line 62 "tokenizer.flex"
{yyTok = PXS; return yyTok;}
	YY_BREAK
case 28:
YY_RULE_SETUP
#line 63 "tokenizer.flex"
{yyTok = CMS; return yyTok;}
	YY_BREAK
case 29:
YY_RULE_SETUP
#line 64 "tokenizer.flex"
{yyTok = MMS; return yyTok;}
	YY_BREAK
case 30:
YY_RULE_SETUP
#line 65 "tokenizer.flex"
{yyTok = INS; return yyTok;}
	YY_BREAK
case 31:
YY_RULE_SETUP
#line 66 "tokenizer.flex"
{yyTok = PTS; return yyTok;}
	YY_BREAK
case 32:
YY_RULE_SETUP
#line 67 "tokenizer.flex"
{yyTok = PCS; return yyTok;}
	YY_BREAK
case 33:
YY_RULE_SETUP
#line 68 "tokenizer.flex"
{yyTok = DEGS; return yyTok;}
	YY_BREAK
case 34:
YY_RULE_SETUP
#line 69 "tokenizer.flex"
{yyTok = RADS; return yyTok;}
	YY_BREAK
case 35:
YY_RULE_SETUP
#line 70 "tokenizer.flex"
{yyTok = GRADS; return yyTok;}
	YY_BREAK
case 36:
YY_RULE_SETUP
#line 71 "tokenizer.flex"
{yyTok = MSECS; return yyTok;}
	YY_BREAK
case 37:
YY_RULE_SETUP
#line 72 "tokenizer.flex"
{yyTok = SECS; return yyTok;}
	YY_BREAK
case 38:
YY_RULE_SETUP
#line 73 "tokenizer.flex"
{yyTok = HERZ; return yyTok;}
	YY_BREAK
case 39:
YY_RULE_SETUP
#line 74 "tokenizer.flex"
{yyTok = KHERZ; return yyTok;}
	YY_BREAK
case 40:
YY_RULE_SETUP
#line 75 "tokenizer.flex"
{yyTok = DIMEN; return yyTok;}
	YY_BREAK
case 41:
YY_RULE_SETUP
#line 76 "tokenizer.flex"
{yyTok = PERCENTAGE; return yyTok;}
	YY_BREAK
case 42:
YY_RULE_SETUP
#line 77 "tokenizer.flex"
{yyTok = INTEGER; return yyTok;}
	YY_BREAK
case 43:
YY_RULE_SETUP
#line 78 "tokenizer.flex"
{yyTok = FLOAT; return yyTok;}
	YY_BREAK
case 44:
YY_RULE_SETUP
#line 81 "tokenizer.flex"
{yyTok = NOTFUNCTION; return yyTok;}
	YY_BREAK
case 45:
YY_RULE_SETUP
#line 82 "tokenizer.flex"
{yyTok = URI; return yyTok;}
	YY_BREAK
case 46:
YY_RULE_SETUP
#line 83 "tokenizer.flex"
{yyTok = URI; return yyTok;}
	YY_BREAK
case 47:
YY_RULE_SETUP
#line 84 "tokenizer.flex"
{yyTok = FUNCTION; return yyTok;}
	YY_BREAK
case 48:
YY_RULE_SETUP
#line 86 "tokenizer.flex"
{yyTok = UNICODERANGE; return yyTok;}
	YY_BREAK
case 49:
YY_RULE_SETUP
#line 87 "tokenizer.flex"
{yyTok = UNICODERANGE; return yyTok;}
	YY_BREAK
case 50:
YY_RULE_SETUP
#line 89 "tokenizer.flex"
{yyTok = *yytext; return yyTok;}
	YY_BREAK
case 51:
YY_RULE_SETUP
#line 91 "tokenizer.flex"
ECHO;
	YY_BREAK
#line 1347 "tok"
case YY_STATE_EOF(INITIAL):
	yyterminate();

	case YY_END_OF_BUFFER:
        yy_c_buf_p = yytext;
        yy_act = YY_STATE_EOF(YY_START);
        goto do_action;

	default:
		YY_FATAL_ERROR(
			"fatal flex scanner internal error--no action found" );
	} /* end of action switch */
		} /* end of scanning one token */
} /* end of yylex */
