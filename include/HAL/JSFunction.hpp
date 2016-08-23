/**
 * HAL
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#ifndef _HAL_JSFUNCTION_HPP_
#define _HAL_JSFUNCTION_HPP_

#include "HAL/JSObject.hpp"
#include <functional>
#include <unordered_map>

namespace HAL {

    
/*!
  @class
  
  @discussion A JavaScript object of the Function type whose body is
  given as a string of JavaScript code. Use this class when you want
  to execute a script repeatedly to avoid the cost of re-parsing the
  script before each execution.

  The only way to create a JSFunction is by using the
  JSContext::CreateFunction member function.
*/
class HAL_EXPORT JSFunction final : public JSObject HAL_PERFORMANCE_COUNTER2(JSFunction) {

public:
    
    static void RegisterJSFunctionCallback(JSObjectRef js_object_ref, JSFunctionCallback);
    static void UnRegisterJSFunctionCallback(JSObjectRef js_object_ref);
    static JSFunctionCallback FindJSFunctionCallback(JSObjectRef js_object_ref);

    JSFunction(const JSFunction& rhs);
    JSFunction(JSFunction&& rhs);
    JSFunction& operator=(const JSFunction& rhs);
    JSFunction& operator=(JSFunction&& rhs);

    virtual ~JSFunction() HAL_NOEXCEPT;

private:
    
    // Only a JSContext can create a JSFunction.
    friend JSContext;
    
    JSFunction(const JSContext& js_context, const JSString& body, const std::vector<JSString>& parameter_names, const JSString& function_name, const JSString& source_url, int starting_line_number);
    JSFunction(const JSContext& js_context, const JSString& function_name, const JSFunctionCallback& callback);

    static JSObjectRef MakeFunction(const JSContext& js_context, const JSString& body, const std::vector<JSString>& parameter_names, const JSString& function_name, const JSString& source_url, int starting_line_number);

    static JSValueRef  JSObjectCallAsFunctionCallback(JSContextRef context_ref, JSObjectRef function_ref, JSObjectRef this_object_ref, size_t argument_count, const JSValueRef arguments_array[], JSValueRef* exception);
    static JSObjectRef MakeFunction(const JSContext& js_context, const JSString& function_name, const JSFunctionCallback& callback);

    void RetainCallbackAfterCopy();

    // Silence 4251 on Windows since private member variables do not
    // need to be exported from a DLL.
#pragma warning(push)
#pragma warning(disable: 4251)
    static std::unordered_map<std::intptr_t, JSFunctionCallback> js_object_ref_to_js_function__;
#pragma warning(pop)

};

} // namespace HAL {

#endif // _HAL_JSFUNCTION_HPP_
