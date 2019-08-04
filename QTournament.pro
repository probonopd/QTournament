#-------------------------------------------------
#
# Project created by QtCreator 2014-09-12T13:04:40
#
#-------------------------------------------------

QT       += widgets network

TARGET = QTournament
TEMPLATE = app
QMAKE_CXXFLAGS += -Wno-unused-parameter
CONFIG += c++17

VERSION = 0.6.0

# Detect the platform we are running on
win32 {
  DEFINES += "__IS_WINDOWS_BUILD"
}

CONFIG(release, debug|release) {
  DEFINES += "RELEASE_BUILD"
}

# linking against BOOST fails if this is not set
DEFINES += "BOOST_LOG_DYN_LINK=1"

HEADERS += \
    Category.h \
    CatMngr.h \
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
    ui/TeamTabWidget.h \
    ui/delegates/PairItemDelegate.h \
    ui/delegates/PlayerItemDelegate.h \
    models/CatTableModel.h \
    models/PlayerTableModel.h \
    Match.h \
    RoundRobinGenerator.h \
    ui/delegates/CatItemDelegate.h \
    ui/delegates/DelegateItemLED.h \
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
    ExternalPlayerDB.h \
    ui/commonCommands/cmdImportSinglePlayerFromExternalDatabase.h \
    ui/DlgImportPlayer.h \
    ui/commonCommands/cmdExportPlayerToExternalDatabase.h \
    ui/commonCommands/cmdCreatePlayerFromDialog.h \
    ui/DlgBulkImportToExtDb.h \
    HelperFunc.h \
    TournamentDatabaseObjectManager.h \
    TournamentDatabaseObject.h \
    CentralSignalEmitter.h \
    ui/DlgSelectReferee.h \
    ui/commonCommands/cmdAssignRefereeToMatch.h \
    ui/commonCommands/cmdCallMatch.h \
    ui/delegates/TeamItemDelegate.h \
    models/TeamTableModel.h \
    ui/TeamTableView.h \
    ui/delegates/CatTabPlayerItemDelegate.h \
    MatchTimePredictor.h \
    ui/TournamentProgressBar.h \
    ui/MatchLogTabWidget.h \
    ui/CommonMatchTableWidget.h \
    ui/MatchLogTable.h \
    ui/delegates/MatchLogItemDelegate.h \
    PlayerProfile.h \
    ui/DlgPlayerProfile.h \
    ui/delegates/RefereeSelectionDelegate.h \
    ui/AutoSizingTable.h \
    ui/delegates/BaseItemDelegate.h \
    SwissLadderGenerator.h \
    CSVImporter.h \
    ui/DlgImportCSV_Step1.h \
    ui/DlgImportCSV_Step2.h \
    ui/DlgPickTeam.h \
    ui/DlgPickCategory.h \
    ui/DlgRoundFinished.h \
    OnlineMngr.h \
    ui/DlgPassword.h \
    HttpClient.h \
    ui/DlgRegisterTournament.h \
    ui/commonCommands/cmdOnlineRegistration.h \
    ui/commonCommands/cmdSetOrChangePassword.h \
    ui/commonCommands/cmdUnlockKeystore.h \
    ui/commonCommands/cmdStartOnlineSession.h \
    ui/commonCommands/cmdFullSync.h \
    ui/commonCommands/cmdDeleteFromServer.h \
    ui/DlgConnectionSettings.h \
    ui/commonCommands/cmdConnectionSettings.h

SOURCES += \
    Category.cpp \
    CatMngr.cpp \
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
    ui/TeamTabWidget.cpp \
    ui/delegates/PairItemDelegate.cpp \
    ui/delegates/PlayerItemDelegate.cpp \
    models/CatTableModel.cpp \
    models/PlayerTableModel.cpp \
    Match.cpp \
    RoundRobinGenerator.cpp \
    ui/delegates/CatItemDelegate.cpp \
    ui/delegates/DelegateItemLED.cpp \
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
    ExternalPlayerDB.cpp \
    ui/commonCommands/cmdImportSinglePlayerFromExternalDatabase.cpp \
    ui/DlgImportPlayer.cpp \
    ui/commonCommands/cmdExportPlayerToExternalDatabase.cpp \
    ui/commonCommands/cmdCreatePlayerFromDialog.cpp \
    ui/DlgBulkImportToExtDb.cpp \
    HelperFunc.cpp \
    TournamentDatabaseObjectManager.cpp \
    TournamentDatabaseObject.cpp \
    CentralSignalEmitter.cpp \
    ui/DlgSelectReferee.cpp \
    ui/commonCommands/cmdAssignRefereeToMatch.cpp \
    ui/commonCommands/cmdCallMatch.cpp \
    ui/delegates/TeamItemDelegate.cpp \
    models/TeamTableModel.cpp \
    ui/TeamTableView.cpp \
    ui/delegates/CatTabPlayerItemDelegate.cpp \
    MatchTimePredictor.cpp \
    ui/TournamentProgressBar.cpp \
    ui/MatchLogTabWidget.cpp \
    ui/CommonMatchTableWidget.cpp \
    ui/MatchLogTable.cpp \
    ui/delegates/MatchLogItemDelegate.cpp \
    PlayerProfile.cpp \
    ui/DlgPlayerProfile.cpp \
    ui/delegates/RefereeSelectionDelegate.cpp \
    ui/AutoSizingTable.cpp \
    ui/delegates/BaseItemDelegate.cpp \
    SwissLadderGenerator.cpp \
    CSVImporter.cpp \
    ui/DlgImportCSV_Step1.cpp \
    ui/DlgImportCSV_Step2.cpp \
    ui/DlgPickTeam.cpp \
    ui/DlgPickCategory.cpp \
    ui/DlgRoundFinished.cpp \
    OnlineMngr.cpp \
    ui/DlgPassword.cpp \
    HttpClient.cpp \
    ui/DlgRegisterTournament.cpp \
    ui/commonCommands/cmdOnlineRegistration.cpp \
    ui/commonCommands/cmdSetOrChangePassword.cpp \
    ui/commonCommands/cmdUnlockKeystore.cpp \
    ui/commonCommands/cmdStartOnlineSession.cpp \
    ui/commonCommands/cmdFullSync.cpp \
    ui/commonCommands/cmdDeleteFromServer.cpp \
    ui/DlgConnectionSettings.cpp \
    ui/commonCommands/cmdConnectionSettings.cpp

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
    ui/DlgPickPlayerSex.ui \
    ui/DlgImportPlayer.ui \
    ui/DlgBulkImportToExtDb.ui \
    ui/DlgSelectReferee.ui \
    ui/MatchLogTabWidget.ui \
    ui/DlgPlayerProfile.ui \
    ui/DlgImportCSV_Step1.ui \
    ui/DlgImportCSV_Step2.ui \
    ui/DlgPickTeam.ui \
    ui/DlgPickCategory.ui \
    ui/DlgRoundFinished.ui \
    ui/DlgPassword.ui \
    ui/DlgRegisterTournament.ui \
    ui/DlgConnectionSettings.ui

TRANSLATIONS = tournament_de.ts


!unix {
  INCLUDEPATH += D:/msys64/usr/local/include
  LIBS += -LD:/msys64/usr/local/lib
  CONFIG(debug, debug|release):   LIBS += -lSimpleReportGeneratord0
  CONFIG(release, debug|release): LIBS += -lSimpleReportGenerator0
}

unix {
  INCLUDEPATH += /usr/local/include
  LIBS += -L/usr/local/lib
  CONFIG(debug, debug|release):   LIBS += -lSimpleReportGeneratord
  CONFIG(release, debug|release): LIBS += -lSimpleReportGenerator
}

LIBS += -lSqliteOverlay -lSloppy  
