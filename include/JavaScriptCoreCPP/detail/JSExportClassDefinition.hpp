/**
 * Javascriptcorecpp
 * Author: Matthew D. Langston
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#ifndef _JAVASCRIPTCORECPP_DETAIL_JSEXPORTCLASSDEFINITION_HPP_
#define _JAVASCRIPTCORECPP_DETAIL_JSEXPORTCLASSDEFINITION_HPP_

#include "JavaScriptCoreCPP/detail/JSBase.hpp"
#include "JavaScriptCoreCPP/JSClassDefinition.hpp"
#include "JavaScriptCoreCPP/JSClassAttribute.hpp"

#include "JavaScriptCoreCPP/detail/JSExportNamedValuePropertyCallback.hpp"
#include "JavaScriptCoreCPP/detail/JSExportNamedFunctionPropertyCallback.hpp"
#include "JavaScriptCoreCPP/detail/JSExportCallbacks.hpp"

#include <string>
#include <unordered_map>

namespace JavaScriptCoreCPP { namespace detail {
  
  template<typename T>
  using JSExportNamedValuePropertyCallbackMap_t    = std::unordered_map<std::string, JSExportNamedValuePropertyCallback<T>>;
  
  template<typename T>
  using JSExportNamedFunctionPropertyCallbackMap_t = std::unordered_map<std::string, JSExportNamedFunctionPropertyCallback<T>>;
  
  template<typename T>
  class JSExportClassDefinitionBuilder;
  
  /*!
   @class
   
   @discussion A JSExportClassDefinition defines the properties and
   callbacks of a JavaScript object implemented by a C++ classed
   derived from JSExport.
   
   The only way to create a JSExportClassDefinition is by using a
   JSExportClassDefinitionBuilder.
   
   This class is thread safe and immutable by design.
   */
  template<typename T>
  class JSExportClassDefinition final : public JSClassDefinition JAVASCRIPTCORECPP_PERFORMANCE_COUNTER2(JSExportClassDefinition<T>) {
  public:
    
    JSExportClassDefinition(const JSExportClassDefinitionBuilder<T>& builder);
    
    JSExportNamedValuePropertyCallbackMap_t<T> get_named_value_property_callback_map() const {
      return named_value_property_callback_map__;
    }
    
    JSExportNamedFunctionPropertyCallbackMap_t<T> get_named_function_property_callback_map() const {
      return named_function_property_callback_map__;
    }
    
    JSExportClassDefinition()                                          = default;
    ~JSExportClassDefinition()                                         = default;
    JSExportClassDefinition(const JSExportClassDefinition&)            = default;
    JSExportClassDefinition(JSExportClassDefinition&&)                 = default;
    JSExportClassDefinition& operator=(const JSExportClassDefinition&) = default;
    JSExportClassDefinition& operator=(JSExportClassDefinition&&)      = default;
    
  private:
    
    void InitializeNamedPropertyCallbacks() noexcept;
    
    JSExportNamedValuePropertyCallbackMap_t<T>    named_value_property_callback_map__;
    JSExportNamedFunctionPropertyCallbackMap_t<T> named_function_property_callback_map__;
  };
  
  template<typename T>
  void JSExportClassDefinition<T>::InitializeNamedPropertyCallbacks() noexcept {
    
    // Initialize staticValues.
    static_values__.clear();
    js_class_definition__.staticValues = nullptr;
    if (!named_value_property_callback_map__.empty()) {
      for (const auto& entry : named_value_property_callback_map__) {
        const auto& property_name       = entry.first;
        const auto& property_attributes = entry.second.get_attributes();
        ::JSStaticValue static_value;
        static_value.name        = property_name.c_str();
        static_value.getProperty = JSExportClass<T>::GetNamedValuePropertyCallback;
        static_value.setProperty = JSExportClass<T>::SetNamedValuePropertyCallback;
        static_value.attributes  = ToJSPropertyAttributes(property_attributes);
        static_values__.push_back(static_value);
        JAVASCRIPTCORECPP_LOG_DEBUG("JSExportClassDefinition<", name__, "> added value property ", static_values__.back().name);
      }
      static_values__.push_back({nullptr, nullptr, nullptr, kJSPropertyAttributeNone});
      js_class_definition__.staticValues = &static_values__[0];
    }
    
    // Initialize staticFunctions.
    static_functions__.clear();
    js_class_definition__.staticFunctions = nullptr;
    if (!named_function_property_callback_map__.empty()) {
      for (const auto& entry : named_function_property_callback_map__) {
        const auto& function_name = entry.first;
        const auto& property_attributes = entry.second.get_attributes();
        ::JSStaticFunction static_function;
        static_function.name           = function_name.c_str();
        static_function.callAsFunction = JSExportClass<T>::CallNamedFunctionCallback;
        static_function.attributes     = ToJSPropertyAttributes(property_attributes);
        static_functions__.push_back(static_function);
        JAVASCRIPTCORECPP_LOG_DEBUG("JSExportClassDefinition<", name__, "> added function property ", static_functions__.back().name);
      }
      static_functions__.push_back({nullptr, nullptr, kJSPropertyAttributeNone});
      js_class_definition__.staticFunctions = &static_functions__[0];
    }
  }
  
}} // namespace JavaScriptCoreCPP { namespace detail {

#endif // _JAVASCRIPTCORECPP_DETAIL_JSEXPORTCLASSDEFINITION_HPP_
