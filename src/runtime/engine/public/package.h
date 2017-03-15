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
package.h:
===============================================================================
*/

#pragma once

namespace Arcade
{

class METACADE_API CPackage : public CRuntimeObject
{
public:
	~CPackage();

	template<typename T>
	T* addAsset()
	{
		T* newAsset = new T(this);
		if ( !addAssetImplementation(newAsset) )
		{
			delete newAsset;
			return nullptr;
		}
		newAsset->setUniqueID(CGUID::generate());
		return newAsset;
	}

	void removeAsset(class IAsset* asset);

	int32 getNumAssets() const;
	class IAsset* getAsset(int32 index) const;

	bool save(IFileObject* file);
	bool load(IFileObject* file);

	const char* getPackageName();
	bool hasPackageFlag(EPackageFlags flag);
	int32 getPackageFlags();

private:

	friend class CPackageManager;

	CPackage(CRuntimeObject* outer, IFileObject* file = nullptr);

	bool addAssetImplementation(class IAsset* asset);

	IFileObject* _file;
	class CAssetMap *_map;
};

}