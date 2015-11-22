#-------------------------------------------------
#
# Project created by QtCreator 2014-09-12T13:04:40
#
#-------------------------------------------------

QT       += sql widgets

TARGET = QTournament
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++14 -Wno-unused-parameter

# Optimization level O3 for release
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

# Optimization level O1 for debug
#QMAKE_CXXFLAGS_DEBUG += -O0

HEADERS += \
    Category.h \
    CatMngr.h \
    GenericDatabaseObject.h \
    GenericObjectManager.h \
    GroupDef.h \
    KO_Config.h \
    MatchGroup.h \
    MatchMngr.h \
    Player.h \
    PlayerMngr.h \
    PlayerPair.h \
    RoundRobinCategory.h \
    Team.h \
    TeamMngr.h \
    Tournament.h \
    TournamentDataDefs.h \
    TournamentDB.h \
    TournamentErrorCodes.h \
    ui/CatTableView.h \
    ui/CatTabWidget.h \
    ui/dlgEditPlayer.h \
    ui/dlgGroupAssignment.h \
    ui/GroupAssignmentListWidget.h \
    ui/GroupConfigWidget.h \
    ui/MainFrame.h \
    ui/PlayerTableView.h \
    ui/PlayerTabWidget.h \
    ui/TeamListView.h \
    ui/TeamTabWidget.h \
    ui/delegates/PairItemDelegate.h \
    ui/delegates/PlayerItemDelegate.h \
    models/CatTableModel.h \
    models/PlayerTableModel.h \
    models/TeamListModel.h \
    Match.h \
    RoundRobinGenerator.h \
    ui/delegates/CatItemDelegate.h \
    ui/delegates/DelegateItemLED.h \
    ThreadSafeQueue.h \
    ui/ScheduleTabWidget.h \
    models/MatchGroupTabModel.h \
    ui/MatchGroupTableView.h \
    models/MatchTabModel.h \
    ui/GuiHelpers.h \
    ui/MatchTableView.h \
    ui/delegates/MatchItemDelegate.h \
    Court.h \
    CourtMngr.h \
    models/CourtTabModel.h \
    ui/CourtTableView.h \
    Score.h \
    ui/delegates/CourtItemDelegate.h \
    CatRoundStatus.h \
    RankingMngr.h \
    RankingEntry.h \
    BracketGenerator.h \
    reports/AbstractReport.h \
    reports/ParticipantsList.h \
    ui/ReportsTabWidget.h \
    reports/ReportFactory.h \
    reports/MatchResultList.h \
    reports/MatchResultList_byGroup.h \
    reports/Standings.h \
    ElimCategory.h \
    reports/InOutList.h \
    ui/SeedingListWidget.h \
    ui/DlgSeedingEditor.h \
    ui/DlgTournamentSettings.h \
    ui/GameResultWidget.h \
    ui/DlgMatchResult.h \
    reports/ResultSheets.h \
    SignalRelay.h \
    reports/ResultsAndNextMatches.h \
    PureRoundRobinCategory.h \
    SwissLadderCategory.h \
    reports/BracketSheet.h \
    reports/BracketVisData.h \
    ui/commonCommands/AbstractCommand.h \
    ui/commonCommands/cmdRegisterPlayer.h \
    ui/commonCommands/cmdUnregisterPlayer.h \
    ui/commonCommands/cmdRemovePlayerFromCategory.h \
    ui/DlgSelectPlayer.h \
    ui/commonCommands/cmdBulkAddPlayerToCat.h \
    ui/commonCommands/cmdBulkRemovePlayersFromCat.h \
    ui/commonCommands/cmdMoveOrCopyPlayerToCategory.h \
    ui/MenuGenerator.h \
    ui/commonCommands/cmdMoveOrCopyPairToCategory.h \
    ui/DlgPickPlayerSex.h \
    ui/commonCommands/cmdCreateNewPlayerInCat.h \
    reports/commonReportElements/AbstractReportElement.h \
    reports/commonReportElements/plotStandings.h \
    reports/MatrixAndStandings.h \
    reports/commonReportElements/MatchMatrix.h \
    ExternalPlayerDB.h

SOURCES += \
    Category.cpp \
    CatMngr.cpp \
    GenericDatabaseObject.cpp \
    GenericObjectManager.cpp \
    GroupDef.cpp \
    KO_Config.cpp \
    main.cpp \
    MatchGroup.cpp \
    MatchMngr.cpp \
    Player.cpp \
    PlayerMngr.cpp \
    PlayerPair.cpp \
    RoundRobinCategory.cpp \
    Team.cpp \
    TeamMngr.cpp \
    Tournament.cpp \
    TournamentDB.cpp \
    ui/CatTableView.cpp \
    ui/CatTabWidget.cpp \
    ui/dlgEditPlayer.cpp \
    ui/dlgGroupAssignment.cpp \
    ui/GroupAssignmentListWidget.cpp \
    ui/GroupConfigWidget.cpp \
    ui/MainFrame.cpp \
    ui/PlayerTableView.cpp \
    ui/PlayerTabWidget.cpp \
    ui/TeamListView.cpp \
    ui/TeamTabWidget.cpp \
    ui/delegates/PairItemDelegate.cpp \
    ui/delegates/PlayerItemDelegate.cpp \
    models/CatTableModel.cpp \
    models/PlayerTableModel.cpp \
    models/TeamListModel.cpp \
    Match.cpp \
    RoundRobinGenerator.cpp \
    ui/delegates/CatItemDelegate.cpp \
    ui/delegates/DelegateItemLED.cpp \
    ThreadSafeQueue.cpp \
    ui/ScheduleTabWidget.cpp \
    models/MatchGroupTabModel.cpp \
    ui/MatchGroupTableView.cpp \
    models/MatchTabModel.cpp \
    ui/GuiHelpers.cpp \
    ui/MatchTableView.cpp \
    ui/delegates/MatchItemDelegate.cpp \
    Court.cpp \
    CourtMngr.cpp \
    models/CourtTabModel.cpp \
    ui/CourtTableView.cpp \
    Score.cpp \
    ui/delegates/CourtItemDelegate.cpp \
    CatRoundStatus.cpp \
    RankingMngr.cpp \
    RankingEntry.cpp \
    BracketGenerator.cpp \
    reports/AbstractReport.cpp \
    reports/ParticipantsList.cpp \
    ui/ReportsTabWidget.cpp \
    reports/ReportFactory.cpp \
    reports/MatchResultList.cpp \
    reports/MatchResultList_byGroup.cpp \
    reports/Standings.cpp \
    ElimCategory.cpp \
    reports/InOutList.cpp \
    ui/SeedingListWidget.cpp \
    ui/DlgSeedingEditor.cpp \
    ui/DlgTournamentSettings.cpp \
    ui/GameResultWidget.cpp \
    ui/DlgMatchResult.cpp \
    reports/ResultSheets.cpp \
    SignalRelay.cpp \
    reports/ResultsAndNextMatches.cpp \
    PureRoundRobinCategory.cpp \
    SwissLadderCategory.cpp \
    reports/BracketSheet.cpp \
    reports/BracketVisData.cpp \
    ui/commonCommands/AbstractCommand.cpp \
    ui/commonCommands/cmdRegisterPlayer.cpp \
    ui/commonCommands/cmdUnregisterPlayer.cpp \
    ui/commonCommands/cmdRemovePlayerFromCategory.cpp \
    ui/DlgSelectPlayer.cpp \
    ui/commonCommands/cmdBulkAddPlayerToCat.cpp \
    ui/commonCommands/cmdBulkRemovePlayersFromCat.cpp \
    ui/commonCommands/cmdMoveOrCopyPlayerToCategory.cpp \
    ui/MenuGenerator.cpp \
    ui/commonCommands/cmdMoveOrCopyPairToCategory.cpp \
    ui/DlgPickPlayerSex.cpp \
    ui/commonCommands/cmdCreateNewPlayerInCat.cpp \
    reports/commonReportElements/AbstractReportElement.cpp \
    reports/commonReportElements/plotStandings.cpp \
    reports/MatrixAndStandings.cpp \
    reports/commonReportElements/MatchMatrix.cpp \
    ExternalPlayerDB.cpp

RESOURCES += \
    tournament.qrc

OTHER_FILES +=

FORMS += \
    ui/CatTabWidget.ui \
    ui/dlgEditPlayer.ui \
    ui/dlgGroupAssignment.ui \
    ui/GroupAssignmentListWidget.ui \
    ui/GroupConfigWidget.ui \
    ui/MainFrame.ui \
    ui/PlayerTabWidget.ui \
    ui/TeamTabWidget.ui \
    ui/ScheduleTabWidget.ui \
    ui/ReportsTabWidget.ui \
    ui/DlgSeedingEditor.ui \
    ui/DlgTournamentSettings.ui \
    ui/GameResultWidget.ui \
    ui/DlgMatchResult.ui \
    ui/DlgSelectPlayer.ui \
    ui/DlgPickPlayerSex.ui

TRANSLATIONS = tournament_de.ts

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-DatabaseOverlayLib-Desktop_Qt_MinGW_w64_64bit_MSYS2-Release/release/ -lDatabaseOverlayLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-DatabaseOverlayLib-Desktop_Qt_MinGW_w64_64bit_MSYS2-Debug/debug/ -lDatabaseOverlayLib
else:unix:!macx: LIBS += -L$$PWD/../build-DatabaseOverlayLib-Desktop_GCC-Debug/ -lDatabaseOverlayLib

INCLUDEPATH += $$PWD/../DatabaseOverlayLib
DEPENDPATH += $$PWD/../DatabaseOverlayLib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-SimpleReportGenerator-Desktop_Qt_MinGW_w64_64bit_MSYS2-Release/release -lSimpleReportGenerator
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-SimpleReportGenerator-Desktop_Qt_MinGW_w64_64bit_MSYS2-Debug/debug -lSimpleReportGenerator
else:unix:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-SimpleReportGenerator-Desktop_Clang-Release/ -lSimpleReportGenerator
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-SimpleReportGenerator-Desktop_Clang-Debug/ -lSimpleReportGenerator

INCLUDEPATH += $$PWD/../SimpleReportGeneratorLib
DEPENDPATH += $$PWD/../SimpleReportGeneratorLib
