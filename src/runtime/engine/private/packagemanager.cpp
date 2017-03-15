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
packagemanager.cpp:
===============================================================================
*/

#include "engine_private.h"

CPackageManager::CPackageManager(CRuntimeObject* outer) 
	: CRuntimeObject(outer)
	, _rootPath(".")
{
}

CPackageBuilder* CPackageManager::createPackageBuilder()
{
	shared_ptr<CPackageBuilder> builder = shared_ptr<CPackageBuilder>( new CPackageBuilder(new CPackage(this, nullptr)) );
	_builders.push_back(builder);
	return builder.get();
}

void CPackageManager::deletePackageBuilder(CPackageBuilder* builder)
{
	for ( auto it = _builders.begin(); it != _builders.end(); ++it )
	{
		if ((*it).get() == builder)
		{
			_builders.erase(it);
			return;
		}
	}
}

void CPackageManager::setRootDirectory(const CString& path)
{
	_rootPath = path;
}

CString CPackageManager::getRootDirectory() const
{
	return _rootPath;
}

bool CPackageManager::findAndPreloadPackages()
{
	_references.clear();

	CFileCollection files;
	if (!listFilesInDirectory(&files, *(_rootPath+"/"), "mpkg"))
	{
		return false;
	}

	for ( uint32 i=0; i<files.numFiles(); ++i )
	{
		CString filename = files.getFile(i);
		log(LOG_MESSAGE, "Try load package %s", *filename);

		IFileObject* file = openFile(filename, FILE_READ);

		if ( file == nullptr )
		{
			log(LOG_WARN, "Failed to open %s", *filename);
			continue;
		}

		shared_ptr<CPackage> newPackage = shared_ptr<CPackage>(new CPackage(this, file));
		_references.push_back(newPackage);

		if ( !newPackage->load() )
		{
			log(LOG_WARN, "Failed to load package %s", *filename);
		}
		else
		{
			log(LOG_MESSAGE, "Loaded package: %s", *filename);
		}
	}

	return true;
}

uint32 CPackageManager::getNumPackages() const
{
	return (uint32) _references.size();
}

IPackage* CPackageManager::getPackage(uint32 index) const
{
	return _references[index].get();
}

void CPackageManager::unloadAllPackages()
{
	_references.clear();
}
