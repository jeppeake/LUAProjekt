#pragma comment(lib, "Irrlicht.lib")
#ifdef _DEBUG
#pragma comment(lib, "LuaLibd.lib")
#else
#pragma comment(lib, "Lualib.lib")
#endif

#include <stdio.h>

#include <lua.hpp>
#include <Windows.h>
#include <iostream>
#include <thread>
#include "lua.hpp"
#include <irrlicht.h>
#include <vector>
#include <string>
#include <sstream>

struct sceneNodeObject {
public:
	sceneNodeObject(irr::scene::ISceneNode * node, std::string name) {
		this->node = node;
		this->name = name;
	}
	irr::scene::ISceneNode * node;
	std::string name;
};

irr::scene::IAnimatedMeshSceneNode* node;
std::vector<sceneNodeObject> objects;
irr::scene::ISceneManager* smgr;
bool searchName(std::string name) {
	for (auto object : objects) {
		if (object.name == name) {
			return true;
		}
	}
	return false;
}
std::string makeName() {
	std::stringstream ss;
	ss << "object_" << objects.size();
	int count = 1;
	while (searchName(ss.str())) {
		ss.clear();
		ss << "object_" << objects.size() + count;
		count++;
	}
	return ss.str();
}
int _addCube(float x, float y, float z, float size, std::string name = "") {
	bool valid;
	std::string n_name;
	if (name == "") {
		valid = true;
		std::cout << "No name set, generating name...\n";
		n_name = makeName();
		std::cout << "New name: " << n_name << "\n";
	}
	else {
		valid = !searchName(name);
		n_name = name;
	}

	if (valid) {
		objects.push_back(sceneNodeObject(smgr->addCubeSceneNode(10, NULL, -1, irr::core::vector3df(x, y, z)), n_name));
		objects.at(objects.size() - 1).node->setMaterialFlag(irr::video::EMF_LIGHTING, false);
		std::cout << "New object created\n";
		return 1;
	}
	else {
		std::cout << "Name already in use!\n";
		return -1;
	}
}

static int addCube(lua_State* L) {
	return 0;
}

static int updatepos(lua_State* L) {
	if (lua_gettop(L) != 3) {
		luaL_loadstring(L, "print('Wrong number of parameters, use this format: updatepos(x,y,z)')");
		lua_pcall(L, 0, 0, 0);
		lua_pop(L, 1);
		return 0;
	}
	
	float x = std::atof(lua_tostring(L, -3));
	float y = std::atof(lua_tostring(L, -2));
	float z = std::atof(lua_tostring(L, -1));
	node->setPosition(irr::core::vector3df(x, y, z));
	return 0;
}

static int getpos(lua_State* L) {
	irr::core::vector3df pos = node->getPosition();
	lua_newtable(L);
	lua_pushnumber(L, pos.X);
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, pos.Y);
	lua_rawseti(L, -2, 2);
	lua_pushnumber(L, pos.Z);
	lua_rawseti(L, -2, 3);
	return 1;
}

void registerLuaFunctions(lua_State* L) {
	lua_pushcfunction(L, updatepos);
	lua_setglobal(L, "updatepos");
	lua_pushcfunction(L, getpos);
	lua_setglobal(L, "getpos");
}


void ConsoleThread(lua_State* L) {
	char command[1000];
	while(GetConsoleWindow()) {
		memset(command, 0, 1000);
		std::cin.getline(command, 1000);
		if (luaL_loadstring(L, command) || lua_pcall(L, 0, 0, 0)) {
			std::cout << lua_tostring(L, -1) << '\n';
			lua_pop(L, 1);
		}
	}
}

int main()
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

		std::thread conThread(ConsoleThread, L);

	irr::IrrlichtDevice* device = irr::createDevice(irr::video::EDT_SOFTWARE, irr::core::dimension2d<irr::u32>(640, 480), 16, false, false, true, 0);
	if(!device)
		return 1;

	device->setWindowCaption(L"Hello World! - Irrlicht Engine Demo");
	irr::video::IVideoDriver* driver	= device->getVideoDriver();
	smgr		= device->getSceneManager();
	irr::gui::IGUIEnvironment* guienv	= device->getGUIEnvironment();

	guienv->addStaticText(L"Hello World! This is the Irrlicht Software renderer!", irr::core::rect<irr::s32>(10, 10, 260, 22), true);

	irr::scene::IAnimatedMesh* mesh = smgr->getMesh("../Bin/Meshes/sydney.md2");
	if (!mesh)
	{
		device->drop();
		return 1;
	}
	node = smgr->addAnimatedMeshSceneNode(mesh);

	if (node)
	{
		node->setMaterialFlag(irr::video::EMF_LIGHTING, false);
		node->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, false);
		node->setMD2Animation(irr::scene::EMAT_STAND);
		node->setMaterialTexture(0, driver->getTexture("../Bin/Meshes/sydney.bmp"));
	}

	node->setScale(irr::core::vector3df(0.5f, 0.5f, 0.5f));

	//smgr->addCameraSceneNode(0, irr::core::vector3df(0, 30, -40), irr::core::vector3df(0, 5, 0));
	_addCube(20, 20, 20, 10);
	_addCube(20, 20, 20, 10);
	_addCube(20, 20, 20, 10);
	_addCube(20, 20, 20, 10, "object_0");
	registerLuaFunctions(L);
	auto camera = smgr->addCameraSceneNodeFPS();
	while(device->run()) {
		bool active = device->isWindowActive();
		camera->setInputReceiverEnabled(active);
		camera->setInputReceiverEnabled(active);

		driver->beginScene(true, true, irr::video::SColor(255, 90, 101, 140));

		smgr->drawAll();
		guienv->drawAll();

		driver->endScene();
	}

	device->drop();

	conThread.join();
	return 0;
}