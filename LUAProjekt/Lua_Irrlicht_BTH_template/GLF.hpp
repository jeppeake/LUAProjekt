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
		luaL_loadstring(L, error.c_str());
		lua_error(L);
		//std::cout << lua_gettop(L) << "\n";
	}

	int EDC(lua_State* L, int index) {//Extract Data continuous (for nested tables with unknown length)
		int l = 1;
		if (!lua_rawgeti(L, index, l)) {
			return -1;
		}
		while (lua_istable(L,-1)) {
			l++;
			lua_rawgeti(L, index, l);
		}
		lua_pop(L,1);
		return l-1;
	}

	int ED(lua_State* L, int index, int len) {//Extract Data (for nested tables)
		luaL_checktype(L, index, LUA_TTABLE);
		lua_rawgeti(L, index, len + 1);
		int t = lua_isnil(L, -1);//checks table length, must be (len)
		if (t < 1) {
			GLF::throwError(L, "ERROR: vector contains too many values.");
			return 0;
		}
		lua_pop(L, 1);
		for (int i = 1; i <= len; i++) {//extract data
			lua_rawgeti(L, index, i);
			int t = lua_isnil(L, -1);
			if (t) {//too few values
				lua_pop(L, 1);
				throwError(L, "ERROR: vector contains too few values.");
				return 0;
			}
		}
	}

	float* RTAV(lua_State* L, int index, int len) {//read table as vector
		luaL_checktype(L, index, LUA_TTABLE);
		lua_rawgeti(L, index, len + 1);
		int t = lua_isnil(L, -1);//checks table length, must be (len)
		if (t < 1) {
			GLF::throwError(L, "ERROR: vector contains too many values.");
			return nullptr;
		}
		lua_pop(L, 1);
		float* p = new float[len];
		for (int i = 1; i <= len; i++) {//extract data
			lua_rawgeti(L, index, i);
			int t = lua_isnil(L, -1);
			if (t) {//too few values
				throwError(L, "ERROR: vector contains too few values.");
				return nullptr;
			}
			p[i - 1] = luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}
		return p;
	}

	unsigned int* RTAUV(lua_State* L, int index, int len) {//read table as vector
		luaL_checktype(L, index, LUA_TTABLE);
		lua_rawgeti(L, index, len + 1);
		int t = lua_isnil(L, -1);//checks table length, must be (len)
		if (t < 1) {
			GLF::throwError(L, "ERROR: vector contains too many values.");
			return nullptr;
		}
		lua_pop(L, 1);
		unsigned int* p = new unsigned int[len];
		for (int i = 1; i <= len; i++) {//extract data
			lua_rawgeti(L, index, i);
			int t = lua_isnil(L, -1);
			if (t) {//too few values
				throwError(L, "ERROR: vector contains too few values.");
				return nullptr;
			}
			p[i - 1] = luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}
		return p;
	}
}