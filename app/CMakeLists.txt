qt_add_executable(GroupDecisionApp
    main.cpp
)

# Theme.qml is a singleton: one shared instance, used as Theme.ink etc.
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# Registers the C++ types marked QML_ELEMENT and bundles the QML files into the executable.
qt_add_qml_module(GroupDecisionApp
    URI GroupDecision
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    SOURCES
        AppController.h       AppController.cpp
        Session.h             Session.cpp
        DecisionListModel.h   DecisionListModel.cpp
        DecisionController.h  DecisionController.cpp
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/LoginPage.qml
        qml/Workspace.qml
        qml/Sidebar.qml
        qml/DecisionPage.qml
        qml/RoundBar.qml
        qml/ResultsTab.qml
        qml/RankingList.qml
        qml/AgreementPanel.qml
        qml/MethodComparison.qml
        qml/AgreementHistory.qml
        qml/SensitivityPanel.qml
        qml/ScoresTab.qml
        qml/SetupTab.qml
        qml/NewDecisionPage.qml
        qml/components/AppButton.qml
        qml/components/AppField.qml
        qml/components/Avatar.qml
        qml/components/Card.qml
        qml/components/ConfirmDialog.qml
        qml/components/EmptyState.qml
        qml/components/Logo.qml
        qml/components/Segmented.qml
        qml/components/Tag.qml
        qml/components/Toast.qml
)

target_link_libraries(GroupDecisionApp PRIVATE
    gdbackend
    Qt6::Quick
    Qt6::QuickControls2
)

# Note: the executable must not be called "GroupDecision", because the build folder
# already has a "GroupDecision" directory for the QML module.
set_target_properties(GroupDecisionApp PROPERTIES
    WIN32_EXECUTABLE TRUE     # no console window on Windows
    MACOSX_BUNDLE TRUE
)
