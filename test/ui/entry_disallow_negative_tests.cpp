// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#define TEST_GUI
#include "tests.h"

#include "ui/entry.h"
#include "ui/message.h"

#include <string>

using namespace ui;

namespace
{

// Entry puts a half-typed expression (or, with disallowNegative, a
// disallowed one) right on kFocusLeaveMessage - send one directly instead
// of standing up a Manager to move focus for real.
void loseFocus(Entry& entry)
{
  Message msg(kFocusLeaveMessage);
  entry.sendMessage(&msg);
}

// Stands in for a window that reads an entry as a number to refresh a live
// preview.
class PreviewReader
{
public:
  explicit PreviewReader(Entry& entry)
    : m_entry(entry)
  {
    m_entry.Change.connect([this] { read(); });
  }

  void read()
  {
    value = m_entry.textInt();
    ++reads;
  }

  int value = 0;
  int reads = 0;

private:
  Entry& m_entry;
};

} // namespace

TEST(EntryDisallowNegative, OffByDefaultLetsNegativeValuesThrough)
{
  Entry entry(32, "0");
  PreviewReader preview(entry);

  entry.setText("-25");
  preview.read();
  EXPECT_EQ(-25, preview.value);

  loseFocus(entry);
  EXPECT_EQ("-25", entry.text());
}

TEST(EntryDisallowNegative, LivePreviewFallsBackInsteadOfGoingNegative)
{
  Entry entry(32, "300");
  entry.setDisallowNegative(true);
  PreviewReader preview(entry);
  preview.read();
  EXPECT_EQ(300, preview.value);

  // "-25" parses fine as an expression, but disallowNegative rejects the
  // result the same way an unparseable expression would: the preview
  // sits still on the last acceptable value instead of going negative.
  entry.setText("-25");
  preview.read();
  EXPECT_EQ(300, preview.value);
}

TEST(EntryDisallowNegative, FocusLeaveRevertsANegativeValue)
{
  Entry entry(32, "5");
  entry.setDisallowNegative(true);
  PreviewReader preview(entry);
  preview.read();

  entry.setText("-3");
  preview.read();

  loseFocus(entry);
  EXPECT_EQ("5", entry.text());
}

TEST(EntryDisallowNegative, ZeroIsStillAccepted)
{
  Entry entry(32, "5");
  entry.setDisallowNegative(true);
  PreviewReader preview(entry);
  preview.read();

  entry.setText("0");
  preview.read();
  EXPECT_EQ(0, preview.value);

  loseFocus(entry);
  EXPECT_EQ("0", entry.text());
}

// An expression that only evaluates negative partway through typing (e.g.
// "10-20") must not get stuck: only the final result is checked.
TEST(EntryDisallowNegative, ExpressionResultIsWhatGetsChecked)
{
  Entry entry(32, "0");
  entry.setDisallowNegative(true);
  PreviewReader preview(entry);
  preview.read();

  entry.setText("10-2");
  preview.read();
  EXPECT_EQ(8, preview.value);

  loseFocus(entry);
  EXPECT_EQ("10-2", entry.text());
}
