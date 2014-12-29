#include "BsScriptGUIVector3Field.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUIVector3Field.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptGUIArea.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUIVector3Field::OnChangedThunkDef ScriptGUIVector3Field::onChangedThunk;

	ScriptGUIVector3Field::ScriptGUIVector3Field(MonoObject* instance, GUIVector3Field* vector3Field)
		:TScriptGUIElement(instance, vector3Field)
	{

	}

	void ScriptGUIVector3Field::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIVector3Field::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIVector3Field::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIVector3Field::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", &ScriptGUIVector3Field::internal_hasInputFocus);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUIVector3Field::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		UINT32 arrayLen = (UINT32)mono_array_length(guiOptions);
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUIVector3Field* field = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			field = GUIVector3Field::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			field = GUIVector3Field::create(options, styleName);
		}

		field->onValueChanged.connect(std::bind(&ScriptGUIVector3Field::onChanged, instance, _1));

		ScriptGUIVector3Field* nativeInstance = new (bs_alloc<ScriptGUIVector3Field>()) ScriptGUIVector3Field(instance, field);
	}

	void ScriptGUIVector3Field::internal_getValue(ScriptGUIVector3Field* nativeInstance, Vector3* output)
	{
		GUIVector3Field* field = static_cast<GUIVector3Field*>(nativeInstance->getGUIElement());

		*output = field->getValue();
	}

	void ScriptGUIVector3Field::internal_setValue(ScriptGUIVector3Field* nativeInstance, Vector3 value)
	{
		GUIVector3Field* field = static_cast<GUIVector3Field*>(nativeInstance->getGUIElement());

		return field->setValue(value);
	}

	void ScriptGUIVector3Field::internal_hasInputFocus(ScriptGUIVector3Field* nativeInstance, bool* output)
	{
		GUIVector3Field* field = static_cast<GUIVector3Field*>(nativeInstance->getGUIElement());

		*output = field->hasInputFocus();
	}

	void ScriptGUIVector3Field::onChanged(MonoObject* instance, Vector3 newValue)
	{
		MonoException* exception = nullptr;
		onChangedThunk(instance, newValue, &exception);

		MonoUtil::throwIfException(exception);
	}
}