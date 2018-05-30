/**
 * HAL
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#ifndef _HAL_JSERROR_HPP_
#define _HAL_JSERROR_HPP_

#include "HAL/JSValue.hpp"
#include "HAL/JSObject.hpp"
#include <vector>
#include <deque> 

namespace HAL {

/*!
  @class
  
  @discussion A JavaScript object of the Error type.

  The only way to create a JSError is by using the
  JSContext::CreateError member function.
*/
class HAL_EXPORT JSError final : public JSObject HAL_PERFORMANCE_COUNTER2(JSError) {
 public:
#pragma warning(push)
#pragma warning(disable : 4251)
 	static std::deque<std::string> NativeStack__;
 	static std::string GetNativeStack();
 	static void ClearNativeStack();
#pragma warning(pop)

 	std::string message() const;
 	std::string name() const;
 	std::string filename() const;
 	std::uint32_t linenumber() const;
 	std::string stack() const;
 	std::string nativeStack() const;
	
 private:
	
	// Only JSContext and JSObject can create a JSError.
	friend JSContext;
	friend JSObject;
	
	JSError(const JSContext& js_context, const std::vector<JSValue>& arguments = {});

	// For interoperability with the JavaScriptCore C API.
	JSError(const JSContext& js_context, JSObjectRef js_object_ref);

	static JSObjectRef MakeError(const JSContext& js_context, const std::vector<JSValue>& arguments);

};

} // namespace HAL {

#endif // _HAL_JSERROR_HPP_
