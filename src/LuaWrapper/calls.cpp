#include "LuaWrapper.h"
extern "C" {
  static int lua_wrapper_pinMode(lua_State *lua) {
    int a = luaL_checkinteger(lua, 1);
    int b = luaL_checkinteger(lua, 2);
    pinMode(a, b);
  }

  static int lua_wrapper_digitalWrite(lua_State *lua) {
    int a = luaL_checkinteger(lua, 1);
    int b = luaL_checkinteger(lua, 2);
    digitalWrite(a, b);
  }
  
  static int lua_wrapper_delay(lua_State *lua) {
    int a = luaL_checkinteger(lua, 1);
    vTaskDelay(a/portTICK_PERIOD_MS);
  }

  static int lua_wrapper_print(lua_State *lua) {
    String a = String(luaL_checkstring(lua, 1));
    Serial.println(a);
  }

  static int lua_wrapper_millis(lua_State *lua) {
    lua_pushnumber(lua, (lua_Number) millis());
    return 1;
  }
}