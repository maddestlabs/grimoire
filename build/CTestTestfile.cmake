# CMake generated Testfile for 
# Source directory: C:/git/grimoire
# Build directory: C:/git/grimoire/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test("GrimoireCoreTests" "C:/git/grimoire/build/Debug/test_grimoire.exe")
  set_tests_properties("GrimoireCoreTests" PROPERTIES  _BACKTRACE_TRIPLES "C:/git/grimoire/CMakeLists.txt;77;add_test;C:/git/grimoire/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test("GrimoireCoreTests" "C:/git/grimoire/build/Release/test_grimoire.exe")
  set_tests_properties("GrimoireCoreTests" PROPERTIES  _BACKTRACE_TRIPLES "C:/git/grimoire/CMakeLists.txt;77;add_test;C:/git/grimoire/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test("GrimoireCoreTests" "C:/git/grimoire/build/MinSizeRel/test_grimoire.exe")
  set_tests_properties("GrimoireCoreTests" PROPERTIES  _BACKTRACE_TRIPLES "C:/git/grimoire/CMakeLists.txt;77;add_test;C:/git/grimoire/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test("GrimoireCoreTests" "C:/git/grimoire/build/RelWithDebInfo/test_grimoire.exe")
  set_tests_properties("GrimoireCoreTests" PROPERTIES  _BACKTRACE_TRIPLES "C:/git/grimoire/CMakeLists.txt;77;add_test;C:/git/grimoire/CMakeLists.txt;0;")
else()
  add_test("GrimoireCoreTests" NOT_AVAILABLE)
endif()
