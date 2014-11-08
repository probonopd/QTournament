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

add_executable(QTournament
    tournament.qrc
    Category.cpp 
    CatMngr.cpp 
    GenericDatabaseObject.cpp 
    GenericObjectManager.cpp
    GroupDef.cpp 
    KO_Config.cpp 
    main.cpp 
    MatchGroup.cpp 
    MatchMngr.cpp 
    Player.cpp 
    PlayerMngr.cpp 
    PlayerPair.cpp 
    RoundRobinCategory.cpp 
    Team.cpp 
    TeamMngr.cpp 
    Tournament.cpp 
    TournamentDB.cpp 
    ui/CatTableView.cpp 
    ui/CatTabWidget.cpp 
    ui/dlgEditPlayer.cpp 
    ui/dlgGroupAssignment.cpp 
    ui/GroupAssignmentListWidget.cpp 
    ui/GroupConfigWidget.cpp 
    ui/MainFrame.cpp 
    ui/PlayerTableView.cpp 
    ui/PlayerTabWidget.cpp 
    ui/TeamListView.cpp 
    ui/TeamTabWidget.cpp 
    ui/delegates/PairItemDelegate.cpp 
    ui/delegates/PlayerItemDelegate.cpp 
    models/CatTableModel.cpp 
    models/PlayerTableModel.cpp
    models/TeamListModel.cpp)


qt5_use_modules(QTournament Widgets Sql)

# Use the Widgets module from Qt 5.
#target_link_libraries(helloworld Qt5::Widgets)
target_link_libraries(QTournament DatabaseOverlayLib)

