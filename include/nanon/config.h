#ifndef __LIB_NANON_CONFIG_H__
#define __LIB_NANON_CONFIG_H__

#include "common.h"
#include <string>

NANON_NAMESPACE_BEGIN

/*!
	Configuration of the nanon renderer.
	The nanon renderer is configured by the XML document named nanon file (*.nanon).
	All configuration needed for rendering is contained the document.
*/
class NANON_PUBLIC_API NanonConfig
{
public:

	NanonConfig();
	~NanonConfig();

private:

	NANON_DISABLE_COPY_AND_MOVE(NanonConfig);
	
public:

	/*!
	*/
	bool Load(const std::string& path);

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_CONFIG_H__