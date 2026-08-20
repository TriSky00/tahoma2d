// Copyright Joseph McCormack
// Inkframe one-click effects: drop a preconfigured FX on the current column
// straight from the menu — no schematic knowledge needed.

// Tnz6 includes
#include "tapp.h"
#include "menubarcommandids.h"

// TnzQt includes
#include "toonzqt/menubarcommand.h"

// TnzLib includes
#include "toonz/fxcommand.h"
#include "toonz/tcolumnhandle.h"
#include "toonz/tframehandle.h"

// TnzBase includes
#include "tfx.h"
#include "tparamset.h"
#include "tparamcontainer.h"
#include "tdoubleparam.h"

#include <QList>

//=============================================================================

namespace {

// Set a double param if the fx has it; quick effects should never assert on
// upstream param renames.
void trySetDouble(TFx *fx, const std::string &name, double value) {
  if (!fx) return;
  TDoubleParam *dp =
      dynamic_cast<TDoubleParam *>(fx->getParams()->getParam(name));
  if (dp) {
    dp->setDefaultValue(value);
    dp->setValue(0, value);
  }
}

void insertQuickFx(const std::string &fxId, const char *fxName,
                   double blurValue = -1) {
  TApp *app = TApp::instance();
  TFx *fx   = TFx::create(fxId);
  if (!fx) return;
  if (fxName && *fxName) fx->setName(QString(fxName).toStdWString());
  if (blurValue >= 0) trySetDouble(fx, "value", blurValue);
  TFxCommand::addFx(fx, QList<TFxP>(), app,
                    app->getCurrentColumn()->getColumnIndex(),
                    app->getCurrentFrame()->getFrame());
}

//=============================================================================

class QuickFxGlow final : public MenuItemHandler {
public:
  QuickFxGlow() : MenuItemHandler(MI_QuickFxGlow) {}
  void execute() override { insertQuickFx("STD_glowFx", "Glow"); }
} quickFxGlow;

class QuickFxSoftBlur final : public MenuItemHandler {
public:
  QuickFxSoftBlur() : MenuItemHandler(MI_QuickFxSoftBlur) {}
  void execute() override { insertQuickFx("STD_blurFx", "Soft Blur", 8); }
} quickFxSoftBlur;

class QuickFxSpeedLines final : public MenuItemHandler {
public:
  QuickFxSpeedLines() : MenuItemHandler(MI_QuickFxSpeedLines) {}
  void execute() override {
    insertQuickFx("STD_directionalBlurFx", "Speed Lines");
  }
} quickFxSpeedLines;

class QuickFxMotionBlur final : public MenuItemHandler {
public:
  QuickFxMotionBlur() : MenuItemHandler(MI_QuickFxMotionBlur) {}
  void execute() override { insertQuickFx("STD_motionBlurFx", "Motion Blur"); }
} quickFxMotionBlur;

class QuickFxParticles final : public MenuItemHandler {
public:
  QuickFxParticles() : MenuItemHandler(MI_QuickFxParticles) {}
  void execute() override {
    insertQuickFx("STD_particlesFx", "Particles");
  }
} quickFxParticles;

}  // namespace
