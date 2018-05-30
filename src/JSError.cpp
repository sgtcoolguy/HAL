/**
 * HAL
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#include "HAL/JSError.hpp"
#include "HAL/JSValue.hpp"
#include "HAL/JSObject.hpp"
#include "HAL/JSString.hpp"
#include "HAL/JSArray.hpp"
#include "HAL/detail/JSUtil.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace HAL {

std::deque<std::string> JSError::NativeStack__; 

JSError::JSError(const JSContext& js_context, const std::vector<JSValue>& arguments)
		: JSObject(js_context, MakeError(js_context, arguments)) {
	SetProperty("nativeStack", js_context.CreateString(JSError::GetNativeStack()));
}

JSError::JSError(const JSContext& js_context, JSObjectRef js_object_ref)
		: JSObject(js_context, js_object_ref) {
	SetProperty("nativeStack", js_context.CreateString(JSError::GetNativeStack()));
}

std::string JSError::message() const {
	if (HasProperty("message")) {
		return static_cast<std::string>(GetProperty("message"));
	}
	return "";
}

std::string JSError::name() const {
	if (HasProperty("name")) {
		return static_cast<std::string>(GetProperty("name"));
	}
	return "";
}

std::string JSError::filename() const {
	if (HasProperty("fileName")) {
		return static_cast<std::string>(GetProperty("fileName"));
	}
	return "";
}

std::uint32_t JSError::linenumber() const {
	if (HasProperty("lineNumber")) {
		return static_cast<std::uint32_t>(GetProperty("lineNumber"));
	}
	return 0;
}

std::string JSError::stack() const {
	if (HasProperty("stack")) {
		return static_cast<std::string>(GetProperty("stack"));
	}
	return "";
}

std::string JSError::nativeStack() const {
	if (HasProperty("nativeStack")) {
		return static_cast<std::string>(GetProperty("nativeStack"));
	}
	return "";
}

std::string JSError::GetNativeStack() {
	std::ostringstream stacktrace;
	for (auto iter = JSError::NativeStack__.rbegin(); iter != JSError::NativeStack__.rend(); ++iter) {
		stacktrace << (std::distance(JSError::NativeStack__.rbegin(), iter) + 1) << "  " << *iter << "\n";
	}
	return stacktrace.str();
}

void JSError::ClearNativeStack() {
	JSError::NativeStack__.clear();
}

JSObjectRef JSError::MakeError(const JSContext& js_context, const std::vector<JSValue>& arguments) {
	JSValueRef exception { nullptr };
	JSObjectRef js_object_ref = nullptr;
	if (!arguments.empty()) {
		std::vector<JSValueRef> arguments_array = detail::to_vector(arguments);
		js_object_ref = JSObjectMakeError(static_cast<JSContextRef>(js_context), arguments_array.size(), &arguments_array[0], &exception);
	} else {
		js_object_ref = JSObjectMakeError(static_cast<JSContextRef>(js_context), 0, nullptr, &exception);
	}
	
	if (exception) {
		// If this assert fails then we need to JSValueUnprotect
		// js_object_ref.
		assert(!js_object_ref);
		detail::ThrowRuntimeError("JSError", JSValue(js_context, exception));
	}

	return js_object_ref;
}

} // namespace HAL {
