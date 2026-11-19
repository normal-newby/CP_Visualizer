include("D:/Projects/sql_practice/build/Desktop_Qt_6_12_0_MinGW_64_bit_Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/sql_practice-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "D:/Projects/sql_practice/build/Desktop_Qt_6_12_0_MinGW_64_bit_Debug/sql_practice.exe"
    GENERATE_QT_CONF
)
