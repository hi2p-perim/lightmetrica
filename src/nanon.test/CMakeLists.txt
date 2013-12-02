#
# Project nanon.test
#

include (PCHTargets)

set(
	_SOURCE_FILES
	"common.h"
	"base.h"
	"base.cpp"
	"test.logger.cpp"
)

pch_add_executable(nanon.test PCH_HEADER "pch.h" ${_SOURCE_FILES})
target_link_libraries(nanon.test libnanon ${GTEST_BOTH_LIBRARIES})
add_dependencies(nanon.test libnanon)
