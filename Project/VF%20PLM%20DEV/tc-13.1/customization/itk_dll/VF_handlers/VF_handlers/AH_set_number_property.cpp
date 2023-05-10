#include "Vinfast_Custom.h"

bool isNumber(const std::string& s);

extern int AH_set_number_property(EPM_action_message_t msg)
{
	int iRetCode = ITK_ok;
	tag_t tRootTask = NULLTAG;
	char* argKey = nullptr;
	char* argValue = nullptr;
	char* property = nullptr;
	char* value = nullptr;
	char* fromObjType = nullptr;
	char* toObjType = nullptr;


	//get argument list
	const auto num_args = TC_number_of_arguments(msg.arguments);
	if (num_args > 0)
	{
		for (int i = 0; i < num_args; ++i)
		{
			CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &argKey, &argValue));
			if (iRetCode == ITK_ok)
			{
				if (tc_strcasecmp(argKey, "property") == 0 && argValue != nullptr)
				{
					property = (char*)MEM_alloc(((int)tc_strlen(argValue) + 1) * sizeof(char));
					tc_strcpy(property, argValue);
				}
				if (tc_strcasecmp(argKey, "value") == 0 && argValue != nullptr)
				{
					value = (char*)MEM_alloc(((int)tc_strlen(argValue) + 1) * sizeof(char));
					tc_strcpy(value, argValue);
				}
				if (tc_strcasecmp(argKey, "from_object_type") == 0 && argValue != nullptr)
				{
					fromObjType = (char*)MEM_alloc(((int)tc_strlen(argValue) + 1) * sizeof(char));
					tc_strcpy(fromObjType, argValue);
				}
				if (tc_strcasecmp(argKey, "to_object_type") == 0 && argValue != nullptr)
				{
					toObjType = (char*)MEM_alloc(((int)tc_strlen(argValue) + 1) * sizeof(char));
					tc_strcpy(toObjType, argValue);
				}

			}
			SAFE_MEM_free(argKey);
			SAFE_MEM_free(argValue);
		}
	}
	
	//return if missing any argument
	if (property == nullptr || property[0] == '\n' 
		|| value == nullptr || value[0] == '\n'
		|| toObjType == nullptr || toObjType[0] == '\n')
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Missing argument (property/value) for handler AH_set_number_property\n", __LINE__, __FUNCTION__);
		return MISSING_ARGUMENT;
	}

	std::vector<string> listProp = split_string2(std::string(property), ",");
	std::vector<string> listValue = split_string2(std::string(value), ",");
	
	//return incase miss match Properties and Values
	if (listProp.size() != listValue.size())
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Argument value of Property and Value is not matching for handler AH_set_number_property\n", __LINE__, __FUNCTION__);
		return INVALID_PARAMETER;
	}

	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask == NULLTAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Root tag invalid for handler AH_set_number_property\n", __LINE__, __FUNCTION__);
		return INTERNAL_ERROR;
	}

	int totalReferObj = 0;
	tag_t* allReferObj = nullptr;
	tag_t referObjNeeded = NULL;
	//get target object
	CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_reference_attachment, &totalReferObj, &allReferObj));
	if (fromObjType != nullptr && fromObjType[0] != '\n' && totalReferObj >= 1)
	{
		for (int i = 0; i < totalReferObj; ++i)
		{
			char* objType = nullptr;
			CHECK_ITK(iRetCode, WSOM_ask_object_type2(allReferObj[i], &objType));
			if (tc_strcasecmp(objType, fromObjType) == 0)
			{
				//need to debug
				referObjNeeded = allReferObj[i];
				SAFE_MEM_free(objType);
				SAFE_MEM_free(allReferObj);
				break;
			}
			SAFE_MEM_free(objType);
			SAFE_MEM_free(allReferObj);
		}
	}

	//get reference object
	int totalTargetObj = 0;
	tag_t* allTargetObj = nullptr;
	CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &totalTargetObj, &allTargetObj));
	if (iRetCode != ITK_ok || totalTargetObj <= 0)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__There is no any target object for handler AH_set_number_property\n", __LINE__, __FUNCTION__);
		SAFE_MEM_free(allTargetObj);
		return INTERNAL_ERROR;
	}

	//prepare mapping data <string, double>
	int i = 0;
	std::map<std::string, double> prop2Val;
	for (string prop : listProp)
	{
		double value = -9999;
		std::string rawVal = listValue.at(i);
		if (!rawVal.empty())
		{
			try {
				value = std::stod(rawVal);
			}
			catch (const std::invalid_argument&) {
				if (referObjNeeded == NULL)
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Cannot get property %s due to reference object null\n", __LINE__, __FUNCTION__, rawVal);
					i++;
					continue;
				}
				TC_write_syslog("[VF_handlers]__%d__%s__Get property: %s\n", __LINE__, __FUNCTION__, rawVal);

				logical isEmpty = false;
				CHECK_ITK(iRetCode, AOM_is_null_empty(referObjNeeded, rawVal.c_str(), true, &isEmpty));
				if (isEmpty)
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Value of property %s is empty\n", __LINE__, __FUNCTION__, rawVal);
					prop2Val.insert(std::pair<std::string, double>(prop, value));
					i++;
					continue;
				}

				CHECK_ITK(iRetCode, AOM_ask_value_double(referObjNeeded, rawVal.c_str(), &value));
				if (iRetCode != ITK_ok)
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Number property %s not existed in reference object\n", __LINE__, __FUNCTION__, rawVal);
					i++;
					continue;
				}
			}
			catch (const std::out_of_range&) {
				TC_write_syslog("[VF_handlers]__%d__%s__Argument is out of range for a double: %s\n", __LINE__, __FUNCTION__, rawVal);
				i++;
				continue;
			}
		}
		prop2Val.insert(std::pair<std::string, double>(prop, value));
		i++;
	}

	//loop each target object
	for (int i = 0; i < totalTargetObj; ++i)
	{
		char* targetObjUid = nullptr;
		char* objtType = nullptr;
		ITK__convert_tag_to_uid(allTargetObj[i], &targetObjUid);
		CHECK_ITK(iRetCode, AOM_refresh(allTargetObj[i], 1));
		CHECK_ITK(iRetCode, WSOM_ask_object_type2(allTargetObj[i], &objtType));
		if (tc_strcasecmp(objtType, toObjType) != 0)
		{
			continue;
		}
		for (auto it = prop2Val.begin(); it != prop2Val.end(); it++)
		{
			std::string prop = it->first;
			double value = it->second;
			
			if (value == -9999)
			{
				/*tag_t attr = NULLTAG;
				tag_t tClass = NULLTAG;
				char* className = NULL;
				CHECK_ITK(iRetCode, POM_class_of_instance(allTargetObj[i], &tClass));
				CHECK_ITK(iRetCode, POM_name_of_class(tClass, &className));
				CHECK_ITK(iRetCode, POM_attr_id_of_attr(prop.c_str(), className, &attr));
				CHECK_ITK(iRetCode, POM_clear_attr(1, &allTargetObj[i], attr));*/

				logical isEmpty = false;
				CHECK_ITK(iRetCode, AOM_is_null_empty(allTargetObj[i], prop.c_str(), true, &isEmpty));
				if (!isEmpty)
				{
					//set 0.0 if target value is not empty
					CHECK_ITK(iRetCode, AOM_set_value_double(allTargetObj[i], prop.c_str(), 0.0));
				}
			}
			else
			{
				CHECK_ITK(iRetCode, AOM_set_value_double(allTargetObj[i], prop.c_str(), value));
				if (iRetCode != ITK_ok)
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Cannot set property %s for object %s\n", __LINE__, __FUNCTION__, prop, targetObjUid);
					continue;
				}
			}
		}
		CHECK_ITK(iRetCode, AOM_save(allTargetObj[i]));
		CHECK_ITK(iRetCode, AOM_refresh(allTargetObj[i], 0));
		SAFE_MEM_free(objtType);
	}
	SAFE_MEM_free(allTargetObj);
	return iRetCode;
}


bool isNumber(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}