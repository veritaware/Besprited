// Aseprite    | Copyright (C) 2001-2015 David Capello
// LibreSprite | Copyright (C) 2021      LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "ui/separator.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/commands/command.h"
#include "app/modules/gui.h"
#include "base/bind.h"
#include "ui/ui.h"

namespace app
{

using namespace ui;

class AboutCommand : public Command
{
public:
  AboutCommand();

protected:
  void onExecute(Context* context) override;
};

AboutCommand::AboutCommand()
  : Command("About", "About", CmdUIOnlyFlag)
{
}

void AboutCommand::onExecute(Context* context)
{
  std::unique_ptr<Window> window(new Window(Window::WithTitleBar, "About " PACKAGE));
  Box* box1 = new Box(VERTICAL);
  Grid* grid = new Grid(2, false);
  Label* title = new Label(PACKAGE_AND_VERSION);
  title->setI18N();
  Label* subtitle = new Label("Animated sprite editor & pixel art tool");
  subtitle->setI18N();

  Separator* license_separator = new Separator("Licensing:", HORIZONTAL | TOP);
  license_separator->setI18N();

  Label* license_desc = new Label("This software is released under the GPL-2.0 license.");
  license_desc->setI18N();
  Label* license_link = new LinkLabel(
      "https://raw.githubusercontent.com/veritaware/Besprited/refs/heads/trunk/LICENSE.txt", "Full license text");
  license_link->setI18N();
  Label* contributors = new LinkLabel(WEBSITE_CONTRIBUTORS, "Contributors");
  contributors->setI18N();

  Box* bottom_box1 = new Box(HORIZONTAL);
  Box* bottom_box2 = new Box(HORIZONTAL);
  Box* bottom_box3 = new Box(HORIZONTAL);
  Label* copyright = new Label(COPYRIGHT);
  Label* website = new LinkLabel(WEBSITE);
  Button* close_button = new Button("&Close");
  close_button->setI18N();

  grid->addChildInCell(title, 2, 1, 0);
  grid->addChildInCell(subtitle, 2, 1, 0);
  grid->addChildInCell(license_separator, 2, 1, 0);
  grid->addChildInCell(license_desc, 2, 1, 0);
  grid->addChildInCell(license_link, 1, 1, 0);
  grid->addChildInCell(contributors, 1, 1, 0);
  grid->addChildInCell(new Separator("", HORIZONTAL | TOP), 2, 1, 0);
  grid->addChildInCell(copyright, 2, 1, 0);
  grid->addChildInCell(website, 2, 1, 0);
  grid->addChildInCell(bottom_box1, 2, 1, 0);

  close_button->setFocusMagnet(true);

  bottom_box2->setExpansive(true);
  bottom_box3->setExpansive(true);

  bottom_box1->addChild(bottom_box2);
  bottom_box1->addChild(close_button);
  bottom_box1->addChild(bottom_box3);

  box1->addChild(grid);
  window->addChild(box1);

  close_button->setBorder(gfx::Border(close_button->border().left() + 16 * guiscale(), close_button->border().top(),
                                      close_button->border().right() + 16 * guiscale(),
                                      close_button->border().bottom()));

  close_button->Click.connect(base::Bind<void>(&Window::closeWindow, window.get(), close_button));

  window->openWindowInForeground();
}

std::unique_ptr<Command> CommandFactory::createAboutCommand()
{
  return std::make_unique<AboutCommand>();
}

} // namespace app
