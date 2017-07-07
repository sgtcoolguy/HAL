/**
 * HAL
 *
 * Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#include "HAL/JSString.hpp"

#include <cassert>

namespace HAL {
  
  JSString::JSString() HAL_NOEXCEPT
  : JSString("") {
    //HAL_LOG_TRACE("JSString::JSString()");
  }
  
  JSString::JSString(const char* string) HAL_NOEXCEPT {
    if (string) {
      js_string_ref__ = JSStringCreateWithUTF8CString(string);
      string__ = string;
    } else {
      js_string_ref__ = JSStringCreateWithUTF8CString("");
      string__ = "";
    }
    
    HAL_LOG_TRACE("JSString:: ctor 1 ", this);
    HAL_LOG_TRACE("JSString:: retain ", js_string_ref__, " (implicit) for ", this);
    
    std::hash<std::string> hash_function = std::hash<std::string>();
    hash_value__ = hash_function(static_cast<std::string>(string__));
    //HAL_LOG_TRACE("JSString::JSString(const char*)");
  }
  
  JSString::JSString(const std::string& string) HAL_NOEXCEPT
  : js_string_ref__(JSStringCreateWithUTF8CString(string.c_str()))
  , string__(string) {
    HAL_LOG_TRACE("JSString:: ctor 2 ", this);
    HAL_LOG_TRACE("JSString:: retain ", js_string_ref__, " (implicit) for ", this);

    std::hash<std::string> hash_function = std::hash<std::string>();
    hash_value__ = hash_function(static_cast<std::string>(string__));
    //HAL_LOG_TRACE("JSString::JSString(const std::string&)");
  }
  
  const std::size_t JSString::length() const  HAL_NOEXCEPT{
    HAL_JSSTRING_LOCK_GUARD;
    return JSStringGetLength(js_string_ref__);
  }
  
  const std::size_t JSString::size() const HAL_NOEXCEPT {
    return length();
  }
  
  const bool JSString::empty() const HAL_NOEXCEPT {
    return length() == 0;
  }
  
  JSString::operator std::string() const HAL_NOEXCEPT {
    return string__;
  }
  
  std::size_t JSString::hash_value() const {
    return hash_value__;
  }
  
  JSString::~JSString() HAL_NOEXCEPT {
    HAL_LOG_TRACE("JSString:: dtor ", this);
    HAL_LOG_TRACE("JSString:: release ", js_string_ref__, " for ", this);
    JSStringRelease(js_string_ref__);
  }
  
  JSString::JSString(const JSString& rhs) HAL_NOEXCEPT
  : js_string_ref__(rhs.js_string_ref__)
  , string__(rhs.string__)
  , hash_value__(rhs.hash_value__) {
    HAL_LOG_TRACE("JSString:: copy ctor ", this);
    HAL_LOG_TRACE("JSString:: retain ", js_string_ref__, " for ", this);
    JSStringRetain(js_string_ref__);
  }
  
  JSString::JSString(JSString&& rhs) HAL_NOEXCEPT
  : js_string_ref__(rhs.js_string_ref__)
  , string__(std::move(rhs.string__))
  , hash_value__(std::move(rhs.hash_value__)) {
    HAL_LOG_TRACE("JSString:: move ctor ", this);
    HAL_LOG_TRACE("JSString:: retain ", js_string_ref__, " for ", this);
    JSStringRetain(js_string_ref__);
  }
  
  JSString& JSString::operator=(JSString rhs) HAL_NOEXCEPT {
    HAL_JSSTRING_LOCK_GUARD;
    HAL_LOG_TRACE("JSString:: assignment ", this);
    swap(rhs);
    return *this;
  }
  
  void JSString::swap(JSString& other) HAL_NOEXCEPT {
    HAL_JSSTRING_LOCK_GUARD;
    HAL_LOG_TRACE("JSString:: swap ", this);
    using std::swap;
    
    // By swapping the members of two classes, the two classes are
    // effectively swapped.
    swap(js_string_ref__, other.js_string_ref__);
    swap(string__       , other.string__);
    swap(hash_value__   , other.hash_value__);
  }
  
  // For interoperability with the JavaScriptCore C API.
  JSString::JSString(JSStringRef js_string_ref) HAL_NOEXCEPT
  : js_string_ref__(js_string_ref) {
    assert(js_string_ref__);
    JSStringRetain(js_string_ref__);
    HAL_LOG_TRACE("JSString:: ctor 3 ", this);
    HAL_LOG_TRACE("JSString:: retain ", js_string_ref__, " for ", this);

    const auto size = JSStringGetMaximumUTF8CStringSize(js_string_ref__);
    auto buffer = new char[size];
    JSStringGetUTF8CString(js_string_ref__, buffer, size);
    string__ = std::string(buffer);
    delete[] buffer;
    
    std::hash<std::string> hash_function = std::hash<std::string>();
    hash_value__ = hash_function(static_cast<std::string>(string__));
  }
  
  bool operator==(const JSString& lhs, const JSString& rhs) {
    return JSStringIsEqual(static_cast<JSStringRef>(lhs), static_cast<JSStringRef>(rhs));
  }
  
} // namespace HAL {
