#ifndef GUIHELPERS_H
#define GUIHELPERS_H

#include <QObject>
#include <QString>

class GuiHelpers : public QObject
{
  Q_OBJECT

public:
  GuiHelpers();
  ~GuiHelpers();
  static QString groupNumToString(int grpNum);
};

#endif // GUIHELPERS_H
