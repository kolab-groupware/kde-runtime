/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/                                                                            

#include <kapp.h>
#include <kcmodule.h>
#include <kservice.h>
#include <klibloader.h>
#include <kdebug.h>


int main(int argc, char *argv[])
{
  KApplication app(argc, argv, "kcminit");

  // locate the desktop files
  KService::List list = KService::allInitServices();

  // get the library loader instance
  KLibLoader *loader = KLibLoader::self();

  // look for X-KDE-Init=... entries
  for(KService::List::Iterator it = list.begin();
      it != list.end();
      ++it)
  {
     KService::Ptr service = (*it);
     if (service->library().isEmpty() || service->init().isEmpty())
        continue; // Skip
     // try to load the library
     QString libName = QString("libkcm_%1").arg(service->library());
     KLibrary *lib = loader->library(libName);
     if (lib)
     {
        // get the init_ function
	QString factory = QString("init_%1").arg(service->init());
	void *init = lib->symbol(factory);
        if (init)
	{
	   // initialize the module
	   kdDebug() << "Initializing " << libName << ": " << factory << endl;

	   void (*func)() = (void(*)())init;
	   func();
	}
        loader->unloadLibrary(libName);
     }
  }

  return 0;
}
