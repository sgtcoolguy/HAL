/**
* HAL
*
* Copyright (c) 2018 by Axway. All Rights Reserved.
* Licensed under the terms of the Apache Public License.
* Please see the LICENSE included with this distribution for details.
*/

#ifndef _HAL_JSCLASS_HPP_
#define _HAL_JSCLASS_HPP_

#include "HAL/detail/JSBase.hpp"
#include "HAL/detail/JSUtil.hpp"
#include <functional>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <cctype>
#include <codecvt>

namespace HAL {

	class JSContext;
	class JSValue;
	class JSObject;

	typedef std::function<JSValue(JSObject, JSObject, const std::vector<JSValue>&)> JSObjectCallAsFunctionCallback;
	typedef std::function<JSValue(JSObject)> JSObjectGetPropertyCallback;
	typedef std::function<bool(JSObject, const JSValue& value)> JSObjectSetPropertyCallback;
	typedef std::function<void(JsValueRef, bool, JsValueRef*, unsigned short, JsValueRef*)> JSExportConstructObjectCallback;
	typedef std::function<void(JsValueRef*)> JSExportInitializeConstructorCallback;
	typedef std::function<void(JsValueRef*)> JSExportInitializePropertiesCallback;

	class HAL_EXPORT JSClass {
	public:
		JSClass() HAL_NOEXCEPT { };
		virtual ~JSClass()          HAL_NOEXCEPT { }
		JSClass(const JSClass& rhs) HAL_NOEXCEPT
			: parent_initialize_ctor_callback__(rhs.parent_initialize_ctor_callback__)
			, parent_initialize_properties_callback__(rhs.parent_initialize_properties_callback__) {
			overloaded_initialize_ctor_callback__ = rhs.GetInitializeConstructorCallback();
			overloaded_construct_object_callback__ = rhs.GetConstructObjectCallback();
			overloaded_initialize_properties_callback__ = rhs.GetInitializePropertiesCallback();
		}
		JSClass(JSClass&& rhs) HAL_NOEXCEPT 
			: parent_initialize_ctor_callback__(rhs.parent_initialize_ctor_callback__)
			, parent_initialize_properties_callback__(rhs.parent_initialize_properties_callback__)  {
			overloaded_initialize_ctor_callback__ = rhs.GetInitializeConstructorCallback();
			overloaded_construct_object_callback__ = rhs.GetConstructObjectCallback();
			overloaded_initialize_properties_callback__ = rhs.GetInitializePropertiesCallback();
		}
		JSClass& operator=(JSClass rhs) HAL_NOEXCEPT {
			std::swap(parent_initialize_ctor_callback__, rhs.parent_initialize_ctor_callback__);
			std::swap(parent_initialize_properties_callback__, rhs.parent_initialize_properties_callback__);
			overloaded_initialize_ctor_callback__ = rhs.GetInitializeConstructorCallback();
			overloaded_construct_object_callback__ = rhs.GetConstructObjectCallback();
			overloaded_initialize_properties_callback__ = rhs.GetInitializePropertiesCallback();
			return *this; 
		}

		virtual void AddFunctionProperty(const std::string& name, JSObjectCallAsFunctionCallback callback) { assert(false); }
		virtual void AddValueProperty(const std::string& name, JSObjectGetPropertyCallback, JSObjectSetPropertyCallback) { assert(false); };
		virtual void AddConstantProperty(const std::string& name, JSObjectGetPropertyCallback) { assert(false); };
		virtual void SetParent(const JSClass& js_class) { assert(false); }

		virtual JSExportInitializeConstructorCallback GetInitializeConstructorCallback() const { 
			if (overloaded_initialize_ctor_callback__) {
				return overloaded_initialize_ctor_callback__;
			}
			return [](JsValueRef*) {}; 
		}
		virtual JSExportConstructObjectCallback GetConstructObjectCallback() const {
			if (overloaded_construct_object_callback__) {
				return overloaded_construct_object_callback__;
			}
			return [](JsValueRef, bool, JsValueRef*, unsigned short, JsValueRef*) {}; 
		}
		virtual JSExportInitializePropertiesCallback GetInitializePropertiesCallback() const {
			if (overloaded_initialize_properties_callback__) {
				return overloaded_initialize_properties_callback__;
			}
			return [](JsValueRef*) {}; 
		}
	protected:

		// Prevent heap based objects.
		void* operator new(std::size_t) = delete;   // #1: To prevent allocation of scalar objects
		void* operator new[](std::size_t) = delete; // #2: To prevent allocation of array of objects

#pragma warning(push)
#pragma warning(disable: 4251)
		JSExportInitializeConstructorCallback parent_initialize_ctor_callback__{ nullptr };
		JSExportInitializePropertiesCallback parent_initialize_properties_callback__{ nullptr };

		// Only used for copy constructor and assignment operator
		JSExportInitializeConstructorCallback overloaded_initialize_ctor_callback__{ nullptr };
		JSExportConstructObjectCallback overloaded_construct_object_callback__{ nullptr };
		JSExportInitializePropertiesCallback overloaded_initialize_properties_callback__{ nullptr };
#pragma warning(pop)

	};

	template<typename T>
	class JSExportClass final : public JSClass {
	public:
		JSExportClass() HAL_NOEXCEPT;
		virtual ~JSExportClass() HAL_NOEXCEPT;
		JSExportClass& operator=(const JSExportClass&) = default;
		JSExportClass(const JSExportClass&) = default;
		JSExportClass(JSExportClass&&) = default;

		virtual void AddValueProperty(const std::string& name, JSObjectGetPropertyCallback, JSObjectSetPropertyCallback) override;
		virtual void AddConstantProperty(const std::string& name, JSObjectGetPropertyCallback) override;
		virtual void AddFunctionProperty(const std::string& name, JSObjectCallAsFunctionCallback callback) override;
		virtual void SetParent(const JSClass& js_class) override;
		virtual JSExportInitializeConstructorCallback GetInitializeConstructorCallback() const override;
		virtual JSExportConstructObjectCallback GetConstructObjectCallback() const override;
		virtual JSExportInitializePropertiesCallback GetInitializePropertiesCallback() const override;
	protected:
		// Prevent heap based objects.
		void* operator new(std::size_t) = delete;   // #1: To prevent allocation of scalar objects
		void* operator new[](std::size_t) = delete; // #2: To prevent allocation of array of objects

#pragma warning(push)
#pragma warning(disable: 4251)
		static std::unordered_map<std::string, JSObjectCallAsFunctionCallback> name_to_function_map__;
		static std::unordered_map<std::string, JSObjectGetPropertyCallback> name_to_getter_map__;
		static std::unordered_map<std::string, JSObjectSetPropertyCallback> name_to_setter_map__;
#pragma warning(pop)
	};

	template<typename T>
	JSExportClass<T>::JSExportClass() HAL_NOEXCEPT {

	}

	template<typename T>
	JSExportClass<T>::~JSExportClass() HAL_NOEXCEPT {
	}

	struct NamedFunctionCallbackState {
		std::string name;
		JSObjectCallAsFunctionCallback callback;
	};

	template<typename T>
	JsValueRef CALLBACK JSExportCreateNamedFunction(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState) {
		const auto state = static_cast<NamedFunctionCallbackState*>(callbackState);
		assert(state != nullptr);

		const auto js_arguments = detail::to_arguments(arguments, argumentCount);
		auto this_object = JSObject(arguments[0]);
		auto function_object = JSObject(callee);
		if (static_cast<JSValue>(this_object).IsUndefined()) {
			this_object = function_object.get_context().get_global_object();
		}

		return static_cast<JsValueRef>(state->callback(function_object, this_object, js_arguments));
	}

	template<typename T>
	void CALLBACK JSExportNamedFunctionBeforeCollect(JsRef ref, void* callbackState) {
		const auto state = static_cast<NamedFunctionCallbackState*>(callbackState);

		assert(!state->name.empty());
		state->name.clear();

		assert(state->callback != nullptr);
		state->callback = nullptr;

		delete state;
	}

	template<typename T>
	void CALLBACK JSExportConstructorBeforeCollect(JsRef ref, void* callbackState) {
		JSObject::RemoveObjectConstructorCallback(static_cast<JsValueRef>(ref));
		if (callbackState != nullptr) {
			const auto js_export_object_ptr = static_cast<T*>(callbackState);
			JSObject::UnregisterJSExportObject(js_export_object_ptr);
			delete js_export_object_ptr;
		}
	}

	template<typename T>
	JSExportInitializeConstructorCallback JSExportClass<T>::GetInitializeConstructorCallback() const {
		const auto parent_initialize_ctor_callback = parent_initialize_ctor_callback__;
		const auto initialize_properties_callback = GetInitializePropertiesCallback();
		const auto name_to_function_map = name_to_function_map__;
		const auto name_to_getter_map = name_to_getter_map__;
		const auto name_to_setter_map = name_to_setter_map__;
		return [=](JsValueRef* ctor_object_ref) {

			JSContext js_context = JSContext(JSObject::GetContextRef());
			JSObject ctor_object = JSObject(*ctor_object_ref);

			if (!JSObject::IsJSExportObjectRegistered(*ctor_object_ref)) {
				const auto js_export_object_ptr = new T(js_context);
				JSObject::RegisterJSExportObject(js_export_object_ptr, *ctor_object_ref);
				ASSERT_AND_THROW_JS_ERROR(JsSetObjectBeforeCollectCallback(*ctor_object_ref, js_export_object_ptr, JSExportConstructorBeforeCollect<T>));
			} else {
				ASSERT_AND_THROW_JS_ERROR(JsSetObjectBeforeCollectCallback(*ctor_object_ref, nullptr, JSExportConstructorBeforeCollect<T>));
			}

			if (parent_initialize_ctor_callback != nullptr) {
				parent_initialize_ctor_callback(ctor_object_ref);
			}

			JSObject js_prototype = ctor_object.HasProperty("prototype") ? static_cast<JSObject>(ctor_object.GetProperty("prototype")) : js_context.CreateObject();

			for (const auto pair : name_to_function_map) {
				JsValueRef js_function_ref;

				const auto callbackState = new NamedFunctionCallbackState();
				callbackState->name = pair.first;
				callbackState->callback = pair.second;

				const auto js_name = static_cast<JsValueRef>(JSString(pair.first));
				ASSERT_AND_THROW_JS_ERROR(JsCreateNamedFunction(js_name, JSExportCreateNamedFunction<T>, callbackState, &js_function_ref));
				js_prototype.SetProperty(pair.first, JSValue(js_function_ref));
				ctor_object.SetProperty(pair.first, JSValue(js_function_ref));

				ASSERT_AND_THROW_JS_ERROR(JsSetObjectBeforeCollectCallback(js_function_ref, callbackState, JSExportNamedFunctionBeforeCollect<T>));
			}

			for (const auto pair : name_to_getter_map) {
				assert(!pair.first.empty());

				// get + capitalized property name
				std::string getter_name = "get" + pair.first;
				getter_name[3] = toupper(getter_name[3]);

				// define getter function
				JsValueRef js_function_ref;

				const auto callbackState = new NamedFunctionCallbackState();
				callbackState->name = getter_name;
				callbackState->callback = [pair](JSObject, JSObject this_object, const std::vector<JSValue>&) {
					return pair.second(this_object);
				};

				const auto js_name = static_cast<JsValueRef>(JSString(getter_name));
				ASSERT_AND_THROW_JS_ERROR(JsCreateNamedFunction(js_name, JSExportCreateNamedFunction<T>, callbackState, &js_function_ref));
				js_prototype.SetProperty(getter_name, JSValue(js_function_ref));
				ctor_object.SetProperty(getter_name, JSValue(js_function_ref));

				ASSERT_AND_THROW_JS_ERROR(JsSetObjectBeforeCollectCallback(js_function_ref, callbackState, JSExportNamedFunctionBeforeCollect<T>));
			}

			for (const auto pair : name_to_setter_map) {
				assert(!pair.first.empty());

				// set + capitalized property name
				std::string setter_name = "set" + pair.first;
				setter_name[3] = toupper(setter_name[3]);

				// define setter function
				JsValueRef js_function_ref;

				const auto callbackState = new NamedFunctionCallbackState();
				callbackState->name = setter_name;
				callbackState->callback = [pair, js_context](JSObject, JSObject this_object, const std::vector<JSValue>& js_arguments) {
					if (js_arguments.size() > 0) {
						pair.second(this_object, js_arguments.at(0));
					}
					return js_context.CreateUndefined();
				};

				const auto js_name = static_cast<JsValueRef>(JSString(setter_name));
				ASSERT_AND_THROW_JS_ERROR(JsCreateNamedFunction(js_name, JSExportCreateNamedFunction<T>, callbackState, &js_function_ref));
				js_prototype.SetProperty(setter_name, JSValue(js_function_ref));
				ctor_object.SetProperty(setter_name, JSValue(js_function_ref));

				ASSERT_AND_THROW_JS_ERROR(JsSetObjectBeforeCollectCallback(js_function_ref, callbackState, JSExportNamedFunctionBeforeCollect<T>));
			}

			initialize_properties_callback(ctor_object_ref);

			ctor_object.SetProperty("prototype", js_prototype);
		};
	}

	template<typename T>
	void CALLBACK JSExportFinalize(void *data) {
		const auto js_export_object_ptr = static_cast<T*>(data);
		JSObject::UnregisterJSExportObject(js_export_object_ptr);
		delete js_export_object_ptr;
	}

	template<typename T>
	JSExportInitializePropertiesCallback JSExportClass<T>::GetInitializePropertiesCallback() const {
		const auto parent_initialize_properties_callback = parent_initialize_properties_callback__;
		const auto name_to_getter_map = name_to_getter_map__;
		const auto name_to_setter_map = name_to_setter_map__;
		return [=](JsValueRef* this_object_ref) {
			if (parent_initialize_properties_callback) {
				parent_initialize_properties_callback(this_object_ref);
			}

			JSContext js_context = JSContext(JSObject::GetContextRef());
			JSObject this_object = JSObject(*this_object_ref);

			// properties
			for (const auto pair : name_to_getter_map) {
				assert(!pair.first.empty());

				const auto property_name = pair.first;

				// get + capitalized property name
				std::string getter_name = "get" + pair.first;
				getter_name[3] = toupper(getter_name[3]);

				const auto setter_position = name_to_setter_map__.find(property_name);
				const auto setter_found = setter_position != name_to_setter_map__.end();

				auto property_descriptor = js_context.CreateObject();

				property_descriptor.SetProperty("get", this_object.GetProperty(getter_name));

				if (setter_found) {
					std::string setter_name = "set" + property_name;
					setter_name[3] = toupper(setter_name[3]);
					property_descriptor.SetProperty("set", this_object.GetProperty(setter_name));
				}

				std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
				const auto wstr_name = converter.from_bytes(property_name);

				JsPropertyIdRef propertyId;
				ASSERT_AND_THROW_JS_ERROR(JsGetPropertyIdFromName(wstr_name.data(), &propertyId));
				bool is_defined;
				ASSERT_AND_THROW_JS_ERROR(JsDefineProperty(*this_object_ref, propertyId, static_cast<JsValueRef>(property_descriptor), &is_defined));
				assert(is_defined == true);
			}
		};
	}

	template<typename T>
	JSExportConstructObjectCallback JSExportClass<T>::GetConstructObjectCallback() const {
		const auto initialize_properties_callback = GetInitializePropertiesCallback();
		const auto name_to_getter_map = name_to_getter_map__;
		const auto name_to_setter_map = name_to_setter_map__;
		return [=](JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, JsValueRef* this_object_ref) {
			assert(isConstructCall);

			JsValueRef js_context_ref;
			ASSERT_AND_THROW_JS_ERROR(JsGetCurrentContext(&js_context_ref));

			const auto js_context = JSContext(js_context_ref);
			auto js_export_object_ptr = new T(js_context);
			ASSERT_AND_THROW_JS_ERROR(JsCreateExternalObject(js_export_object_ptr, JSExportFinalize<T>, this_object_ref));

			JSObject::RegisterJSExportObject(js_export_object_ptr, *this_object_ref);

			const auto js_arguments = detail::to_arguments(arguments, argumentCount);
			auto this_object = JSObject(*this_object_ref);

			const auto js_prototype = JSObject(callee).GetProperty("prototype");
			if (js_prototype.IsObject()) {

				// functions
				const auto prototype_obj = static_cast<JSObject>(js_prototype);
				const auto properties = prototype_obj.GetProperties();
				for (const auto pair : properties) {
					this_object.SetProperty(pair.first, pair.second);
				}

				// properties
				initialize_properties_callback(this_object_ref);
			}

			js_export_object_ptr->postInitialize(this_object);
			js_export_object_ptr->postCallAsConstructor(js_context, js_arguments);
		};
	}

	template<typename T>
	std::unordered_map<std::string, JSObjectCallAsFunctionCallback> JSExportClass<T>::name_to_function_map__;

	template<typename T>
	std::unordered_map<std::string, JSObjectGetPropertyCallback> JSExportClass<T>::name_to_getter_map__;

	template<typename T>
	std::unordered_map<std::string, JSObjectSetPropertyCallback> JSExportClass<T>::name_to_setter_map__;

	template<typename T>
	void JSExportClass<T>::AddFunctionProperty(const std::string& name, JSObjectCallAsFunctionCallback callback) {
		const auto position = name_to_function_map__.find(name);
		const auto found = position != name_to_function_map__.end();
		assert(!found);
		name_to_function_map__.emplace(name, callback);
	};

	template<typename T>
	void JSExportClass<T>::AddValueProperty(const std::string& name, JSObjectGetPropertyCallback getter, JSObjectSetPropertyCallback setter) {
		{
			const auto getter_position = name_to_getter_map__.find(name);
			const auto getter_found = getter_position != name_to_getter_map__.end();
			assert(!getter_found);
			name_to_getter_map__.emplace(name, getter);
		}
		{
			const auto setter_position = name_to_setter_map__.find(name);
			const auto setter_found = setter_position != name_to_setter_map__.end();
			assert(!setter_found);
			name_to_setter_map__.emplace(name, setter);
		}
	};

	template<typename T>
	void JSExportClass<T>::AddConstantProperty(const std::string& name, JSObjectGetPropertyCallback getter) {
		const auto getter_position = name_to_getter_map__.find(name);
		const auto getter_found = getter_position != name_to_getter_map__.end();
		assert(!getter_found);
		name_to_getter_map__.emplace(name, getter);
	};

	template<typename T>
	void JSExportClass<T>::SetParent(const JSClass& js_class) {
		parent_initialize_ctor_callback__ = js_class.GetInitializeConstructorCallback();
		parent_initialize_properties_callback__ = js_class.GetInitializePropertiesCallback();
	};

} // namespace HAL {

#endif // _HAL_JSCLASS_HPP_
