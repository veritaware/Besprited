// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#define TEST_GUI
#include "tests.h"

#include "ui/translator.h"
#include "ui/widget.h"

using namespace ui;

namespace
{

class WidgetI18nTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    setTranslator([](std::string_view key, std::string_view fallback)
                  { return key == "Open" ? std::string("Abrir") : std::string(fallback); });
  }
  void TearDown() override { setTranslator(nullptr); }
};

} // namespace

TEST_F(WidgetI18nTest, SetI18NTranslatesCurrentText)
{
  Widget w(kGenericWidget);
  w.setText("&Open");
  w.setI18N(); // key is "Open" (without '&')
  EXPECT_EQ("Abrir", w.text());
}

TEST_F(WidgetI18nTest, RuntimeSetTextKeepsProgramText)
{
  Widget w(kGenericWidget);
  w.setText("Open");
  w.setI18N();
  ASSERT_EQ("Abrir", w.text());

  w.setText("Open");
  EXPECT_EQ("Open", w.text());
}

TEST_F(WidgetI18nTest, SetTextQuietKeepsProgramText)
{
  Widget w(kGenericWidget);
  w.setText("Open");
  w.setI18N();
  w.setTextQuiet("Open");
  EXPECT_EQ("Open", w.text());
}
