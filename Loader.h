#pragma once
#include "Com/Com.h"
#include <stdexcept>

namespace Com
{
	namespace Import
	{
		class Loader
		{
		public:
			Loader()
			{
				GetInstance() = this;
			}
			~Loader()
			{
				GetInstance() = nullptr;
			}

			virtual void Reference(Pointer<ITypeLib> typeLibrary) = 0;

			static void AddReference(Pointer<ITypeLib> typeLibrary)
			{
				if (GetInstance() == nullptr)
					throw std::runtime_error("An instance of loader must exist prior to calling AddReference.");
				GetInstance()->Reference(typeLibrary);
			}

		private:
			static Loader*& GetInstance()
			{
				static Loader* instance = nullptr;
				return instance;
			}
		};
	}
}
