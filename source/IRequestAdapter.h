#pragma once

#include <kj/string.h>

namespace gen{
	/*
		In the future this should be a interface for different kinds of implementations
		Currently this is the strictly single threaded variant
	*/
	class IRequestAdapter {
	public:
		virtual ~IRequestAdapter() = default;

		virtual kj::Promise<kj::String> interpret(kj::String request) = 0;
	};
}
