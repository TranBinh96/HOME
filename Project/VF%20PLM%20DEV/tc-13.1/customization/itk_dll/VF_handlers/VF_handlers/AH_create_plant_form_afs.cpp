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
extern int AH_create_plant_form_afs(EPM_action_message_t msg)
{
	int result = ITK_ok;
	tag_t rootTask = NULLTAG;
	int count = 0;
	tag_t *attachments = NULLTAG;
	int argNums = 0;
	char *flag = NULL;
	char *value = NULL;
	char *obtype = NULL;
	char *plantcode = NULL;
	char *procurement_type = NULL;
	//Get root task
	TC_write_syslog("\nEntering handler AH_create_plant_form_zraw");
	//getting arguments
	argNums = TC_number_of_arguments(msg.arguments);
	if(argNums > 0)
	{
		for(int i=0; i < argNums; i++)
		{
			result = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &flag, &value);
			
			if(tc_strcmp(flag, "plant") == 0)
			{
				plantcode = (char*) MEM_alloc(((int) tc_strlen(value + 1) * sizeof(char)) );
				tc_strcpy(plantcode, value);
			}
		}
	}
	else
	{
		result = EPM_wrong_number_of_arguments;
	}
	CHECK_ITK(result, EPM_ask_root_task(msg.task, &rootTask));
	if(rootTask != NULLTAG)
	{
		//TC_write_syslog("\nplant code: %s", plantcode );
		//Get target objects
		CHECK_ITK(result, EPM_ask_attachments(rootTask, EPM_target_attachment, &count, &attachments));
		for(int i=0; i < count; i++)
		{
			char *objtype = NULL;
			char *form_type = NULL;
			tag_t item = NULLTAG;
			char *make_buy;
			char *material_type;
			char *form_name;
			bool is_relevant;
			int count_obj = 0;
			tag_t plant_rel = NULLTAG;
			tag_t *obj_forms = NULLTAG;
			tag_t newForm  = NULLTAG;
			tag_t createInput = NULLTAG;
			tag_t form_tag = NULLTAG;
			tag_t new_rel = NULLTAG;
			int count_form = 0;
			tag_t *forms = NULLTAG;
			char *exist_plantcode;
			bool create_form = true;
			/*Find object type
			if it is car plant code*/
			CHECK_ITK(result, WSOM_ask_object_type2(attachments[i], &objtype));
			if(tc_strcasecmp(plantcode, "4002") == 0)
			{
				if(tc_strcasecmp(objtype, "VF4_Design") == 0 || tc_strcasecmp(objtype, "VF4_BP_Design") == 0)
				{
					//Check if Item have plant form before
					CHECK_ITK(result, GRM_find_relation_type("VF4_plant_form_relation", &plant_rel));
					CHECK_ITK(result, GRM_list_secondary_objects_only(attachments[i], plant_rel, &count_form, &forms));
					for(int j = 0; j < count_form; j++)
					{
						//if have 4002 form before
						CHECK_ITK(result, AOM_ask_value_string(forms[j], "vf4_plant", &exist_plantcode));
						if(tc_strcasecmp(exist_plantcode, "4002") == 0)
						{
							create_form = false;
						}
						

					}
					if(create_form == true)
					{
						CHECK_ITK(result, AOM_ask_value_string(attachments[i], "vf4_item_make_buy", &make_buy));
				
				
						CHECK_ITK(result, AOM_ask_value_string(attachments[i], "vf4_itm_material_type", &material_type));
				
				
						CHECK_ITK(result, AOM_ask_value_logical(attachments[i], "vf4_itm_after_sale_relevant", &is_relevant));

						//Set plant code 4002
						if(tc_strcasecmp(make_buy, "BUY") == 0 && tc_strcasecmp(material_type, "ZRAW") == 0 && is_relevant == true)
						{
							//create new plant form and set plant code 4002, PURCHASE
							//Create new plant form
							//CHECK_ITK(result, GRM_find_relation_type("VF4_plant_form_relation", &plant_rel));
							CHECK_ITK(result, TCTYPE_find_type("VF4_plant_form", "Form", &newForm));
							CHECK_ITK(result, TCTYPE_construct_create_input(newForm, &createInput));
							if(result == ITK_ok && createInput != NULLTAG)
							{
								CHECK_ITK(result, AOM_ask_value_string(attachments[i], "object_name", &form_name));
								CHECK_ITK(result, AOM_set_value_string(createInput, "object_name", form_name));
								CHECK_ITK(result, AOM_set_value_string(createInput, "vf4_plant", plantcode));
								CHECK_ITK(result, AOM_set_value_string(createInput, "vf4_make_buy", "F"));
								CHECK_ITK(result, TCTYPE_create_object(createInput, &form_tag));
								if(result == ITK_ok && form_tag != NULLTAG)
								{
									CHECK_ITK(result, GRM_create_relation(attachments[i], form_tag, plant_rel, NULLTAG, &new_rel));
									if(result == ITK_ok && new_rel != NULLTAG)
									{
										CHECK_ITK(result, AOM_save(form_tag));
										if(result == ITK_ok)
										{
											CHECK_ITK(result, AOM_refresh(form_tag,0));
											if(result == ITK_ok)
											{
												CHECK_ITK(result, AOM_save(new_rel));									
											}
										}
									}

								}
								SAFE_MEM_free(form_name);
							}
						
						}
						SAFE_MEM_free(make_buy);
						SAFE_MEM_free(material_type);
						SAFE_MEM_free(exist_plantcode);
					}
					else if(create_form == false)
					{
						SAFE_MEM_free(exist_plantcode);
					}
						
						
		
				}
		}
		else if(tc_strcasecmp(plantcode, "4001") == 0)
		{
			if(tc_strcasecmp(objtype, "VF3_Scooter_part") == 0 || tc_strcasecmp(objtype, "VF3_service") == 0 || tc_strcasecmp(objtype, "VF3_car") == 0)
			{
				//Check if Item have plant form before
					CHECK_ITK(result, GRM_find_relation_type("VF3_plant_rel", &plant_rel));
					CHECK_ITK(result, GRM_list_secondary_objects_only(attachments[i], plant_rel, &count_form, &forms));
					for(int k = 0; k < count_form; k++)
					{
						//if have 4002 form before
						CHECK_ITK(result, AOM_ask_value_string(forms[k], "vf3_plant", &exist_plantcode));
						if(tc_strcasecmp(exist_plantcode, "4001") == 0)
						{
							create_form = false;
						}
						

					}
					if(create_form == true)
					{
						CHECK_ITK(result, AOM_ask_value_string(attachments[i], "vf4_item_make_buy", &make_buy));
				
				
						CHECK_ITK(result, AOM_ask_value_string(attachments[i], "vf4_itm_material_type", &material_type));
				
				
						CHECK_ITK(result, AOM_ask_value_logical(attachments[i], "vf4_itm_after_sale_relevant", &is_relevant));

						//Set plant code 4001
						if(tc_strcasecmp(make_buy, "BUY") == 0 && tc_strcasecmp(material_type, "ZRAW") == 0 && is_relevant == true)
						{


							//create new plant form and set plant code 4001, PURCHASE
							//Create new plant form
							//CHECK_ITK(result, GRM_find_relation_type("VF3_plant_rel", &plant_rel));
							CHECK_ITK(result, TCTYPE_find_type("VF3_plant_form", "Form", &newForm));
							CHECK_ITK(result, TCTYPE_construct_create_input(newForm, &createInput));
							if(result == ITK_ok && createInput != NULLTAG)
							{
								CHECK_ITK(result, AOM_ask_value_string(attachments[i], "object_name", &form_name));
								CHECK_ITK(result, AOM_set_value_string(createInput, "object_name", form_name));
								CHECK_ITK(result, AOM_set_value_string(createInput, "vf3_plant", plantcode));
								CHECK_ITK(result, AOM_set_value_string(createInput, "vf3_make_buy", "F"));
								CHECK_ITK(result, TCTYPE_create_object(createInput, &form_tag));
								if(result == ITK_ok && form_tag != NULLTAG)
								{
									CHECK_ITK(result, GRM_create_relation(attachments[i], form_tag, plant_rel, NULLTAG, &new_rel));
									if(result == ITK_ok && new_rel != NULLTAG)
									{
										CHECK_ITK(result, AOM_save(form_tag));
										if(result == ITK_ok)
										{
											CHECK_ITK(result, AOM_refresh(form_tag,0));
											if(result == ITK_ok)
											{
												CHECK_ITK(result, AOM_save(new_rel));									
											}
										}
									}
								}
								SAFE_MEM_free(form_name);
							}
			}

						SAFE_MEM_free(make_buy);
						SAFE_MEM_free(material_type);
						SAFE_MEM_free(exist_plantcode);
					}
					else if(create_form == false)
					{
						SAFE_MEM_free(exist_plantcode);
					}
						
		}
		
		
		
	}
	SAFE_MEM_free(objtype);
	}
	}
	TC_write_syslog("\nExiting handler AH_create_plant_form_zraw");
	return result;
}