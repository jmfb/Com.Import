#include "Loader.h"
#include <stdexcept>

namespace Com
{
	namespace Import
	{
		Loader::Loader()
		{
			GetInstance() = this;
		}

		Loader::~Loader()
		{
			GetInstance() = nullptr;
		}

		void Loader::AddReference(Pointer<ITypeLib> typeLibrary)
		{
			if (GetInstance() == nullptr)
				throw std::runtime_error("An instance of loader must exist prior to calling AddReference.");
			GetInstance()->Reference(typeLibrary);
		}

		Loader*& Loader::GetInstance()
		{
			static Loader* instance = nullptr;
			return instance;
		}
	}
}
