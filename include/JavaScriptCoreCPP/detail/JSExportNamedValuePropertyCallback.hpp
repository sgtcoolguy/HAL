/**
 * JavaScriptCoreCPP
 * Author: Matthew D. Langston
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#ifndef _JAVASCRIPTCORECPP_DETAIL_JSEXPORTNAMEDVALUEPROPERTYCALLBACK_HPP_
#define _JAVASCRIPTCORECPP_DETAIL_JSEXPORTNAMEDVALUEPROPERTYCALLBACK_HPP_

#include "JavaScriptCoreCPP/detail/JSBase.hpp"
#include "JavaScriptCoreCPP/detail/JSPropertyCallback.hpp"
#include "JavaScriptCoreCPP/detail/JSExportCallbacks.hpp"
#include "JavaScriptCoreCPP/detail/JSUtil.hpp"

namespace JavaScriptCoreCPP { namespace detail {
  
  /*!
   @class
   
   @discussion A JSExportNamedValuePropertyCallback is an RAII wrapper
   around the JavaScriptCore C API JSStaticValue, which describes a
   value property of a JavaScript object.
   */
  template<typename T>
  class JSExportNamedValuePropertyCallback final : public JSPropertyCallback JAVASCRIPTCORECPP_PERFORMANCE_COUNTER2(JSExportNamedValuePropertyCallback<T>) {
    
  public:
    
    /*!
     @method
     
     @abstract Set the callbacks to invoke when getting and setting
     a property value on a JavaScript object.
     
     @param name The value property's name.
     
     @param get_callback The callback to invoke when getting a
     property's value from a JavaScript object.
     
     @param set_callback The callback to invoke when setting a
     property's value on a JavaScript object. This may be nullptr,
     in which case the ReadOnly attribute is automatically set.
     
     @param attributes The set of JSPropertyAttributes to give to
     the value property.
     
     @result An object which describes a JavaScript value property.
     
     @throws std::invalid_argument exception under these
     preconditions:
     
     1. If property_name is empty.
     
     2. If the ReadOnly attribute is set and the get_callback is not
     provided.
     
     3. If the ReadOnly attribute is set and the set_callback is
     provided.
     
     4. If both get_callback and set_callback are missing.
     */
    JSExportNamedValuePropertyCallback(const std::string& property_name,
                                       GetNamedValuePropertyCallback<T> get_callback,
                                       SetNamedValuePropertyCallback<T> set_callback,
                                       const std::unordered_set<JSPropertyAttribute>& attributes);
    
    GetNamedValuePropertyCallback<T> get_callback() const JAVASCRIPTCORECPP_NOEXCEPT {
      return get_callback__;
    }
    
    SetNamedValuePropertyCallback<T> set_callback() const JAVASCRIPTCORECPP_NOEXCEPT {
      return set_callback__;
    }
    
    ~JSExportNamedValuePropertyCallback() = default;
    JSExportNamedValuePropertyCallback(const JSExportNamedValuePropertyCallback&) JAVASCRIPTCORECPP_NOEXCEPT;
    JSExportNamedValuePropertyCallback(JSExportNamedValuePropertyCallback&&) JAVASCRIPTCORECPP_NOEXCEPT;
    JSExportNamedValuePropertyCallback& operator=(const JSExportNamedValuePropertyCallback&) JAVASCRIPTCORECPP_NOEXCEPT;
    JSExportNamedValuePropertyCallback& operator=(JSExportNamedValuePropertyCallback&&) JAVASCRIPTCORECPP_NOEXCEPT;
    void swap(JSExportNamedValuePropertyCallback&) JAVASCRIPTCORECPP_NOEXCEPT;
    
  private:
    
    template<typename U>
    friend bool operator==(const JSExportNamedValuePropertyCallback<U>& lhs, const JSExportNamedValuePropertyCallback<U>& rhs) JAVASCRIPTCORECPP_NOEXCEPT;
    
    GetNamedValuePropertyCallback<T> get_callback__ { nullptr };
    SetNamedValuePropertyCallback<T> set_callback__ { nullptr };
  };
  
  template<typename T>
  JSExportNamedValuePropertyCallback<T>::JSExportNamedValuePropertyCallback(
                                                                            const std::string& property_name,
                                                                            GetNamedValuePropertyCallback<T> get_callback,
                                                                            SetNamedValuePropertyCallback<T> set_callback,
                                                                            const std::unordered_set<JSPropertyAttribute>& attributes)
  : JSPropertyCallback(property_name, attributes)
  , get_callback__(get_callback)
  , set_callback__(set_callback) {
    
    if (!get_callback && !set_callback) {
      ThrowInvalidArgument("JSExportNamedValuePropertyCallback", "Both get_callback and set_callback are missing. At least one callback must be provided");
    }
    
    if (attributes.find(JSPropertyAttribute::ReadOnly) != attributes.end()) {
      if (!get_callback) {
        ThrowInvalidArgument("JSExportNamedValuePropertyCallback", "ReadOnly attribute is set but get_callback is missing");
      }
      
      if (set_callback) {
        ThrowInvalidArgument("JSExportNamedValuePropertyCallback", "ReadOnly attribute is set but set_callback is provided");
      }
    }
    
    // Force the ReadOnly attribute if only the get_callback is
    // provided.
    if (get_callback && !set_callback) {
      attributes__.insert(JSPropertyAttribute::ReadOnly);
    }
  }
  
  template<typename T>
  JSExportNamedValuePropertyCallback<T>::JSExportNamedValuePropertyCallback(const JSExportNamedValuePropertyCallback& rhs) JAVASCRIPTCORECPP_NOEXCEPT
  : JSPropertyCallback(rhs)
  , get_callback__(rhs.get_callback__)
  , set_callback__(rhs.set_callback__) {
  }
  
  template<typename T>
  JSExportNamedValuePropertyCallback<T>::JSExportNamedValuePropertyCallback(JSExportNamedValuePropertyCallback&& rhs) JAVASCRIPTCORECPP_NOEXCEPT
  : JSPropertyCallback(rhs)
  , get_callback__(std::move(rhs.get_callback__))
  , set_callback__(std::move(rhs.set_callback__)) {
  }
  
  template<typename T>
  JSExportNamedValuePropertyCallback<T>& JSExportNamedValuePropertyCallback<T>::operator=(const JSExportNamedValuePropertyCallback<T>& rhs) JAVASCRIPTCORECPP_NOEXCEPT {
    JAVASCRIPTCORECPP_DETAIL_JSPROPERTYCALLBACK_LOCK_GUARD;
    JSPropertyCallback::operator=(rhs);
    get_callback__ = rhs.get_callback__;
    set_callback__ = rhs.set_callback__;
    return *this;
  }
  
  template<typename T>
  JSExportNamedValuePropertyCallback<T>& JSExportNamedValuePropertyCallback<T>::operator=(JSExportNamedValuePropertyCallback<T>&& rhs) JAVASCRIPTCORECPP_NOEXCEPT {
    JAVASCRIPTCORECPP_DETAIL_JSPROPERTYCALLBACK_LOCK_GUARD;
    swap(rhs);
    return *this;
  }
  
  template<typename T>
  void JSExportNamedValuePropertyCallback<T>::swap(JSExportNamedValuePropertyCallback<T>& other) JAVASCRIPTCORECPP_NOEXCEPT {
    JAVASCRIPTCORECPP_DETAIL_JSPROPERTYCALLBACK_LOCK_GUARD;
    JSPropertyCallback::swap(other);
    using std::swap;
    
    // By swapping the members of two classes, the two classes are
    // effectively swapped.
    swap(get_callback__, other.get_callback__);
    swap(set_callback__, other.set_callback__);
  }
  
  template<typename T>
  void swap(JSExportNamedValuePropertyCallback<T>& first, JSExportNamedValuePropertyCallback<T>& second) JAVASCRIPTCORECPP_NOEXCEPT {
    first.swap(second);
  }
  
  // Return true if the two JSExportNamedValuePropertyCallbacks are
  // equal.
  template<typename T>
  bool operator==(const JSExportNamedValuePropertyCallback<T>& lhs, const JSExportNamedValuePropertyCallback<T>& rhs) JAVASCRIPTCORECPP_NOEXCEPT {
    // get_callback__
    if (lhs.get_callback__ && !rhs.get_callback__) {
      return false;
    }
    
    if (!lhs.get_callback__ && rhs.get_callback__) {
      return false;
    }
    
    // set_callback__
    if (lhs.set_callback__ && !rhs.set_callback__) {
      return false;
    }
    
    if (!lhs.set_callback__ && rhs.set_callback__) {
      return false;
    }
    
    return static_cast<JSPropertyCallback>(lhs) == static_cast<JSPropertyCallback>(rhs);
  }
  
  // Return true if the two JSExportNamedValuePropertyCallback are
  // not equal.
  template<typename T>
  bool operator!=(const JSExportNamedValuePropertyCallback<T>& lhs, const JSExportNamedValuePropertyCallback<T>& rhs) JAVASCRIPTCORECPP_NOEXCEPT {
    return ! (lhs == rhs);
  }
  
}} // namespace JavaScriptCoreCPP { namespace detail {

#endif // _JAVASCRIPTCORECPP_DETAIL_JSEXPORTNAMEDVALUEPROPERTYCALLBACK_HPP_
