#ifndef __LIB_LIGHTMETRICA_DISPATCHER_H__
#define __LIB_LIGHTMETRICA_DISPATCHER_H__

#include "common.h"
#include <boost/signals2.hpp>

LM_NAMESPACE_BEGIN

class NanonConfig;

/*!
	Renderer dispatcher.
	Create renderer from the configuration and execute rendering.
*/
class LM_PUBLIC_API RendererDispatcher
{
public:

	RendererDispatcher();
	~RendererDispatcher();

private:

	LM_DISABLE_COPY_AND_MOVE(RendererDispatcher);

public:

	/*!
		Dispatch the renderer.
		Dispatch rendering with the specified renderer in the configuration.
		We note that the function creates a new thread for rendering.
		\param config Configuration.
	*/
	void Dispatch(const NanonConfig& config);

public:

	/*!
		Connect to the Progress signal.
		The signal is emitted when the progress of the renderer is reported.
		\param func Slot function.
	*/
	boost::signals2::connection Connect_Progress(const std::function<void ()>& func);
	
public:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_DISPATCHER_H__