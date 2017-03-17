/*
Copyright 2017 Zachary Blystone <zakblystone@gmail.com>

This file is part of Metacade.

Metacade is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Metacade is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Metacade.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
===============================================================================
lua_vm.cpp:
===============================================================================
*/

#include "lua_private.h"

#include <iostream>
#include <fstream>

static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
	unsigned int *Usage = (unsigned int *)ud;

	if (nsize == 0) {
		*Usage -= (unsigned int)osize;
		free(ptr);
		return NULL;
	}
	else
	{
		*Usage += (unsigned int)(nsize - osize);
		return realloc(ptr, nsize);
	}
}

static int l_print(lua_State *S)
{
	const char *STR = lua_tostring(S, 1);

	std::cout << STR << std::endl;
	return 0;
}

LuaVM::LuaVM()
	: _L(nullptr)
	, _memUsage(0)
{

}

LuaVM::~LuaVM()
{
	shutdown();
}

Arcade::ELanguage LuaVM::getLanguage()
{
	return LANG_LUA;
}

bool LuaVM::init()
{
	if ( _L != nullptr ) return true;

	_memUsage = 0;
	_L = lua_newstate(l_alloc, &_memUsage);

	luaJIT_setmode(_L, -1, LUAJIT_MODE_DEBUG | LUAJIT_MODE_ON);
	luaJIT_setmode(_L, -1, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);

	luaopen_base(_L);
	luaopen_bit(_L);
	luaopen_math(_L);
	luaopen_table(_L);

	OpenLuaMathModule(_L);
	OpenLuaDrawModule(_L);

	//check errors

	return true;
}

void LuaVM::shutdown()
{
	if ( _L == nullptr ) return;

	lua_close(_L);

	_L = nullptr;
}

bool Arcade::LuaVM::isRunning()
{
	return _L != nullptr;
}

bool Arcade::LuaVM::pcall(int nargs)
{
	if (lua_pcall(_L, nargs, 0, 0)) 
	{
		std::cout << "Lua: " << lua_tostring(_L, -1) << std::endl;
		lua_pop(_L, 1);
		return false;
	}
	return true;
}

lua_State* Arcade::LuaVM::getState()
{
	return _L;
}

bool LuaVM::pushVariant(const CVariant& variant)
{
	switch(variant.type())
	{
	case Arcade::VT_NONE:
		lua_pushnil(_L);
		return true;
	break;
	case Arcade::VT_BOOLEAN:
		{
			bool b;
			if ( variant.get(b) ) { lua_pushboolean(_L, b); return true; }
		}
	break;
	case Arcade::VT_UINT:
		{
			uint64 v;
			if ( variant.get(v) ) { lua_pushnumber(_L, (lua_Number) v); return true; }
		}
	break;
	case Arcade::VT_INT:
		{
			int64 v;
			if ( variant.get(v) ) { lua_pushnumber(_L, (lua_Number) v); return true; }
		}
	break;
	case Arcade::VT_DOUBLE:
		{
			double v;
			if ( variant.get(v) ) { lua_pushnumber(_L, (lua_Number) v); return true; }
		}
	break;
	case Arcade::VT_STRING:
		{
			CString str;
			if ( variant.get(str) ) { lua_pushstring(_L, *str); return true; }
		}
	break;
	default:
		lua_pushnil(_L);
		return true;
	break;
	}

	return false;
}

static int testMetaGet(lua_State *L)
{
	std::cout << "META GET" << std::endl;
	return 0;
}

int LuaVMClass::testMetaSet(lua_State *L)
{
	lua_getfield(L, 1, "__klass");

	LuaVMClass *klass = (LuaVMClass *) lua_touserdata(L, -1);
	const char *key = lua_tostring(L, 2);

	if ( klass != nullptr )
	{
		int type = lua_type(L, 3);

		if ( type == LUA_TFUNCTION )
		{
			lua_getglobal(L, "_G");
			lua_setfenv(L, 3);

			auto entry = make_pair(std::string(key), make_shared<LuaVMReference>(klass->_host, 3));
			klass->_functions.insert(entry);
		}
		else
		{
			luaL_error(L, "functions only");
		}
	}

	return 0;
}

Arcade::IVMClass* LuaVM::loadGameVMClass()
{
	string filename("E:/Projects/metacade/bin/Release/test.lua");

	shared_ptr<LuaVMClass> newClass;

	auto found = _loadedClasses.find(filename);
	if ( found != _loadedClasses.end() )
	{
		return ((*found).second).get();
	}
	else
	{
		newClass = make_shared<LuaVMClass>(shared_from_this());
		_loadedClasses.insert(make_pair(filename, newClass));

		if ( newClass->loadFromFile(filename) )
		{
			return newClass.get();
		}
	}

	return nullptr;
}

bool LuaVM::includeGameScript()
{
	return false;
}

bool LuaVM::validateGameScript()
{
	return false;
}

extern shared_ptr<IVMHost> Arcade::getLuaVM()
{
	return make_shared<LuaVM>();
}

Arcade::LuaVMClass::LuaVMClass(shared_ptr<LuaVM> host)
	: _host(host)
{

}

Arcade::LuaVMClass::~LuaVMClass()
{

}

bool Arcade::LuaVMClass::reload()
{
	if ( _lastLoadFile != "" )
	{
		return loadFromFile(_lastLoadFile);
	}
	return false;
}

class CGameMetadata* Arcade::LuaVMClass::getMetaData()
{
	return nullptr;
}

class IVMHost* Arcade::LuaVMClass::getHost()
{
	return _host.get();
}

class IVMInstance* Arcade::LuaVMClass::createVMInstance()
{
	return new LuaVMInstance(shared_from_this());
}

void Arcade::LuaVMClass::shutdownVMInstance(IVMInstance* instance)
{
	delete instance;
}

bool Arcade::LuaVMClass::pushLuaFunction(string functionName) const
{
	auto found = _functions.find(functionName);
	if ( found != _functions.end() )
	{
		(*found).second->push();
		return true;
	}
	return false;
}

bool Arcade::LuaVMClass::loadFromFile(string filename)
{
	_lastLoadFile = filename;
	_functions.clear();

	std::fstream input(filename, std::ios::binary | std::ios::in | std::ios::ate);

	lua_State *L = _host->getState();

	if ( input.is_open() )
	{
		uint32 size = (uint32) input.tellg();
		input.seekg(0);

		if ( size == 0 ) return false;
		
		char *buffer = new char[size];
		input.read(buffer, size);

		if (luaL_loadbuffer(L, buffer, size, "main"))
		{
			std::cout << "Lua: main: " << lua_tostring(L, -1);
			lua_pop(L, 1);
			return nullptr;	
		}

		lua_newtable(L);

		lua_pushlightuserdata(L, this);
		lua_setfield(L, -2, "__klass");

		lua_newtable(L);
		lua_newtable(L);
		lua_setfield(L, -2, "sounds");

		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");

		lua_pushcclosure(L, LuaVMClass::testMetaSet, 0);
		lua_setfield(L, -2, "__newindex");

		lua_setmetatable(L, -2);
		lua_setfenv(L, -2);

		if (lua_pcall(L, 0, 0, 0)) {
			std::cout << "Lua: main: " << lua_tostring(L, -1);
			lua_pop(L, 1);
			return false;
		}

		return true;
	}

	return false;
}

//VM INSTANCE
Arcade::LuaVMInstance::LuaVMInstance(shared_ptr<LuaVMClass> klass)
	: _klass(klass)
{
	lua_State *L = _klass->_host->_L;

	lua_newtable(L);
	_object = make_shared<LuaVMReference>(_klass->_host, -1);

	for ( auto funcdef : klass->_functions )
	{
		funcdef.second->push();
		lua_setfield(L, -2, funcdef.first.c_str());
	}

	lua_pop(L, 1);

	if ( getLuaClass()->pushLuaFunction("init") ) pcall(0);
}

Arcade::LuaVMInstance::~LuaVMInstance()
{

}

class IVMClass* Arcade::LuaVMInstance::getClass()
{
	return _klass.get();
}

void Arcade::LuaVMInstance::setMachineEnvironment(IMachineEnvironment *env)
{

}

bool Arcade::LuaVMInstance::postCommand(const char** commandBuffer)
{
	return false;
}

void Arcade::LuaVMInstance::postInputEvent(const class CInputEvent& input)
{

}

void Arcade::LuaVMInstance::precacheAssets()
{

}

void Arcade::LuaVMInstance::think(float seconds, float deltaSeconds)
{
	if ( getLuaClass()->pushLuaFunction("think") )
	{
		lua_State *L = getLuaHost()->_L;

		lua_pushnumber(L, seconds);
		lua_pushnumber(L, deltaSeconds);
		pcall(2);
	}
}

void Arcade::LuaVMInstance::render(shared_ptr<CElementRenderer> renderer)
{
	if ( getLuaClass()->pushLuaFunction("draw") ) 
	{
		lua_State *L = getLuaHost()->_L;

		pushRenderer(L, renderer);
		lua_setglobal(L, "_r");

		pcall(0);

		lua_pushnil(L);
		lua_setglobal(L, "_r");
	}
}

void Arcade::LuaVMInstance::reset()
{
	if ( getLuaClass()->pushLuaFunction("reset") ) pcall(0);
}

bool LuaVMInstance::callFunction(CFunctionCall call)
{
	lua_State *L = getLuaHost()->_L;

	if ( !getLuaClass()->pushLuaFunction(*call.getFunction()) ) 
	{
		lua_pop(L, 1);
		return false;
	}

	int top = lua_gettop(L);

	for ( uint32 i=0; i<call.numArgs(); ++i )
	{
		if ( !getLuaHost()->pushVariant(call.getArg(i)) )
		{
			lua_settop(L, top);
			return false;
		}
	}

	return pcall(call.numArgs());
}

shared_ptr<Arcade::LuaVM> Arcade::LuaVMInstance::getLuaHost() const
{
	return _klass->getLuaHost();
}

bool Arcade::LuaVMInstance::pcall(int nargs)
{
	getLuaObject()->push();
	lua_setglobal(getLuaHost()->_L, "game");
	return getLuaHost()->pcall(nargs);
}

Arcade::LuaVMReference::LuaVMReference(shared_ptr<class LuaVM> host, int idx)
	: _host(host)
{
	lua_State *L = host->_L;
	
	lua_pushvalue(L, idx);
	_ref = luaL_ref(L, LUA_REGISTRYINDEX);
}

Arcade::LuaVMReference::~LuaVMReference()
{
	lua_State *L = _host->_L;

	if ( L == nullptr ) return;

	luaL_unref(L, LUA_REGISTRYINDEX, _ref);
}

void Arcade::LuaVMReference::push()
{
	lua_State *L = _host->_L;

	if ( L == nullptr ) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, _ref);
}
