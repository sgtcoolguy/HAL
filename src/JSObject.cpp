/**
 * HAL
 *
 * Copyright (c) 2018 by Axway. All Rights Reserved.
 * Licensed under the terms of the Apache Public License.
 * Please see the LICENSE included with this distribution for details.
 */

#include "HAL/JSObject.hpp"
#include "HAL/JSValue.hpp"
#include "HAL/JSString.hpp"
#include "HAL/JSClass.hpp"
#include "HAL/JSError.hpp"
#include "HAL/JSArray.hpp"
#include "HAL/detail/JSUtil.hpp"
#include <codecvt>
#include <cassert>

namespace HAL {

	bool JSObject::HasProperty(const std::string& property_name) const HAL_NOEXCEPT {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		const auto name = converter.from_bytes(property_name);

		bool hasProperty;
		JsPropertyIdRef propertyId;
		ASSERT_AND_THROW_JS_ERROR(JsGetPropertyIdFromName(name.data(), &propertyId));
		ASSERT_AND_THROW_JS_ERROR(JsHasProperty(js_object_ref__, propertyId, &hasProperty));
		return hasProperty;
	}

	JSValue JSObject::GetProperty(const std::string& property_name) const {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		const auto name = converter.from_bytes(property_name);

		JsValueRef value;
		JsPropertyIdRef propertyId;
		ASSERT_AND_THROW_JS_ERROR(JsGetPropertyIdFromName(name.data(), &propertyId));
		ASSERT_AND_THROW_JS_ERROR(JsGetProperty(js_object_ref__, propertyId, &value));
		return value;
	}

	JSValue JSObject::GetProperty(unsigned property_index) const {
		JsValueRef js_value;
		JsValueRef numberValue;
		ASSERT_AND_THROW_JS_ERROR(JsIntToNumber(property_index, &numberValue));
		ASSERT_AND_THROW_JS_ERROR(JsGetIndexedProperty(js_object_ref__, numberValue, &js_value));
		return js_value;
	}

	void JSObject::SetProperty(const std::string& property_name, const JSValue& property_value) {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		const auto name = converter.from_bytes(property_name);

		JsPropertyIdRef propertyId;
		ASSERT_AND_THROW_JS_ERROR(JsGetPropertyIdFromName(name.data(), &propertyId));
		ASSERT_AND_THROW_JS_ERROR(JsSetProperty(js_object_ref__, propertyId, static_cast<JsValueRef>(property_value), true));

#ifndef NDEBUG
		assert(HasProperty(property_name));
		assert(GetProperty(property_name) == property_value);
#endif
	}

	void JSObject::SetProperty(unsigned property_index, const JSValue& property_value) {
		JsValueRef numberValue;
		ASSERT_AND_THROW_JS_ERROR(JsIntToNumber(property_index, &numberValue));
		ASSERT_AND_THROW_JS_ERROR(JsSetIndexedProperty(js_object_ref__, numberValue, static_cast<JsValueRef>(property_value)));
	}

	bool JSObject::DeleteProperty(const std::string& property_name) {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		const auto name = converter.from_bytes(property_name);

		JsValueRef result;
		JsPropertyIdRef propertyId;
		ASSERT_AND_THROW_JS_ERROR(JsGetPropertyIdFromName(name.data(), &propertyId));
		ASSERT_AND_THROW_JS_ERROR(JsDeleteProperty(js_object_ref__, propertyId, true, &result));
		return static_cast<bool>(JSValue(result));
	}

	JSArray JSObject::GetPropertyNames() const HAL_NOEXCEPT {
		JsValueRef names;
		ASSERT_AND_THROW_JS_ERROR(JsGetOwnPropertyNames(js_object_ref__, &names));
		return JSArray(names);
	}

	std::unordered_map<std::string, JSValue> JSObject::GetProperties() const HAL_NOEXCEPT {
		std::unordered_map<std::string, JSValue> properties;
		for (const auto& property_name : static_cast<std::vector<std::string>>(GetPropertyNames())) {
			properties.emplace(property_name, GetProperty(property_name));
		}
		return properties;
	}

	bool JSObject::IsFunction() const HAL_NOEXCEPT {
		JsValueType valueType;
		ASSERT_AND_THROW_JS_ERROR(JsGetValueType(js_object_ref__, &valueType));
		return (valueType == JsValueType::JsFunction);
	}

	bool JSObject::IsArray() const HAL_NOEXCEPT {
		JsValueType valueType;
		ASSERT_AND_THROW_JS_ERROR(JsGetValueType(js_object_ref__, &valueType));
		return (valueType == JsValueType::JsArray);
	}

	bool JSObject::IsError() const HAL_NOEXCEPT {
		JsValueType valueType;
		ASSERT_AND_THROW_JS_ERROR(JsGetValueType(js_object_ref__, &valueType));
		return (valueType == JsValueType::JsError);
	}

	bool JSObject::IsConstructor() const HAL_NOEXCEPT {
		JsValueType valueType;
		ASSERT_AND_THROW_JS_ERROR(JsGetValueType(js_object_ref__, &valueType));
		return (valueType == JsValueType::JsFunction);
	}

	JSValue JSObject::operator()(JSObject this_object) { return CallAsFunction(std::vector<JSValue>(), this_object); }
	JSValue JSObject::operator()(JSValue&                     argument, JSObject this_object) { return CallAsFunction({ argument }, this_object); }
	JSValue JSObject::operator()(const std::vector<JSValue>&  arguments, JSObject this_object) { return CallAsFunction(arguments, this_object); }

	JSObject JSObject::CallAsConstructor() { return CallAsConstructor(std::vector<JSValue>  {}); }
	JSObject JSObject::CallAsConstructor(const JSValue&               argument) { return CallAsConstructor(std::vector<JSValue>  {argument}); }
	JSObject JSObject::CallAsConstructor(const std::vector<JSValue>&  arguments) {
		JsValueRef result;

		auto js_args = detail::to_arguments(arguments, js_object_ref__);
		ASSERT_AND_THROW_JS_ERROR(JsConstructObject(js_object_ref__, &js_args[0], static_cast<unsigned short>(js_args.size()), &result));

		return JSObject(result);
	}

	void* JSObject::GetPrivate() const HAL_NOEXCEPT {
		void* data = nullptr;
		bool hasData = false;
		ASSERT_AND_THROW_JS_ERROR(JsHasExternalData(js_object_ref__, &hasData));
		if (hasData) {
			ASSERT_AND_THROW_JS_ERROR(JsGetExternalData(js_object_ref__, &data));
		}
		return data;
	}

	bool JSObject::SetPrivate(void* data) const HAL_NOEXCEPT {
		return (JsSetExternalData(js_object_ref__, data) == JsErrorCode::JsNoError);
	}

	JSObject::~JSObject() HAL_NOEXCEPT {
	}

	JSObject::JSObject(const JSObject& rhs) HAL_NOEXCEPT
		: js_object_ref__(rhs.js_object_ref__) {
	}

	JSObject::JSObject(JSObject&& rhs) HAL_NOEXCEPT
		: js_object_ref__(rhs.js_object_ref__) {
	}

	JSObject& JSObject::operator=(JSObject rhs) {
		swap(rhs);
		return *this;
	}

	void JSObject::swap(JSObject& other) HAL_NOEXCEPT {
		using std::swap;

		// By swapping the members of two classes, the two classes are
		// effectively swapped.
		swap(js_object_ref__, other.js_object_ref__);
	}

	std::unordered_map<JsValueRef, const std::uintptr_t> JSObject::js_object_ref_to_js_private_data_map__;
	std::unordered_map<std::uintptr_t, const JsValueRef> JSObject::js_private_data_to_js_object_ref_map__;
	std::unordered_map<std::uintptr_t, JSExportConstructObjectCallback> JSObject::js_ctor_ref_to_constructor_map__;

	JSObject JSObject::GetObject(const JSExportObject* js_export_ptr) {
		const auto key = reinterpret_cast<std::uintptr_t>(js_export_ptr);
		const auto position = js_private_data_to_js_object_ref_map__.find(key);
		const auto found = position != js_private_data_to_js_object_ref_map__.end();
		assert(found);
		if (found) {
			return JSObject(position->second);
		}
		return GetUndefinedRef();
	}

	bool JSObject::IsJSExportObjectRegistered(const JsValueRef js_value_ref) {
		const auto position = js_object_ref_to_js_private_data_map__.find(js_value_ref);
		return (position != js_object_ref_to_js_private_data_map__.end());
	}

	void JSObject::RegisterGlobalObject(const JSExportObject* js_export_ptr, const JsValueRef js_value_ref) {
		const auto key = reinterpret_cast<std::uintptr_t>(js_export_ptr);
		const auto position = js_object_ref_to_js_private_data_map__.find(js_value_ref);
		const auto found = position != js_object_ref_to_js_private_data_map__.end();
		assert(!found);
		js_object_ref_to_js_private_data_map__.emplace(js_value_ref, key);
	}

	void JSObject::UnregisterGlobalObject(const JsValueRef js_value_ref) {
		const auto position = js_object_ref_to_js_private_data_map__.find(js_value_ref);
		const auto found = position != js_object_ref_to_js_private_data_map__.end();
		assert(found);
		js_object_ref_to_js_private_data_map__.erase(js_value_ref);
	}

	void JSObject::RegisterJSExportObject(const JSExportObject* js_export_ptr, const JsValueRef js_value_ref) {
		const auto key = reinterpret_cast<std::uintptr_t>(js_export_ptr);
		const auto position = js_private_data_to_js_object_ref_map__.find(key);
		const auto found = position != js_private_data_to_js_object_ref_map__.end();
		assert(!found);
		js_private_data_to_js_object_ref_map__.emplace(key, js_value_ref);
		js_object_ref_to_js_private_data_map__.emplace(js_value_ref, key);
	}

	void JSObject::UnregisterJSExportObject(const JSExportObject* js_export_ptr) {
		const auto key = reinterpret_cast<std::uintptr_t>(js_export_ptr);
		const auto position = js_private_data_to_js_object_ref_map__.find(key);
		const auto found = position != js_private_data_to_js_object_ref_map__.end();
		assert(found);
		const auto js_value_ref = position->second;
		js_private_data_to_js_object_ref_map__.erase(key);
		js_object_ref_to_js_private_data_map__.erase(js_value_ref);
	}

	JSExportConstructObjectCallback JSObject::GetObjectInitializerCallback(const JsValueRef js_ctor_ref) {
		const auto key = reinterpret_cast<std::uintptr_t>(js_ctor_ref);
		const auto position = js_ctor_ref_to_constructor_map__.find(key);
		const auto found = position != js_ctor_ref_to_constructor_map__.end();
		assert(found);
		if (found) {
			return position->second;
		}
		return nullptr;
	}

	void JSObject::RemoveObjectConstructorCallback(const JsValueRef js_ctor_ref) {
		const auto key = reinterpret_cast<std::uintptr_t>(js_ctor_ref);
		const auto position = js_ctor_ref_to_constructor_map__.find(key);
		const auto found = position != js_ctor_ref_to_constructor_map__.end();
		assert(found);
		if (found) {
			js_ctor_ref_to_constructor_map__.erase(key);
		}
	}

	JsValueRef CALLBACK JSExportCallConstructor(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState) {
		if (isConstructCall) {
			const auto callback = JSObject::GetObjectInitializerCallback(callee);
			if (callback) {
				JsValueRef js_object_ref = nullptr;
				callback(callee, isConstructCall, arguments, argumentCount, &js_object_ref);
				assert(js_object_ref);
				return js_object_ref;
			}
		}

		// This means JSExportObject is called as a function.
		return JSObject::GetUndefinedRef();
	}

	JSObject::JSObject(const JSClass& js_class) {
		// Create constructor
		ASSERT_AND_THROW_JS_ERROR(JsCreateFunction(JSExportCallConstructor, nullptr, &js_object_ref__));
		const auto ctor_init = js_class.GetInitializeConstructorCallback();
		assert(ctor_init != nullptr);
		ctor_init(&js_object_ref__);

		const auto key = reinterpret_cast<std::uintptr_t>(js_object_ref__);
		const auto position = js_ctor_ref_to_constructor_map__.find(key);
		if (position != js_ctor_ref_to_constructor_map__.end()) {
			js_ctor_ref_to_constructor_map__.erase(key);
		}
		js_ctor_ref_to_constructor_map__.emplace(key, js_class.GetConstructObjectCallback());
	}

	// For interoperability with the JSRT API.
	JSObject::JSObject(JsValueRef js_object_ref)
		: js_object_ref__(js_object_ref) {
	}

	JSObject::operator JSValue() const {
		return JSValue(js_object_ref__);
	}

	JSObject::operator JSArray() const {
		return JSArray(js_object_ref__);
	}

	JSObject::operator JSError() const {
		return JSError(js_object_ref__);
	}

	JSValue JSObject::CallAsFunction(const std::vector<JSValue>&  arguments, JSObject this_object) {
		const auto this_object_ref = static_cast<JsValueRef>(this_object);
		JsValueRef result;
		if (arguments.empty()) {
			JsValueRef js_args[] = { this_object_ref };
			ASSERT_AND_THROW_JS_ERROR(JsCallFunction(js_object_ref__, js_args, 1, &result));
		} else {
			auto js_args = detail::to_arguments(arguments, this_object_ref);
			ASSERT_AND_THROW_JS_ERROR(JsCallFunction(js_object_ref__, &js_args[0], static_cast<unsigned short>(js_args.size()), &result));
		}
		return result;
	}

} // namespace HAL {
