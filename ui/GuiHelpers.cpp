#include "GuiHelpers.h"

#include "TournamentDataDefs.h"

GuiHelpers::GuiHelpers()
{

}

GuiHelpers::~GuiHelpers()
{

}

QString GuiHelpers::groupNumToString(int grpNum)
{
  if (grpNum > 0) return QString::number(grpNum);

  switch (grpNum)
  {
  case GROUP_NUM__ITERATION:
    return "--";
  case GROUP_NUM__L16:
    return tr("L16");
  case GROUP_NUM__QUARTERFINAL:
    return tr("QF");
  case GROUP_NUM__SEMIFINAL:
    return tr("SF");
  case GROUP_NUM__FINAL:
    return tr("FI");
  }

  return "??";

}
