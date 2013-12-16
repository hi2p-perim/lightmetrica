#
# Project nanon.test
#

include (PCHTargets)

set(
	_SOURCE_FILES
	"common.h"
	"base.h"
	"base.cpp"
	"base.math.h"
	"stub.assetfactory.h"
	"stub.assetfactory.cpp"
	"stub.asset.h"
	"stub.asset.cpp"
	"test.logger.cpp"
	"test.config.cpp"
	"test.defaultassets.cpp"
	"test.asset.cpp"
	"test.scene.cpp"
	
	# math
	"test.vector.cpp"
	"test.matrix.cpp"
	
	# pugixml
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugiconfig.hpp"
	"${PROJECT_SOURCE_DIR}/external/src/pugixml-1.2/src/pugixml.cpp"
)

pch_add_executable(nanon.test PCH_HEADER "pch.h" ${_SOURCE_FILES})
target_link_libraries(nanon.test libnanon ${GTEST_BOTH_LIBRARIES})
add_dependencies(nanon.test libnanon)
