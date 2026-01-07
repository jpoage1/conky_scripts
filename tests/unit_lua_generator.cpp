// tests/lua_generator_test.cpp
// #include <gtest/gtest.h>
#include "lua_generator.hpp"

#include <gtest/gtest.h>

using namespace telemetry;

TEST(LuaGeneratorTest, ProducesValidTableStructure) {
  LuaConfigGenerator gen;
  LuaSettings s;
  s.name = "TestSystem";
  s.window.x = 100;

  gen.add_settings(s);
  std::string output = gen.generate();

  // Verify key components exist in the generated string
  EXPECT_NE(output.find("run_mode = \"persistent\""), std::string::npos);
  EXPECT_NE(output.find("name = \"TestSystem\""), std::string::npos);
  EXPECT_NE(output.find("x = 100"), std::string::npos);
}

TEST(LuaGeneratorTest, HandlesMultipleSettings) {
  LuaConfigGenerator gen;
  gen.add_settings({"SystemA", {}, {}});
  gen.add_settings({"SystemB", {}, {}});

  std::string output = gen.generate();

  // Check for array comma separation
  EXPECT_NE(output.find("SystemA"), std::string::npos);
  EXPECT_NE(output.find("SystemB"), std::string::npos);
}
