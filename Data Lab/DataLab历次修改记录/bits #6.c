/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

DOUBLE PRECISION FLOATING POINT CODING RULES

For the problems that require you to implent double precision floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds
(also long long and unsigned long long). 
You can use arbitrary integer and unsigned constants
(also long long and unsigned long long constants).

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int, unsigned, long long or unsigned long long.  
      This means that you cannot use arrays, structs, or unions.
  6. Use any (double precision) floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the BDD checker to formally verify your functions
  4. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 8.0.0.  Version 8.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2014, plus Amendment 1 (published
   2015-05-15).  */
/* We do not support C11 <threads.h>.  */

/*
 * halfAdd - single-bit add using bit-wise operations only.
 *   Both x and y belong to {0, 1}. 
 *   Example: halfAdd(1, 0) = 1,
 *            halfAdd(1, 1) = 2,
 *   Legal ops: ~ | ^ & << >>
 *   Max ops: 7
 *   Rating: 1
 */
int halfAdd(int x, int y) {
    return ((x&y)<<1)|(x^y);
}
/* 
 * sameSign - return 1 if x and y have same sign, and 0 otherwise
 *   Examples sameSign(0x12345678, 0) = 1, sameSign(0xFFFFFFFF,0x1) = 0
 *   Legal ops: ! ~ & ! ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int sameSign(int x, int y) {
  return !((x>>31)^(y>>31));
}
/* 
 * oneMoreThan - return 1 if y is one more than x, and 0 otherwise
 *   Examples oneMoreThan(0, 1) = 1, oneMoreThan(-1, 1) = 0
 *   Legal ops: ~ & ! ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int oneMoreThan(int x, int y) {
/*consider x+1 overflow*/
  int op1=1<<31;
  return (!(y^(x+1)))&(!!(x^(~op1)));
}
/*
 * fullAdd - 4-bits add using bit-wise operations only.
 *   (0 <= x, y < 16) 
 *   Example: fullAdd(12, 7) = 3,
 *            fullAdd(7, 8) = 15,
 *   Legal ops: ~ | ^ & << >>
 *   Max ops: 30
 *   Rating: 2
 */
int fullAdd(int x, int y) {
  int temp=x^y;
  y=(x&y)<<1;
  x=temp;
  temp=x^y;
  y=(x&y)<<1;
  x=temp;
  temp=x^y;
  y=(x&y)<<1;
  x=temp;
  temp=x^y;
  y=(x&y)<<1;
  x=temp;
  return x&0xf;
}
/*
 * testAndSet: if low 16-bit of x == y, then let high 16-bit of x = z and
 *   return x, otherwise return x directly.
 *   x is a 32-bit integer, both y and z are 16-bit integers. 
 *   Example: testAndSet(0xFFFFFFFF, 0xEEEE, 0xDDDD) = 0xFFFFFFFF,
 *            testAndSet(0xFFFFFFFF, 0xFFFF, 0xDDDD) = 0xDDDDFFFF,
 *   Legal ops: ~ | ^ & << >> + !
 *   Max ops: 20
 *   Rating: 2
 */
int testAndSet(int x, int y, int z) {
  int mask,E,flag;
  mask=0xff|(0xff<<8);
  E=(x&mask)|(z<<16);
  flag=((!((x^y)<<16))<<31)>>31;
  return (flag&E)|(~flag&x);
}
/* 
 * zeroByte - return 1 if one of four bytes of x is zero, and 0 otherwise
 *   Example: zeroByte(0xFF00EEDD) = 1, zeroByte(-0x12345678) = 0
 *   Legal ops: ~ & ! | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int zeroByte(int x) {
/*will it must consider under zero?must only one bytes is 0? &(!((ans>>1)&0x1))*/
  int x1,x2,x3,x4;
  x1=!(x&0xff);
  x2=!(x&(0xff<<8));
  x3=!(x&(0xff<<16));
  x4=!(x&(0xff<<24));
  return (x1|x2|x3|x4);
}
/*
 * isTmin - returns 1 if x is the minimum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmin(int x) {
  return (!(x+x))&(!!x);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x+1;
}
/* 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOK(int x, int y) {
/*x-y=x+(~y+1),consider if y==Tmin*/
  int sxy,fx,fy,fs;
  sxy=x+~y+1;
  fx=(x>>31)&1;
  fy=(y>>31)&1;
  fs=(sxy>>31)&1;
  return (!(fx ^ fy)) | (!(fx ^ fs));
}
/*
 * negPerByte: negate each byte of x, then return x.
 *   Example: negPerByte(0xF8384CA9) = 0x08C8B457,
 *   Legal ops: ~ | ^ & << >> + !
 *   Max ops: 30
 *   Rating: 3
 */
int negPerByte(int x) {
  int x1,x2,x3,x4;
  x1 = (~x+1) & 0xff;
  x = x >> 8;
  x2 = (~x+1) & 0xff;
  x = x >> 8;
  x3 = (~x+1) & 0xff;
  x = x >> 8;
  x4 = (~x+1) & 0xff;
  return x1 | (x2 << 8) | (x3 << 16) | (x4 << 24);
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
/*FIRST CODE,I MYSELF CAN'T UNDERSTAND
  int fany,a,b;
  fany=~y;
  a=!((x+fany+1)|(x^y));
  b=(~x)&(x^fany);
  return a|((b>>31)&1);
  HAVE TRIED ALL THE POSSIBILITY,THIS TIME TRY TO REMOVE THE IMPOSSIBILLITIES
*/
  int sx,sy,case1,case2;
  sx=x>>31;
  sy=y>>31;
  case1=(!(sx^sy))&((~y+x)>>31);
  case2=sx & !sy;
  return !(case1|case2);
}
/*
 * modThree - calculate x mod 3 without using %.
 *   Example: modThree(12) = 0,
 *            modThree(2147483647) = 1,
 *            modThree(-8) = -2,
 *   Legal ops: ~ ! | ^ & << >> +
 *   Max ops: 60
 *   Rating: 4
 */
int modThree(int x) {
/* old even */
  int sign = x >> 31;  
  int flag, ans, minusagain;
  int mi = (~0) ^ 2;
  x = (x >> 16) + (x & (0xff | (0xff << 8)));
  x = (x >> 8) + (x & 0xff);
  x = (x >> 4) + (x & 0xf);
  x = (x >> 2) + (x & 3);
  x = (x >> 2) + (x & 3);
  x = (x >> 2) + (x & 3);
  flag = (!(x ^ 3)) << 31 >> 31; 
  ans = x & ~flag;
  sign = ~(!(ans) << 31 >> 31) & sign; 
  minusagain = ans + mi;
  return (ans | sign) & (minusagain | ~sign);

}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
/*change into posix all find the location that the first 1 appear
 *when x=0/x=-1,flag=0
bug:zero should be !zero
*/
  int posix,x1,x2,x3,x4,x5,flag,sumx,zero;
  posix=x^(x>>31);
  zero=!posix;
  flag=(!!posix)<<31>>31;
  x1=(!!(posix>>16))<<4;
  posix=posix>>x1;
  x2=(!!(posix>>8))<<3;
  posix=posix>>x2;
  x3=(!!(posix>>4))<<2;
  posix=posix>>x3;
  x4=(!!(posix>>2))<<1;
  posix=posix>>x4;
  x5=!!(posix>>1);
  sumx=x1+x2+x3+x4+x5+2;
  return (sumx&flag)|(zero);
}
/* 
 * twos2ones - convert from two's complement to one's complement,
             - the conversion should work as follows:
             -      1. given as argument an integer A
             -      2. take A as an normal two's complement integer and remember its value
             -      3. produce as result an integer B such that when B's bits
             -         are interpreted as one's complement, the value of B equals
             -         the value of A
             -      4. return B
             -  NOTE: 
             -      1.Since morden computers use two's complment, B's actual value in the computer
             -        may differ from A's. Remember what we want is essentially the bit representation
             -        not the value.
             -      2.As you may know, the ranges of two's complement and one's complement are not exactly the same.
             -        You should assume all the testing arguments can be both represented with two's and one's.
             -      3.There are two representations of 0 in one's complment, please *only* return 0.
 *   Example: twos2ones(1) = 1,
              twos2ones(-1) = -2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 3
 */
int twos2ones(int x) {
  int flag,one;
  flag=x>>31;
  one=~1+1;
  return x+flag&one;
}
/* 
 * ones2twos - convert from one's complement to two's complement,
             - the conversion should work as follows:
             -      1. given as argument an integer A
             -      2. keep A's bit representation, calculate the value it represents under one's complement
             -      3. produce as result an integer B such that B's value in two's complement 
             -         equals the above value
             -      4. return B
             -  NOTE: 
             -      1.Since morden computers use two's complment, A's actual value in the computer
             -        may differ from B's. Remember what we want is essentially the bit representation
             -        of A not the value.
 *   Example: ones2twos(-1) = 0,
              ones2twos(0) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 3
 */
int ones2twos(int x) {
  int flag;
  flag=!!(x>>31);
  return x+flag;
}
/* 

 * float2double - Turn single-precision floating point number f to

 *   double-precision floating point argument d.

 *   Both the argument and result are passed as unsigned long long's, but

 *   they are to be interpreted as the bit-level representations of

 *   single-precision and double-precision floating point values.

 *   When argument is a denormalized number, handle it like a normalized number

 *   Legal ops: Any long long/unsigned long long operations incl. ||, &&. also if, while

 *   Hint: You can only use immediate values of int range. 

 *   If you want to use long long values, consider using << operator.

 *   Max ops: 15

 *   Rating: 2

 */
unsigned long long float2double(unsigned long long uf) {
  unsigned long long exp,s,exp_float,frac;
  exp_float=uf&0x7f800000;
  exp=((exp_float>>23)+1024-128)<<52;
  frac=(uf&0x007fffff)<<29;
  s=(uf&0x80000000)<<32;
  if(exp_float==0x7f800000){
    exp = 0x7ff00000;
    exp=exp<<32;
  }
  return (s)|(exp)|(frac);

}
/* 
 * double_abs - Return bit-level equivalent of absolute value of f for
 *   doubleing point argument f.
 *   Both the argument and result are passed as unsigned long long's, but
 *   they are to be interpreted as the bit-level representations of
 *   double-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any long long/unsigned long long operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned long long double_abs(unsigned long long uf) {
  unsigned long long exp,frac,exp_mask,frac_mask,mask1,mask2;
  mask1=0x7ff;
  mask2=0x3ffffff;
  exp_mask=mask1<<52;
  exp=uf&exp_mask;
  frac_mask=(mask2<<26)|mask2;
  frac=uf&frac_mask;

  if(exp==exp_mask&&frac)return uf;
  else return uf<<1>>1;
}
/* 
 * double_neg - Return bit-level equivalent of expression -f for
 *   double point argument f.
 *   Both the argument and result are passed as unsigned long long's, but
 *   they are to be interpreted as the bit-level representations of
 *   double-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned long long double_neg(unsigned long long uf) {
/*if(((uf>>52)>0x7ff)&&((uf<<12)!=0))return uf;
  else return uf^(1<<63);*/
  unsigned long long exp,frac,exp_mask,frac_mask,mask1,mask2,mask3;
  mask1=0x7ff;
  mask2=0x3ffffff;
  mask3=1;
  exp_mask=mask1<<52;
  exp=uf&exp_mask;
  frac_mask=(mask2<<26)|mask2;
  frac=uf&frac_mask;

  if(exp==exp_mask&&frac)return uf;
  else return uf^(mask3<<63);
}
/* 
 * double_half - Return bit-level equivalent of expression 0.5*f for
 *   double argument f.
 *   Both the argument and result are passed as unsigned long long's, but
 *   they are to be interpreted as the bit-level representation of
 *   double-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned long long double_half(unsigned long long uf) {
  unsigned long long exp,s,frac,exp_mask,frac_mask,s_mask;
  unsigned long long mask1,mask2,mask3,carry;
  mask1=0x7ff;
  mask2=0x3ffffff;
  mask3=1;

  exp_mask=mask1<<52;
  exp=uf&exp_mask;

  frac_mask=(mask2<<26)|mask2;
  frac=uf&frac_mask;

  s_mask=mask3<<63;
  s=uf&s_mask;

  if(exp==exp_mask)return uf;

  carry=!((uf&3)^3);
  if(exp==(mask3<<52))return s|(carry+(uf<<1>>2));
  if(exp==0)return s|(carry+(frac>>1));

  return s|((exp>>52)-1)<<52|frac;
}
/* 
 * double_twice - Return bit-level equivalent of expression 2*f for
 *   double argument f.
 *   Both the argument and result are passed as unsigned long long's, but
 *   they are to be interpreted as the bit-level representation of
 *   double-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned long long double_twice(unsigned long long uf) {
  unsigned long long s,exp,frac;
  s=uf>>63;
  exp=uf<<1>>53;
  frac=uf<<12>>12;
  if(exp==0) frac=frac << 1;
  else if((exp+1)==(1<<11))return uf;
  else if(exp==0x7fe){
         frac=0;
         exp=(1<<11)-1;
       }
  else exp=exp+1;
  return s<<63|exp<<52|frac;
}
/* 
 * double_pwr2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 64-bit integer x.
 *
 *   The unsigned long long value that is returned should have the identical bit
 *   representation as the double-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned long long double_pwr2(long long x) {
  unsigned long long mask;
  mask=0x7ff;
  if(x<-1022)return 0;
  else if(x>3072)return mask<<52;
  x=x+1023;
  x=x<<52;
  return x;
}
/* 
 * double_negpwr2 - Return bit-level equivalent of the expression 2.0^-x
 *   (2.0 raised to the power -x) for any 64-bit integer x.
 *
 *   The unsigned long long value that is returned should have the identical bit
 *   representation as the double-precision floating-point number 2.0^-x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned long long double_negpwr2(long long x) {
  unsigned long long mask;
  mask=0x7ff;
  x=-x;
  if(x<-1022)return 0;
  else if(x>3072)return mask<<52;
  x=x+1023;
  x=x<<52;
  return x;
}
/* 
 * double_add - Return bit-level equivalent of f1 + f2 for
 *   double-precision floating point arguments f1 and f2.
 *   Both the argument and result are passed as unsigned long long's, but
 *   they are to be interpreted as the bit-level representations of
 *   double-precision floating point values.
 *   If either of the arguments is NaN/+oo/-oo, return 0. 
 *   NOTE: For simplicity, ignore the sign bit and treat all arguments as positive.
 *   Hint: be careful of rounding issues, refer to 
 *   http://pages.cs.wisc.edu/~markhill/cs354/Fall2008/notes/flpt.apprec.html for more details
 *   Legal ops: Any long long/unsigned long long operations incl. ||, &&. also if, while
 *   Max ops: 100
 *   Rating: 5
 */
unsigned long long double_add(unsigned long long f1, unsigned long long f2) {
/*
  unsigned long long exp1,frac1,exp2,frac2;
  unsigned long long exp_mask,s_mask;
  unsigned long long mask1,mask3,mask4,t,mve;
  unsigned long long exp,frac;
  mask1=0x7ff;
  mask3=1;
  mask4=0x7fe;
  if(f1>f2){
    t=f1;
    f1=f2;
    f2=f1;
  }

  exp_mask=mask1<<52;
  s_mask=mask3<<63;

  exp1=f1&exp_mask;
  frac1=f1<<11>>11;

  exp2=f2&exp_mask;
  frac2=f2<<11>>11;

  if(exp1==exp_mask || exp2==exp_mask)return 0;
  
  mve=(exp2>>52)-(exp1>>52);

  if (frac1&(1 << (mve - 1))) {
    if (frac1&((1 << (mve - 1)) - 1))
        frac1 = (frac1 >> mve) + 1;
  }
  else frac1 = frac1 >> mve;

  frac=frac1+frac2;
  exp=exp2;

  if((exp>>52)==mask4 && jinwei)return exp_mask;

  if((frac&(1<<10))&&(frac&0x3ff))frac+=1;
  jinwei= !!(frac&s_mask);
 
  exp=((exp>>52)+jinwei)<<52;

  return exp|frac;*/

  unsigned long long exp1, exp2, t, mve, frac;

  f1=f1<<1>>1;
  f2=f2<<1>>1;
  if (f1 > f2) {
    t = f1;
    f1 = f2;
    f2 = t;
  }

  exp1 = f1 >> 52;
  exp2 = f2 >> 52;
  if (exp1 == 0x7ff || exp2 == 0x7ff)return 0;

  mve = exp2 - exp1;
  frac = f1 << 12 >> 12;
  if (mve) {

    if (frac&(1 << (mve - 1))) {
      if (frac&((1 << (mve - 1)) - 1))
	frac = (frac >> mve) + 1;
    }
    else frac = frac >> mve;
  }
  frac += f2 << 12 >> 12;

  if (frac&(1 << 52)) {
    if(exp2)
      frac = frac >> 1;
    exp2 += 1;
  }
  if (exp2 == 0x7ff)return exp2 << 52;
  return (exp2 << 52) | frac;

}
