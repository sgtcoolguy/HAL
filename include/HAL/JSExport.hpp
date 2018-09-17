/**
* HAL
*
* Copyright (c) 2018 by Axway. All Rights Reserved.
* Licensed under the terms of the Apache Public License.
* Please see the LICENSE included with this distribution for details.
*/

#ifndef _HAL_JSEXPORT_HPP_
#define _HAL_JSEXPORT_HPP_

#include "HAL/detail/JSBase.hpp"
#include "HAL/JSContext.hpp"
#include "HAL/JSValue.hpp"
#include "HAL/JSObject.hpp"
#include "HAL/JSClass.hpp"

namespace HAL {

	template<typename T>
	using GetNamedValuePropertyCallback = std::function<JSValue(T&)>;

	template<typename T>
	using SetNamedValuePropertyCallback = std::function<bool(T&, const JSValue&)>;

	template<typename T>
	using CallNamedFunctionCallback = std::function<JSValue(T&, const std::vector<JSValue>&, JSObject&)>;

	template<typename T>
	class JSExport {

	public:
		static JSExportClass<T> Class();
		virtual ~JSExport() HAL_NOEXCEPT {}

	protected:
		static JSExportClass<T> js_class__;
		static void AddFunctionProperty(const std::string& name, CallNamedFunctionCallback<T> callback);
		static void AddValueProperty(const std::string& name, GetNamedValuePropertyCallback<T> getter, SetNamedValuePropertyCallback<T> setter);
		static void AddConstantProperty(const std::string& name, GetNamedValuePropertyCallback<T> getter);
		static void SetParent(const JSClass& js_class);
		static void SetClassVersion(const std::uint32_t& class_version);
	};

	template<typename T>
	JSExportClass<T> JSExport<T>::js_class__;

	template<typename T>
	void JSExport<T>::AddFunctionProperty(const std::string& name, CallNamedFunctionCallback<T> callback) {
		js_class__.AddFunctionProperty(name, [callback](JSObject function_object, JSObject this_object, const std::vector<JSValue>& arguments) {
			auto t = this_object.GetPrivate<T>();
			if (t) {
				return callback(*t, arguments, this_object);
			}
			// This means this_object has no JSExportObject assigned,
			// probably because constructor prototype function is called directly.
			// In this case we just do nothing but return undefined.
			return this_object.get_context().CreateUndefined();
		});
	}

	template<typename T>
	void JSExport<T>::AddValueProperty(const std::string& name, GetNamedValuePropertyCallback<T> getter, SetNamedValuePropertyCallback<T> setter) {
		js_class__.AddValueProperty(name,
			[getter](JSObject this_object) {
			auto t = this_object.GetPrivate<T>();
			if (t && getter) {
				return getter(*t);
			}
			return this_object.get_context().CreateUndefined();
		},
			[setter](JSObject this_object, const JSValue& value) {
			auto t = this_object.GetPrivate<T>();
			if (t && setter) {
				return setter(*t, value);
			}
			return false;
		}
		);
	}

	template<typename T>
	void JSExport<T>::AddConstantProperty(const std::string& name, GetNamedValuePropertyCallback<T> getter) {
		js_class__.AddConstantProperty(name, [getter](JSObject this_object) {
			auto t = this_object.GetPrivate<T>();
			if (t && getter) {
				return getter(*t);
			}
			return this_object.get_context().CreateUndefined();
		});
	}

	template<typename T>
	JSExportClass<T> JSExport<T>::Class() {
		static std::once_flag of;
		std::call_once(of, []() {
			T::JSExportInitialize();
		});
		return js_class__;
	}

	template<typename T>
	void JSExport<T>::SetParent(const JSClass& js_class) {
		js_class__.SetParent(js_class);
	}

	template<typename T>
	void JSExport<T>::SetClassVersion(const std::uint32_t& class_version) {
		// Nothing to do for Chakra
	}

} // namespace HAL {

#endif // _HAL_JSEXPORT_HPP_
