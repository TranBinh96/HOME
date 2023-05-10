#include "Vinfast_Custom.h"
/*******************************************************************************
File         : AH_create_escooter_plant_form.cpp

Description  : update plant code based on Make/Buy, Material Type, After Sales Relevant properties

Input        : None

Output       : None

Author       : VinFast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Jan,14 2021     1.0         Duc		 Initial Creation
Feb,17 2021		1.1			Duc		 Adding plant code 4002 for cars
Feb,19 2021		1.2			Duc		 Change Logic input plant code
/*******************************************************************************/
extern int AH_create_or_fill_plant_form(EPM_action_message_t msg)
{
	int result = ITK_ok;
	tag_t rootTask = NULLTAG;
	int count = 0;
	tag_t* attachments = NULLTAG;
	int argNums = 0;
	char* flag = NULL;
	char* value = NULL;
	char* obtype = NULL;
	char* plantcode = NULL;
	char* type = NULL;
	char* procurement_type = NULL;
	//Get root task
	TC_write_syslog("\n[VF] ENTER %s", __FUNCTION__);
	//getting arguments
	argNums = TC_number_of_arguments(msg.arguments);
	if (argNums > 0)
	{
		for (int i = 0; i < argNums; i++)
		{
			result = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &flag, &value);

			if (tc_strcmp(flag, "plant_code") == 0)
			{
				plantcode = (char*)MEM_alloc(((int)tc_strlen(value + 1) * sizeof(char)));
				tc_strcpy(plantcode, value);
			}

			if (tc_strcmp(flag, "type") == 0)
			{
				type = (char*)MEM_alloc(((int)tc_strlen(value + 1) * sizeof(char)));
				tc_strcpy(type, value);
			}

		}
	}
	else
	{
		result = EPM_wrong_number_of_arguments;
	}
	CHECK_ITK(result, EPM_ask_root_task(msg.task, &rootTask))

	if (rootTask != NULLTAG)
	{
		//TC_write_syslog("\nplant code: %s", plantcode );
		//Get target objects
		CHECK_ITK(result, EPM_ask_attachments(rootTask, EPM_target_attachment, &count, &attachments));
		for (int i = 0; i < count; i++)
		{
			char* objtype = NULL;
			char* form_type = NULL;
			tag_t item = NULLTAG;
			char* make_buy;
			char* form_name;
			bool is_relevant;
			int count_obj = 0;
			tag_t plant_rel = NULLTAG;
			tag_t* obj_forms = NULLTAG;
			tag_t newForm = NULLTAG;
			tag_t createInput = NULLTAG;
			tag_t form_tag = NULLTAG;
			tag_t new_rel = NULLTAG;
			int count_form = 0;
			tag_t* forms = NULLTAG;
			
			/*Find object type
			if it is car plant code*/
			CHECK_ITK(result, WSOM_ask_object_type2(attachments[i], &objtype));

			bool isSkipped = false;

			tag_t targetType = NULLTAG;
			logical isDesiredType = false;
						
			CHECK_ITK(result, TCTYPE_ask_object_type(attachments[i], &targetType));
			CHECK_ITK(result, TCTYPE_is_type_of_as_str(targetType, type, &isDesiredType));

			isSkipped = !isDesiredType;
			if (isSkipped == true)
			{
				continue;
			}

			char* procurementTypeAttr = NULL;
			char* plantCodeAttr = NULL;
			char* plantFormType = NULL;
			char* plantFormRelation = NULL;

			if (tc_strstr(objtype, "VF4_"))
			{
				procurementTypeAttr = "vf4_make_buy";
				plantCodeAttr = "vf4_plant";
				plantFormType = "VF4_plant_form";
				plantFormRelation = "VF4_plant_form_relation";
			}
			else if (tc_strstr(objtype, "VF3_"))
			{
				procurementTypeAttr = "vf3_make_buy";
				plantCodeAttr = "vf3_plant";
				plantFormType = "VF3_plant_form";
				plantFormRelation = "VF3_plant_rel";
			}
			else
			{
				char errorMsg[1024] = { "" };
				sprintf(errorMsg, "Do not know plant form attributes for \"%s\"", objtype);
				EMH_store_error_s1(EMH_severity_user_error, -1, errorMsg);
				return -1;
			}

			//Check if Item have plant form before
			CHECK_ITK(result, GRM_find_relation_type(plantFormRelation, &plant_rel));
			CHECK_ITK(result, GRM_list_secondary_objects_only(attachments[i], plant_rel, &count_form, &forms));
			tag_t theEmptyForm = NULLTAG;
			bool desiredFormExisted = false;
			for (int j = 0; j < count_form; j++)
			{
				char* exist_plantcode = NULL;
				CHECK_ITK(result, AOM_ask_value_string(forms[j], plantCodeAttr, &exist_plantcode));
				logical isMakeBuyNull = false;
				logical isPlantCodeNull = false;
				logical isZeroLength = false;
				CHECK_ITK(result, AOM_is_null_empty(forms[j], procurementTypeAttr, &isZeroLength, &isMakeBuyNull));
				CHECK_ITK(result, AOM_is_null_empty(forms[j], plantCodeAttr, &isZeroLength, &isPlantCodeNull));

				if (isMakeBuyNull && isPlantCodeNull)
				{
					theEmptyForm = forms[j];
				}
				else if (tc_strcasecmp(exist_plantcode, plantcode) == 0)
				{
					desiredFormExisted = true;
					SAFE_SM_FREE(exist_plantcode);
					break;
				}

				SAFE_SM_FREE(exist_plantcode);
			}

			if (desiredFormExisted == false)
			{
				CHECK_ITK(result, AOM_ask_value_string(attachments[i], "vf4_item_make_buy", &make_buy));
				if (tc_strcasecmp(make_buy, "Make") == 0)
				{
					procurement_type = "E";
				}
				else if(tc_strcasecmp(make_buy, "Buy") == 0)
				{
					procurement_type = "F";
				}
				TC_write_syslog("\n[VF] procurement_type=%s", procurement_type);
				TC_write_syslog("\n[VF] make_buy=%s", make_buy);
				if (theEmptyForm == NULLTAG)
				{
					CHECK_ITK(result, TCTYPE_find_type(plantFormType, "Form", &newForm));
					CHECK_ITK(result, TCTYPE_construct_create_input(newForm, &createInput));
					if (result == ITK_ok && createInput != NULLTAG)
					{
						CHECK_ITK(result, AOM_ask_value_string(attachments[i], "object_name", &form_name));
						CHECK_ITK(result, AOM_set_value_string(createInput, "object_name", form_name));
						CHECK_ITK(result, AOM_set_value_string(createInput, plantCodeAttr, plantcode));
						CHECK_ITK(result, AOM_set_value_string(createInput, procurementTypeAttr, procurement_type));
						CHECK_ITK(result, TCTYPE_create_object(createInput, &form_tag));
						if (result == ITK_ok && form_tag != NULLTAG)
						{
							CHECK_ITK(result, GRM_create_relation(attachments[i], form_tag, plant_rel, NULLTAG, &new_rel));
							if (result == ITK_ok && new_rel != NULLTAG)
							{
								CHECK_ITK(result, AOM_save_with_extensions(form_tag));
								if (result == ITK_ok)
								{
									CHECK_ITK(result, AOM_refresh(form_tag, 0));
									if (result == ITK_ok)
									{
										CHECK_ITK(result, AOM_save_with_extensions(new_rel));
									}
								}
							}

						}
						SAFE_MEM_free(form_name);
					}
				}
				else
				{
					CHECK_ITK(result, AOM_refresh(theEmptyForm, TRUE));

					CHECK_ITK(result, AOM_set_value_string(theEmptyForm, plantCodeAttr, plantcode));
					CHECK_ITK(result, AOM_set_value_string(theEmptyForm, procurementTypeAttr, procurement_type));
					
					CHECK_ITK(result, AOM_save_with_extensions(theEmptyForm));
					CHECK_ITK(result, AOM_refresh(theEmptyForm, FALSE));
				}

				SAFE_MEM_free(make_buy);
			}

			SAFE_SM_FREE(forms);

		}


	}

	SAFE_MEM_free(type);
	SAFE_MEM_free(plantcode);
	TC_write_syslog("\n[VF] LEAVE %s", __FUNCTION__);
	return result;
}