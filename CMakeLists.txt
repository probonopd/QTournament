cmake_minimum_required(VERSION 2.8.11)

project(QTournament)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5Core)


# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
#set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# Include the header of the database overlay lib
include_directories("../DatabaseOverlayLib")
link_directories("../DatabaseOverlayLib")

# Enable support for C++ 11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(QTournament_Sources
    ui/GuiHelpers.cpp
    tournament.qrc
    Category.cpp 
    CatMngr.cpp
    CatRoundStatus.cpp
    Court.cpp
    CourtMngr.cpp 
    GenericDatabaseObject.cpp 
    GenericObjectManager.cpp
    GroupDef.cpp 
    KO_Config.cpp  
    MatchGroup.cpp 
    MatchMngr.cpp 
    Match.cpp 
    Player.cpp 
    PlayerMngr.cpp 
    PlayerPair.cpp 
    RankingEntry.cpp
    RankingMngr.cpp
    RoundRobinCategory.cpp 
    RoundRobinGenerator.cpp 
    Score.cpp
    Team.cpp 
    TeamMngr.cpp 
    ThreadSafeQueue.cpp
    Tournament.cpp 
    TournamentDB.cpp 
    models/CatTableModel.cpp 
    models/MatchGroupTabModel.cpp
    models/PlayerTableModel.cpp
    models/CourtTabModel.cpp
    models/MatchTabModel.cpp
    models/TeamListModel.cpp)

set(QTournament_Sources_UI
    ui/CatTableView.cpp 
    ui/CatTabWidget.cpp 
    ui/CourtTableView.cpp
    ui/dlgEditPlayer.cpp 
    ui/dlgGroupAssignment.cpp 
    ui/GroupAssignmentListWidget.cpp 
    ui/GroupConfigWidget.cpp 
    ui/MainFrame.cpp 
    ui/MatchGroupTableView.cpp
    ui/MatchTableView.cpp
    ui/PlayerTableView.cpp 
    ui/PlayerTabWidget.cpp 
    ui/ScheduleTabWidget.cpp
    ui/TeamListView.cpp 
    ui/TeamTabWidget.cpp 
    ui/delegates/CatItemDelegate.cpp 
    ui/delegates/CourtItemDelegate.cpp
    ui/delegates/DelegateItemLED.cpp 
    ui/delegates/MatchItemDelegate.cpp
    ui/delegates/PairItemDelegate.cpp 
    ui/delegates/PlayerItemDelegate.cpp)

set(QTournament_Tests
    tests/BasicTestClass.cpp
    tests/tstGenericDBObject.cpp
    tests/tstPlayer.cpp
    tests/tstTournament.cpp
    tests/DatabaseTestScenario.cpp
    tests/tstGroupDef.cpp
    tests/tstPlayerMngr.cpp
    tests/tstRoundRobinGenerator.cpp
    tests/tstCategory.cpp
    tests/tstKOConfig.cpp
    tests/tstScore.cpp
    tests/tstTeam.cpp
    tests/tstMatch.cpp
    tests/tstCatMngr.cpp
    tests/tstMatchMngr.cpp
    tests/tstRankingMngr.cpp
    tests/tstTeamMngr.cpp)
    
add_executable(QTournament main.cpp ${QTournament_Sources} ${QTournament_Sources_UI})

add_executable(QTournament_Tests EXCLUDE_FROM_ALL tests/unitTestMain.cpp ${QTournament_Sources} ${QTournament_Tests})

qt5_use_modules(QTournament Widgets Sql)
qt5_use_modules(QTournament_Tests Widgets Sql)

# Use the Widgets module from Qt 5.
#target_link_libraries(helloworld Qt5::Widgets)
target_link_libraries(QTournament DatabaseOverlayLib)
target_link_libraries(QTournament_Tests DatabaseOverlayLib cppunit)

