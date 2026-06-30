//This file is licensed under MIT-0
//Copyright 2026 Ribbon-otter
//It is the unit tests for evalmath, a simple math string interpeter
#include <gtest/gtest.h>

#include "evalmath.h"

using namespace evalmath;

TEST(evalmath, eval)
{
   EXPECT_EQ(1.0, *eval("1"));
   EXPECT_FALSE(eval("1 +"));
   EXPECT_EQ(2.0, *eval("1+1"));
   EXPECT_EQ(220.0, *eval(" 120 + 100 ")) ;
   EXPECT_EQ(4.0, *eval("1+1+ 2")) ;
   EXPECT_EQ(5.0, *eval("1+2 * 2")) ;
   EXPECT_EQ(9.0, *eval("2*3+3")) ;
   EXPECT_EQ(12.0, *eval("6+2*3")); 
   EXPECT_EQ(2.0, *eval("9-7"));  
   EXPECT_EQ(7.0, *eval("6-2+3"));  
   EXPECT_EQ(1.0, *eval("6-2-3")); 
   EXPECT_EQ(3.0, *eval("6/2")); 
   EXPECT_EQ(6.0, *eval("6 / 2+3")); 
   EXPECT_FALSE(eval("6+2*")); 
   EXPECT_FALSE(eval("6 2")); 
   EXPECT_FALSE(eval("6++2")); 
   EXPECT_FALSE(eval("+6")); 
   EXPECT_FALSE(eval("+")); 
   EXPECT_FALSE(eval("1(5)")) ; 
}

TEST(evalmath, eval_paren)
{
   EXPECT_EQ(12.0, *eval("2*(3+3)")) ;
   EXPECT_EQ(12.0, *eval("(3+3)*2")) ;
   EXPECT_EQ(3.0, *eval("(3+3) / 2")) ;
   EXPECT_EQ(3.0, *eval("((3+ (3 ) ) / 2)")) ;
   EXPECT_FALSE(eval("(3+3 / 2")) ;
   EXPECT_FALSE(eval("3(")) ;
   EXPECT_FALSE(eval("(3")) ;
   EXPECT_FALSE(eval(")3")) ;
   EXPECT_FALSE(eval("3)")) ;
   EXPECT_FALSE(eval("1+()2")) ;
   EXPECT_FALSE(eval("()")) ;
   EXPECT_FALSE(eval("3(+)3 / 2")) ;
   EXPECT_FALSE(eval("3+(3 /) 2")) ;
   EXPECT_FALSE(eval("3+3 (/ 2)")) ;
}

TEST(evalmath, eval_decimal)
{
   EXPECT_EQ(5.5, *eval("5.5")) ;
   EXPECT_EQ(0.5, *eval("0.5")) ;
   EXPECT_EQ(5, *eval("5.")) ;
   EXPECT_FALSE(eval("0.5.3")) ;
   EXPECT_FALSE(eval("0..3")) ;
   EXPECT_EQ(0.5, eval(".5")) ; 
}

TEST(evalmath, eval_negative)
{
   EXPECT_EQ(-5, *eval("-5")) ;
   EXPECT_EQ(-4, *eval("1 + -5")) ;
   EXPECT_FALSE(eval("1(-5)")) ; 
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
// vim: ts=4 sw=4 expandtab
