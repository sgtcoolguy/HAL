/**
 * HAL
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#include "HAL/JSFunction.hpp"
#include "HAL/JSString.hpp"
#include "HAL/JSValue.hpp"
#include "HAL/JSUndefined.hpp"
#include "HAL/detail/JSUtil.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace HAL {

JSFunction::JSFunction(const JSContext& js_context, const JSString& body, const std::vector<JSString>& parameter_names, const JSString& function_name, const JSString& source_url, int starting_line_number)
        : JSObject(js_context, MakeFunction(js_context, body, parameter_names, function_name, source_url, starting_line_number)) {
}

JSFunction::JSFunction(const JSContext& js_context, const JSString& function_name, const JSFunctionCallback& callback)
        : JSObject(js_context, MakeFunction(js_context, function_name, callback)) {
}

JSFunction::JSFunction(const JSFunction& rhs) : JSObject(rhs) {
    RetainCallbackAfterCopy();
}

JSFunction::JSFunction(JSFunction&& rhs) : JSObject(std::move(rhs)) {
    RetainCallbackAfterCopy();
}

JSFunction& JSFunction::operator=(const JSFunction& rhs) {
    JSObject::operator=(rhs);
    RetainCallbackAfterCopy();
    return *this;
}

JSFunction& JSFunction::operator=(JSFunction&& rhs) {
    JSObject::operator=(std::move(rhs));
    RetainCallbackAfterCopy();
    return *this;
}

/**
 * To keep the callback valid after the destruction of rhs,
 * we must obtain a new JSObjectRef and register the callback with it.
 * The rhs will unregister the callback with the original js_object_ref__ in destructor.
 */
void JSFunction::RetainCallbackAfterCopy() {
    const auto &callback = FindJSFunctionCallback(js_object_ref__);
    if (callback) {
        JSValue name(js_context__, JSObjectGetProperty(static_cast<JSContextRef>(js_context__), js_object_ref__, static_cast<JSStringRef>(JSString("name")), nullptr));
        std::string name_string = static_cast<std::string>(name);
        UnRegisterJSContext(js_object_ref__);
        js_object_ref__ = MakeFunction(js_context__, static_cast<JSString>(name), callback);
        JSFunction::RegisterJSFunctionCallback(js_object_ref__, callback);
    }
}

JSObjectRef JSFunction::MakeFunction(const JSContext& js_context, const JSString& body, const std::vector<JSString>& parameter_names, const JSString& func_name, const JSString& source_url, int starting_line_number) {

    JSString function_name = func_name;
    if (function_name == "") {
        function_name = JSString("anonymous");
    }

    JSValueRef exception { nullptr };
    JSStringRef source_url_ref = (source_url.length() > 0) ? static_cast<JSStringRef>(source_url) : nullptr;
    JSObjectRef js_object_ref = nullptr;
    if (!parameter_names.empty()) {
        std::vector<JSStringRef> parameter_name_array = detail::to_vector(parameter_names);
        js_object_ref = JSObjectMakeFunction(static_cast<JSContextRef>(js_context), static_cast<JSStringRef>(function_name), static_cast<unsigned>(parameter_name_array.size()), &parameter_name_array[0], static_cast<JSStringRef>(body), source_url_ref, starting_line_number, &exception);
    } else {
        js_object_ref = JSObjectMakeFunction(static_cast<JSContextRef>(js_context), static_cast<JSStringRef>(function_name), 0, nullptr, static_cast<JSStringRef>(body), source_url_ref, starting_line_number, &exception);
    }
    
    if (exception) {
        // If this assert fails then we need to JSValueUnprotect
        // js_object_ref.
        assert(!js_object_ref);
        detail::ThrowRuntimeError("JSFunction", JSValue(js_context, exception));
    }

    return js_object_ref;
}

std::unordered_map<std::intptr_t, JSFunctionCallback> JSFunction::js_object_ref_to_js_function__;

void JSFunction::RegisterJSFunctionCallback(JSObjectRef js_object_ref, JSFunctionCallback callback) {
    HAL_JSOBJECT_LOCK_GUARD_STATIC;
    const auto key   = reinterpret_cast<std::intptr_t>(js_object_ref);
    const auto value = callback;
    const auto position = js_object_ref_to_js_function__.find(key);
    const bool found    = position != js_object_ref_to_js_function__.end();
    
    if (found) {
      HAL_LOG_DEBUG("JSFunction::RegisterJSFunctionCallback: JSObjectRef ", js_object_ref, " already registered");
    } else {
      const auto insert_result = js_object_ref_to_js_function__.emplace(key, value);
      const bool inserted      = insert_result.second;
      
      assert(inserted);
    }
}

void JSFunction::UnRegisterJSFunctionCallback(JSObjectRef js_object_ref) {
    HAL_JSOBJECT_LOCK_GUARD_STATIC;
    const auto key = reinterpret_cast<std::intptr_t>(js_object_ref);
    js_object_ref_to_js_function__.erase(key);
}

JSFunctionCallback JSFunction::FindJSFunctionCallback(JSObjectRef js_object_ref) {
    HAL_JSOBJECT_LOCK_GUARD_STATIC;
    const auto key      = reinterpret_cast<std::intptr_t>(js_object_ref);
    const auto position = js_object_ref_to_js_function__.find(key);
    const bool found    = position != js_object_ref_to_js_function__.end();
    
    if (found) {
      return position->second;
    } else {
        return nullptr;
    }
}

JSValueRef JSFunction::JSObjectCallAsFunctionCallback(JSContextRef context_ref, JSObjectRef function_ref, JSObjectRef this_object_ref, size_t argument_count, const JSValueRef arguments_array[], JSValueRef* exception) {
    const auto callback = FindJSFunctionCallback(function_ref);
    if (callback == nullptr) {
        return JSValueMakeUndefined(context_ref);
    }
    const auto ctx = JSContext(context_ref);
    std::vector<JSValue> arguments;
    arguments.reserve(argument_count);
    for (size_t i = 0; i < argument_count; i++) {
        arguments.push_back(JSValue(ctx, arguments_array[i]));
    }
    auto this_object = JSObject(ctx, this_object_ref);
    return static_cast<JSValueRef>(callback(arguments, this_object));
}

JSObjectRef JSFunction::MakeFunction(const JSContext& js_context, const JSString& function_name, const JSFunctionCallback& callback) {
    JSObjectRef js_object_ref = JSObjectMakeFunctionWithCallback(static_cast<JSContextRef>(js_context), static_cast<JSStringRef>(function_name), JSFunction::JSObjectCallAsFunctionCallback);
    JSFunction::RegisterJSFunctionCallback(js_object_ref, callback);
    return js_object_ref;
}

JSFunction::~JSFunction() HAL_NOEXCEPT {
    JSFunction::UnRegisterJSFunctionCallback(js_object_ref__);
}
    
} // namespace HAL {
