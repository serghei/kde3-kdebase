#ifndef __RULES_H__
#define __RULES_H__

#include <qstring.h>
#include <qdict.h>
#include <qmap.h>


class XkbRules
{
public:

  XkbRules(bool layoutsOnly=false);

  const QDict<char> &models() const { return m_models; };
  const QDict<char> &layouts() const { return m_layouts; };
  const QDict<char> &options() const { return m_options; };
  
  QStringList getAvailableVariants(const QString& layout);
  unsigned int getDefaultGroup(const QString& layout, const QString& includeGroup);

  bool isSingleGroup(const QString& layout);

protected:

  void loadRules(QString filename, bool layoutsOnly=false);
  void loadGroups(QString filename);
  void loadOldLayouts(QString filename);

private:

  QDict<char> m_models;
  QDict<char> m_layouts;
  QDict<char> m_options;
  QMap<QString, unsigned int> m_initialGroups;
  QDict<QStringList> m_varLists;
  QStringList m_oldLayouts;
  QStringList m_nonLatinLayouts;
  
  QString X11_DIR;	// pseudo-constant
  
//  void fixLayouts();
};


#endif
