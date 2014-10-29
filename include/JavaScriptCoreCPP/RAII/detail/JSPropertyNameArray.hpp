/**
 * JavaScriptCoreCPP
 * Author: Matthew D. Langston
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#ifndef _JAVASCRIPTCORECPP_RAII_DETAIL_JSPROPERTYNAMEARRAY_HPP_
#define _JAVASCRIPTCORECPP_RAII_DETAIL_JSPROPERTYNAMEARRAY_HPP_

#include "JavaScriptCoreCPP/RAII/JSObject.hpp"
#include "JavaScriptCoreCPP/RAII/JSString.hpp"
#include <vector>
#include <algorithm>
#include <cassert>
#include <JavaScriptCore/JavaScript.h>

namespace JavaScriptCoreCPP { namespace detail {

using namespace JavaScriptCoreCPP::RAII;

/*!
  @class
  
  @discussion A JSPropertyNameArray is an RAII wrapper around a
  JSPropertyNameArrayRef, the JavaScriptCore C API representation of
  an array of JavaScript property names.
*/
class JSPropertyNameArray final	{
	
private:

	// This class is only intended to be used as a helper class by
	// JSObject.
	friend class RAII::JSObject;
	
	/*!
	  @method
	  
	  @abstract Gets the names of an object's enumerable properties.
	  
	  @param object The object whose property names you want to get.
	  
	  @result A JSPropertyNameArray containing the names of the object's
	  enumerable properties.
	*/
	JSPropertyNameArray(const JSObject& js_object) : js_property_name_array_ref__(JSObjectCopyPropertyNames(js_object.get_context(), js_object)) {
	}
  
	/*!
	  @method
	  
	  @abstract Return the number of items in this JavaScript property
	  name array.
	  
	  @result The number of names in this JavaScript property name
	  array.
	*/
	size_t GetCount() const {
		return JSPropertyNameArrayGetCount(js_property_name_array_ref__);
	}
	
	/*!
	  @method
	  
	  @abstract Gets a property name at a given index in this JavaScript
	  property name array.
	  
	  @param index The index of the property name to retrieve.
	  
	  @result A JSString containing the property name.
	*/
	JSString GetNameAtIndex(size_t index) const {
		return JSPropertyNameArrayGetNameAtIndex(js_property_name_array_ref__, index);
	}
	
	operator std::vector<JSString>() const {
		std::vector<JSString> property_names;
		for (size_t i = 0, count = GetCount(); i < count; ++i) {
			property_names.emplace_back(GetNameAtIndex(i));
		}

		return property_names;
	}

	~JSPropertyNameArray() {
	  JSPropertyNameArrayRelease(js_property_name_array_ref__);
  }

	// Copy constructor.
	JSPropertyNameArray(const JSPropertyNameArray& rhs) : js_property_name_array_ref__(rhs.js_property_name_array_ref__) {
		JSPropertyNameArrayRetain(js_property_name_array_ref__);
	}
	
  // Move constructor.
  JSPropertyNameArray(JSPropertyNameArray&& rhs) : js_property_name_array_ref__(rhs.js_property_name_array_ref__) {
	  JSPropertyNameArrayRetain(js_property_name_array_ref__);
  }
  
  // Create a copy of another JSPropertyNameArray by assignment. This
  // is a unified assignment operator that fuses the copy assignment
  // operator, X& X::operator=(const X&), and the move assignment
  // operator, X& X::operator=(X&&);
  JSPropertyNameArray& operator=(JSPropertyNameArray rhs) {
    swap(*this, rhs);
    return *this;
  }
  
  friend void swap(JSPropertyNameArray& first, JSPropertyNameArray& second) noexcept {
    // enable ADL (not necessary in our case, but good practice)
    using std::swap;
    
    // by swapping the members of two classes,
    // the two classes are effectively swapped
    swap(first.js_property_name_array_ref__, second.js_property_name_array_ref__);
  }
  
  // For interoperability with the JavaScriptCore C API.
  operator JSPropertyNameArrayRef() const {
	  return js_property_name_array_ref__;
  }

	// Prevent heap based objects.
	static void * operator new(size_t);			 // #1: To prevent allocation of scalar objects
	static void * operator new [] (size_t);	 // #2: To prevent allocation of array of objects
	
	JSPropertyNameArrayRef js_property_name_array_ref__;
};

}} // namespace JavaScriptCoreCPP { namespace detail {

#endif // _JAVASCRIPTCORECPP_RAII_DETAIL_JSPROPERTYNAMEARRAY_HPP_