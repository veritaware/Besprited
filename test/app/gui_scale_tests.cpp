// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"

#include "app/modules/gui.h"

using namespace app;

TEST(GuessUiScale, ThresholdsProduceTheExpectedScale)
{
  EXPECT_EQ(1, guessUiScale(800, 0)) << "1280x800 stays at 100%";
  EXPECT_EQ(1, guessUiScale(999, 0)) << "just under the first threshold";
  EXPECT_EQ(2, guessUiScale(1000, 0));
  EXPECT_EQ(2, guessUiScale(1799, 0));
  EXPECT_EQ(3, guessUiScale(1800, 0));
  EXPECT_EQ(3, guessUiScale(4320, 0)) << "well past the top threshold, still capped at 3";
}

TEST(GuessUiScale, AnUnknownScreenHeightFallsBackToTheGivenHeight)
{
  EXPECT_EQ(guessUiScale(1000, 0), guessUiScale(0, 1000));
  EXPECT_EQ(guessUiScale(1800, 0), guessUiScale(-1, 1800));
}

TEST(GuessUiScale, MinimumScaleIsOne)
{
  EXPECT_EQ(1, guessUiScale(0, 0));
  EXPECT_EQ(1, guessUiScale(1, 1));
}
