/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pch.test.h"
#include <lightmetrica.test/proxylistener.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/fp.h>

int main(int argc, char** argv)
{
	// Initialize Google test
	testing::InitGoogleTest(&argc, argv);

	// Replace default printer
	auto& listeners = testing::UnitTest::GetInstance()->listeners();
	auto defaultPriner = listeners.Release(listeners.default_result_printer());
	listeners.Append(new lightmetrica::test::ProxyTestEventListener(defaultPriner));

	// Floating-point control
#if LM_STRICT_FP && LM_PLATFORM_WINDOWS
	lightmetrica::FloatintPointUtils::EnableFPControl();
#endif

	return RUN_ALL_TESTS();
}