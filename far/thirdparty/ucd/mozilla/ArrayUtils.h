#pragma once

#define MOZ_ASSERT(x)

namespace mozilla
{
	template<typename T, size_t N>
	size_t ArrayLength(T (&arr)[N])
	{
		return N;
	}
}
