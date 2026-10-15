// Compiles public headers at the strictest warning level with unused parameters as errors, as a consumer
// building with /W4 or -Wextra does. The test is that this file compiles.

#if defined(_MSC_VER)
#pragma warning(push, 4)
#pragma warning(error : 4100)
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wunused-parameter"
#endif

#include "../crskit/IAuthorityProvider.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <gtest/gtest.h>

TEST(HeaderWarnings, IAuthorityProviderHasNoUnusedParameters)
{
	SUCCEED();
}
