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
variant.cpp:
===============================================================================
*/

#include "core_private.h"

static char *typenames[VT_MAX] = {
	"VT_NONE",
	"VT_BOOLEAN",
	"VT_UINT",
	"VT_INT",
	"VT_DOUBLE",
	"VT_STRING",
};

CVariant::CVariant() 
	: _type(VT_NONE)
	, _strdata(nullptr)
	, _strrefs(nullptr)
{

}

Arcade::CVariant::CVariant(const CVariant& other)
{
	_strrefs = other._strrefs;
	_strdata = other._strdata;
	_type = other._type;
	*((uint64 *)(_pod)) = *((uint64 *)(other._pod));

	if ( _strrefs != nullptr ) ++(*_strrefs);
}

CVariant& CVariant::operator=(const CVariant& other)
{
	if ( &other == this ) return *this;

	reset();

	_strrefs = other._strrefs;
	_strdata = other._strdata;
	_type = other._type;
	*((uint64 *)(_pod)) = *((uint64 *)(other._pod));

	if ( _strrefs != nullptr ) ++(*_strrefs);

	return *this;
}

CVariant::~CVariant()
{
	reset();
}

void CVariant::set(bool value)
{
	reset();
	*((uint64 *)(_pod)) = (uint64)(value);
	_type = VT_BOOLEAN;
}

void CVariant::set(uint64 value)
{
	reset();
	*((uint64 *)(_pod)) = value;
	_type = VT_UINT;
}

void CVariant::set(int64 value)
{
	reset();
	*((int64 *)(_pod)) = value;
	_type = VT_INT;
}

void CVariant::set(float value)
{
	reset();
	*((double *)(_pod)) = (double)(value);
	_type = VT_DOUBLE;
}

void CVariant::set(double value)
{
	reset();
	*((double *)(_pod)) = value;
	_type = VT_DOUBLE;
}

void CVariant::set(const char *value)
{
	if ( value == nullptr ) return;

	reset();

	if ( _strrefs == nullptr )
	{
		_strrefs = new uint32(1);
	}

	int32 length = (int32) strlen(value);

	_type = VT_STRING;
	*((int32 *)(_pod)) = length;
	
	_strdata = (uint8 *) new uint8[length * sizeof(char)];
	memcpy(_strdata, value, length * sizeof(char));
}

const char* CVariant::getTypeName() const
{
	return typenames[_type];
}

void CVariant::reset()
{
	*((uint64 *)(_pod)) = 0;

	if ( _strrefs && --(*_strrefs) == 0 )
	{
		delete [] _strdata;
		delete _strrefs;
		_strrefs = nullptr;
		_strdata = nullptr;
	}
}

EVariantType CVariant::type() const
{
	return _type;
}

bool CVariant::get(bool& value) const
{
	if ( _type != VT_BOOLEAN ) return false;

	value = *((uint64 *)(_pod)) != 0;
	return true;
}

bool CVariant::get(uint64& value) const
{
	if ( _type != VT_UINT ) return false;

	value = *((uint64 *)(_pod));
	return true;
}

bool CVariant::get(int64& value) const
{
	if ( _type != VT_INT ) return false;

	value = *((int64 *)(_pod));
	return true;
}

bool CVariant::get(double& value) const
{
	if ( _type != VT_DOUBLE ) return false;

	value = *((double *)(_pod));
	return true;
}

bool CVariant::get(const char*& buffer, int32& length) const
{
	if ( _type != VT_STRING ) return false;

	buffer = (const char *) _strdata;
	length = *((int32 *)(_pod));
	return true;
}