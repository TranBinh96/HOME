#include "Vinfast_Custom.h"

extern int AH_cpy_piece_cost_form(EPM_action_message_t msg)
{
	int iRetCode = ITK_ok;
	tag_t tRootTask = NULLTAG;


	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask == NULLTAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Root tag invalid for handler AH-cpy-piece-cost-form\n", __LINE__, __FUNCTION__);
		return INTERNAL_ERROR;
	}

	std::map<std::string, tag_t> mapSrcForm;
	std::map<std::string, tag_t> mapDestForm;

	//get target object
	int totalTargetObj = 0;
	tag_t* allTargetObj = nullptr;
	CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &totalTargetObj, &allTargetObj));
	if (totalTargetObj >= 1)
	{
		for (int i = 0; i < totalTargetObj; ++i)
		{
			char* objType = nullptr;
			CHECK_ITK(iRetCode, WSOM_ask_object_type2(allTargetObj[i], &objType));
			if (tc_strcasecmp(objType, "VF4_PieceCostForm") == 0)
			{
				char* objName = nullptr;
				CHECK_ITK(iRetCode, WSOM_ask_name2(allTargetObj[i], &objName));
				if (objName == nullptr || objName[0] == '\n' || (std::string(objName)).find("_") == string::npos)
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Invalid object_name for handler AH-cpy-piece-cost-form: %s\n", __LINE__, __FUNCTION__, objName);
					return INTERNAL_ERROR;
				}
				std::vector<std::string> vecStr = split_string2(objName, "_");
				if (vecStr.size() == 2)
				{
					if (tc_strcmp(vecStr.at(1).c_str(), "MFGRequestUpdateCost") == 0)
					{
						//push form has name PartNumber_MFGRequestUpdateCost into source map
						mapSrcForm.insert(std::pair<std::string, tag_t>(vecStr.at(0), allTargetObj[i]));
					}
					else if (tc_strcmp(vecStr.at(1).c_str(), "ActualPieceCost") == 0)
					{
						//push form has name PartNumber_ActualPieceCost into destination map
						mapDestForm.insert(std::pair<std::string, tag_t>(vecStr.at(0), allTargetObj[i]));
					}
					else {
						TC_write_syslog("[VF_handlers]__%d__%s__Invalid object_name for handler AH-cpy-piece-cost-form: %s\n", __LINE__, __FUNCTION__, objName);
						return INTERNAL_ERROR;
					}

				}
				else
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Invalid object_name for handler AH-cpy-piece-cost-form: %s\n", __LINE__, __FUNCTION__, objName);
					return INTERNAL_ERROR;
				}

				SAFE_MEM_free(objName);
			}
			SAFE_MEM_free(objType);
		}
	}

	for (auto destIter = mapDestForm.begin(); destIter != mapDestForm.end(); destIter++)
	{
		std::string partId = destIter->first;
		tag_t destForm = destIter->second;
		CHECK_ITK(iRetCode, AOM_refresh(destForm, 1));
		auto srcIter = mapSrcForm.find(partId);
		if (srcIter != mapSrcForm.end())
		{
			tag_t srcForm = srcIter->second;
			tag_t formType = NULL_TAG;
			CHECK_ITK(iRetCode, TCTYPE_find_type("VF4_PieceCostForm", "", &formType));

			int nProps = 0;
			tag_t* propTag = NULLTAG;

			CHECK_ITK(iRetCode, TCTYPE_list_properties(formType, &nProps, &propTag));

			for (auto i = 0; i < nProps; i++)
			{
				char* propName = NULL;
				PROP_value_type_t valType;
				PROP_protection_t protecType;
				PROP_type_t propType;
				char* valTypeName = NULL;
				char* propTypeName = NULL;
				CHECK_ITK(iRetCode, PROPDESC_ask_name(propTag[i], &propName));
				CHECK_ITK(iRetCode, PROPDESC_ask_protection(propTag[i], &protecType));
				CHECK_ITK(iRetCode, PROPDESC_ask_value_type(propTag[i], &valType, &valTypeName));
				CHECK_ITK(iRetCode, PROPDESC_ask_property_type(propTag[i], &propType, &propTypeName));

				if (propName != NULL && protecType == PROP_write && (std::string(propName)).find("vf4") != string::npos && propType == PROP_runtime)
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Processing property: %s\n", __LINE__, __FUNCTION__, propName);
					char* newStrVal = NULL;
					logical isSrcEmpty = false;
					logical	isDestEmpty = false;
					logical newLogValue = false;
					date_s dateValue;
					switch (valType)
					{
					case PROP_string:
						if (tc_strcmp(propName, "vf4_comment") == 0)
						{
							char* currVal = NULL;
							tag_t owningUser = NULL;
							date_t relDate;
							char* userObjStr = NULL;
							char* strRelDate = NULL;
							std::string finalCmt = "";
							CHECK_ITK(iRetCode, AOM_ask_value_string(destForm, propName, &currVal));
							CHECK_ITK(iRetCode, AOM_ask_value_string(srcForm, propName, &newStrVal));
							CHECK_ITK(iRetCode, AOM_ask_value_tag(srcForm, "owning_user", &owningUser));
							CHECK_ITK(iRetCode, AOM_ask_value_string(owningUser, "object_string", &userObjStr));
							CHECK_ITK(iRetCode, AOM_ask_value_date(srcForm, "date_released", &relDate));
							CHECK_ITK(iRetCode, DATE_date_to_string(relDate, "%Y-%m-%d %H:%M:%S", &strRelDate));

							if (currVal[0] != '\n')
							{
								finalCmt.append(currVal).append("\n").append(strRelDate).append("-").append(userObjStr).append(":").append(newStrVal);
								//tc_strcat(currVal, "\n");
								//tc_strcat(currVal, newStrVal);
							}
							else
							{
								finalCmt.append(strRelDate).append("-").append(userObjStr).append(":").append(newStrVal);
							}
							CHECK_ITK(iRetCode, AOM_set_value_string(destForm, propName, finalCmt.c_str()));
							SAFE_MEM_free(currVal);
							SAFE_MEM_free(userObjStr);
						}
						else
						{
							CHECK_ITK(iRetCode, AOM_ask_value_string(srcForm, propName, &newStrVal));
							CHECK_ITK(iRetCode, AOM_set_value_string(destForm, propName, newStrVal));
						}
						SAFE_MEM_free(newStrVal);
						break;
					case PROP_double:

						CHECK_ITK(iRetCode, AOM_is_null_empty(srcForm, propName, true, &isSrcEmpty));
						CHECK_ITK(iRetCode, AOM_is_null_empty(destForm, propName, true, &isDestEmpty));
						if (!isSrcEmpty)
						{
							double newDblVal = 0.0;
							CHECK_ITK(iRetCode, AOM_ask_value_double(srcForm, propName, &newDblVal));
							CHECK_ITK(iRetCode, AOM_set_value_double(destForm, propName, newDblVal));
						}
						else if (isSrcEmpty && !isDestEmpty)
						{
							//CHECK_ITK(iRetCode, AOM_set_value_double(destForm, propName, 0.0));
							tag_t pomIns = NULLTAG;
							CHECK_ITK(iRetCode, FORM_ask_pom_instance(destForm, &pomIns));

							tag_t classTg = NULL;
							CHECK_ITK(iRetCode, POM_class_of_instance(pomIns, &classTg));

							char* className = NULL;
							CHECK_ITK(iRetCode, POM_name_of_class(classTg, &className));

							tag_t dataFileAttrTg = NULL;
							CHECK_ITK(iRetCode, POM_attr_id_of_attr(propName, className, &dataFileAttrTg));

							CHECK_ITK(iRetCode, POM_refresh_instances(1, &pomIns, NULLTAG, POM_modify_lock));
							CHECK_ITK(iRetCode, POM_set_attr_null(1, &pomIns, dataFileAttrTg));
							CHECK_ITK(iRetCode, POM_save_instances(1, &pomIns, false));
							CHECK_ITK(iRetCode, POM_refresh_instances(1, &pomIns, NULLTAG, POM_no_lock));
						}
						break;
					case PROP_logical:

						CHECK_ITK(iRetCode, AOM_ask_value_logical(srcForm, propName, &newLogValue));
						CHECK_ITK(iRetCode, AOM_set_value_logical(destForm, propName, newLogValue));
						break;
					case PROP_date:
						CHECK_ITK(iRetCode, AOM_ask_value_date(srcForm, propName, &dateValue));
						CHECK_ITK(iRetCode, AOM_set_value_date(destForm, propName, dateValue));
						break;
					case PROP_int:
						break;
					default:
						TC_write_syslog("[VF_handlers]__%d__%s__Ignore property: %s\n", __LINE__, __FUNCTION__, propName);
						break;
					}
				}
				else
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Readonly property: %s\n", __LINE__, __FUNCTION__, propName);
				}
				SAFE_MEM_free(propName);
				SAFE_MEM_free(valTypeName);
			}
		}
		CHECK_ITK(iRetCode, AOM_save_without_extensions(destForm));
		CHECK_ITK(iRetCode, AOM_refresh(destForm, 0));
	}


	SAFE_MEM_free(allTargetObj);
	return iRetCode;
}


