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

irr::scene::IAnimatedMeshSceneNode* node;
irr::IrrlichtDevice* device;

using namespace irr::scene;

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

static int addMesh(lua_State* L) {
	luaL_argcheck(L, lua_istable(L, -1), 1, "<table> expected!");
	if (!lua_rawgeti(L, 1, 1) || !lua_rawgeti(L, 1, 2) || !lua_rawgeti(L, 1, 3)) {
		luaL_loadstring(L, "print('Error: not a valid number of vertices.')");
		lua_pcall(L, 0, 0, 0);
		lua_pop(L, 1);
		return 0;
	}
	//make sure all the 3 elements in the table are tables
	luaL_argcheck(L, lua_istable(L, -1), 1, "<table> expected in table!");
	luaL_argcheck(L, lua_istable(L, -2), 1, "<table> expected in table!");
	luaL_argcheck(L, lua_istable(L, -3), 1, "<table> expected in table!");
	float vertices[3][3];
	int index = 0; //sane index
	for (int i = 2; i < 5; i++) {
		//if too few coordinates are passed.
		if (!lua_rawgeti(L, i, 1) || !lua_rawgeti(L, i, 2) || !lua_rawgeti(L, i, 3)) {
			luaL_loadstring(L, "print('Error: number of components.')");
			lua_pcall(L, 0, 0, 0);
			lua_pop(L, 1);
			return 0;
		}
		//check to make sure all elements are numbers
		luaL_checknumber(L, -1); luaL_checknumber(L, -2); luaL_checknumber(L, -3);
		//get all coordinates
		float zi = lua_tonumber(L, -1);
		float yi = lua_tonumber(L, -2);
		float xi = lua_tonumber(L, -3);
		vertices[index][0] = xi; vertices[index][1] = yi; vertices[index][2] = zi;
		index++;
	}
	SMesh* mesh = new SMesh();

	SMeshBuffer *buf = 0;
	buf = new SMeshBuffer();
	mesh->addMeshBuffer(buf);
	buf->drop();

	buf->Vertices.reallocate(3);
	buf->Vertices.set_used(3);

	for (int i = 0; i < 3; i++) {
		std::cout << "Table[" << i << "] coordinates: " << vertices[i][0] << " " << vertices[i][1] << " " << vertices[i][2] << "\n";
		buf->Vertices[i] = irr::video::S3DVertex(vertices[i][0], vertices[i][1], vertices[i][3], 0, 1, 0, irr::video::SColor(255, 0, 255, 255), 0, 1);
	}

	buf->Indices.reallocate(3);
	buf->Indices.set_used(3);

	buf->Indices[0] = 0;
	buf->Indices[1] = 1;
	buf->Indices[2] = 2;

	buf->recalculateBoundingBox();


	IMeshSceneNode* myNode = device->getSceneManager()->addMeshSceneNode(mesh);

	myNode->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, false);
	myNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
	myNode->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, false);
	
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
	lua_pushcfunction(L, addMesh);
	lua_setglobal(L, "addMesh");
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

	device = irr::createDevice(irr::video::EDT_SOFTWARE, irr::core::dimension2d<irr::u32>(640, 480), 16, false, false, true, 0);
	if(!device)
		return 1;

	device->setWindowCaption(L"Hello World! - Irrlicht Engine Demo");
	irr::video::IVideoDriver* driver	= device->getVideoDriver();
	irr::scene::ISceneManager* smgr		= device->getSceneManager();
	irr::gui::IGUIEnvironment* guienv	= device->getGUIEnvironment();

	guienv->addStaticText(L"Hello World! This is the Irrlicht Software renderer!", irr::core::rect<irr::s32>(10, 10, 260, 22), true);

	irr::scene::IAnimatedMesh* mesh = smgr->getMesh("../Meshes/sydney.md2");
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
		node->setMaterialTexture(0, driver->getTexture("../Meshes/sydney.bmp"));
	}

	node->setScale(irr::core::vector3df(0.5f, 0.5f, 0.5f));
	
	//smgr->addCameraSceneNode(0, irr::core::vector3df(0, 30, -40), irr::core::vector3df(0, 5, 0));

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