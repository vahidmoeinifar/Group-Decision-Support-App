# Storage, accounts and app rules. Uses Qt (Core, Sql, Network) but no UI,
# so it can be tested without a window.
add_library(gdbackend STATIC
    AppError.h
    Domain.h
    Database.h          Database.cpp
    PasswordHasher.h    PasswordHasher.cpp
    DecisionStore.h     DecisionStore.cpp
    AuthService.h       AuthService.cpp
    DecisionService.h   DecisionService.cpp
    SeedData.h          SeedData.cpp
)

target_include_directories(gdbackend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Qt::Network is only used for QPasswordDigestor (PBKDF2)
target_link_libraries(gdbackend
    PUBLIC  gdcore Qt6::Core Qt6::Sql
    PRIVATE Qt6::Network
)
