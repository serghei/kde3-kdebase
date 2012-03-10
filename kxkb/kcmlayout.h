#ifndef __KCM_LAYOUT_H__
#define __KCM_LAYOUT_H__


#include <kcmodule.h>

#include <qstring.h>
#include <qlistview.h>

#include "kxkbconfig.h"


class OptionListItem;
class LayoutConfigWidget;
class XkbRules;

class LayoutConfig : public KCModule
{
  Q_OBJECT

public:
  LayoutConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~LayoutConfig();

  void load();
  void save();
  void defaults();
  void initUI();

protected:
  QString createOptionString();
  void updateIndicator(QListViewItem* selLayout);

protected slots:
  void moveUp();
  void moveDown();
  void variantChanged();
  void displayNameChanged(const QString& name);
  void latinChanged();
  void layoutSelChanged(QListViewItem *);
  void loadRules();
  void updateLayoutCommand();
  void updateOptionsCommand();
  void add();
  void remove();

  void changed();

private:
  LayoutConfigWidget* widget;

  XkbRules *m_rules;
  KxkbConfig m_kxkbConfig;
  QDict<OptionListItem> m_optionGroups;

  QWidget* makeOptionsTab();
  void updateStickyLimit();
  static LayoutUnit getLayoutUnitKey(QListViewItem *sel);
};


#endif
