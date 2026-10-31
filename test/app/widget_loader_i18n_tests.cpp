// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#define TEST_GUI
#include "tests.h"

#include "app/widget_loader.h"
#include "base/fs.h"
#include "base/path.h"
#include "she/font.h"
#include "ui/entry.h"
#include "ui/label.h"
#include "ui/paint_event.h"
#include "ui/theme.h"
#include "ui/translator.h"

#include <fstream>

using namespace app;

namespace
{

// Fixed-metrics font and a no-op theme: enough for widgets to be sized.
class StubFont : public she::Font
{
public:
  she::FontType type() override { return she::FontType::kUnknown; }
  int height() const override { return 8; }
  gfx::Rect charBounds(int) const override { return gfx::Rect(0, 0, 6, 8); }
  int charWidth(int) const override { return 6; }
  int textLength(const std::string& str) const override { return int(str.size()) * 6; }
  bool isScalable() const override { return false; }
  void setSize(int) override {}
  void setAntialias(bool) override {}
};

class StubTheme : public ui::Theme
{
public:
  std::shared_ptr<she::Font> getDefaultFont() const override { return m_font; }
  std::shared_ptr<she::Font> getWidgetFont(const ui::Widget*) const override { return m_font; }
  ui::Cursor* getCursor(ui::CursorType) override { return nullptr; }
  void initWidget(ui::Widget*) override {}
  void getWindowMask(ui::Widget*, gfx::Region&) override {}
  void setDecorativeWidgetBounds(ui::Widget*) override {}
  int getScrollbarSize() override { return 0; }
  void paintDesktop(ui::PaintEvent&) override {}
  void paintBox(ui::PaintEvent&) override {}
  void paintButton(ui::PaintEvent&) override {}
  void paintCheckBox(ui::PaintEvent&) override {}
  void paintEntry(ui::PaintEvent&) override {}
  void paintGrid(ui::PaintEvent&) override {}
  void paintLabel(ui::PaintEvent&) override {}
  void paintLinkLabel(ui::PaintEvent&) override {}
  void paintListBox(ui::PaintEvent&) override {}
  void paintListItem(ui::PaintEvent&) override {}
  void paintMenu(ui::PaintEvent&) override {}
  void paintMenuItem(ui::PaintEvent&) override {}
  void paintSplitter(ui::PaintEvent&) override {}
  void paintRadioButton(ui::PaintEvent&) override {}
  void paintSeparator(ui::PaintEvent&) override {}
  void paintSlider(ui::PaintEvent&) override {}
  void paintComboBoxEntry(ui::PaintEvent&) override {}
  void paintComboBoxButton(ui::PaintEvent&) override {}
  void paintTextBox(ui::PaintEvent&) override {}
  void paintView(ui::PaintEvent&) override {}
  void paintViewScrollbar(ui::PaintEvent&) override {}
  void paintViewViewport(ui::PaintEvent&) override {}
  void paintWindow(ui::PaintEvent&) override {}
  void paintPopupWindow(ui::PaintEvent&) override {}
  void paintTooltip(ui::PaintEvent&) override {}

protected:
  void onRegenerate() override {}

private:
  std::shared_ptr<she::Font> m_font = std::make_shared<StubFont>();
};

class WidgetLoaderI18nTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    ui::CurrentTheme::set(&m_theme);
    // Translates every key, so an untranslated widget stands out.
    ui::setTranslator([](std::string_view key, std::string_view) { return "T:" + std::string(key); });

    m_path = base::join_path(base::get_temp_path(), "besprited_widget_loader_i18n_tests.xml");
    std::ofstream(m_path) << R"(<gui>
      <entry id="translated_literal" text="32" maxsize="16" />
      <entry id="opted_out" text="Open" maxsize="16" i18n="false" />
      <label id="plain_label" text="Open" />
      <label id="opted_out_label" text="Open" i18n="false" />
    </gui>)";
  }

  void TearDown() override
  {
    ui::setTranslator(nullptr);
    ui::CurrentTheme::set(nullptr);
    base::delete_file(m_path);
  }

  ui::Widget* load(const char* id)
  {
    WidgetLoader loader;
    return loader.loadWidget(m_path.c_str(), id);
  }

  StubTheme m_theme;
  std::string m_path;
};

} // namespace

TEST_F(WidgetLoaderI18nTest, EntryKeepsItsLiteralAndStaysEditable)
{
  std::unique_ptr<ui::Widget> w(load("translated_literal"));
  ASSERT_TRUE(dynamic_cast<ui::Entry*>(w.get()));
  EXPECT_EQ("32", w->text());

  w->setText("48");
  EXPECT_EQ("48", w->text());
}

TEST_F(WidgetLoaderI18nTest, OptOutAttributeSkipsTranslation)
{
  std::unique_ptr<ui::Widget> w(load("opted_out_label"));
  EXPECT_EQ("Open", w->text());
}

TEST_F(WidgetLoaderI18nTest, LabelsAreStillTranslated)
{
  std::unique_ptr<ui::Widget> w(load("plain_label"));
  EXPECT_EQ("T:Open", w->text());

  w->setText("Runtime");
  EXPECT_EQ("Runtime", w->text());
}
