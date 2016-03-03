#pragma once
#include "Com/Com.h"

namespace Com
{
	namespace Import
	{
		class Loader
		{
		public:
			Loader();
			~Loader();

			virtual void Reference(Pointer<ITypeLib> typeLibrary) = 0;

			static void AddReference(Pointer<ITypeLib> typeLibrary);

		private:
			static Loader*& GetInstance();
		};
	}
}
