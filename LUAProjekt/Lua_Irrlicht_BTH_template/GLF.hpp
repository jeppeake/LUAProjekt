#pragma comment(lib, "Irrlicht.lib")
#ifdef _DEBUG
#pragma comment(lib, "LuaLibd.lib")
#else
#pragma comment(lib, "Lualib.lib")
#endif

#include <lua.hpp>
#include <string>
#include <sstream>
namespace GLF {//General LUA Functions
	void throwError(lua_State* L, std::string error) {
		std::stringstream ss;
		ss << "print('" << error << "')";
		luaL_loadstring(L, ss.str().c_str());
		lua_pcall(L, 0, 0, 0);
		lua_pop(L, 1);
		lua_settop(L, 0);//might not be needed
		//std::cout << lua_gettop(L) << "\n";
	}

	float* RTAV(lua_State* L, int index, int len) {//read table as vector
		luaL_checktype(L, index, LUA_TTABLE);
		lua_rawgeti(L, index, len + 1);
		int t = lua_isnil(L,-1);//checks table length, must be 3
		if (t < 1) {
			GLF::throwError(L, "ERROR: vector contains too many values.");
			return nullptr;
		}

		float* p = new float[len];
		for (int i = 1; i <= 3; i++) {//extract data
			lua_rawgeti(L, index, i);
			int t = lua_isnil(L, -1);
			if (t) {
				throwError(L, "ERROR: vector contains too few values.");
				return nullptr;
			}
			p[i - 1] = luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}
		return p;
	}
}