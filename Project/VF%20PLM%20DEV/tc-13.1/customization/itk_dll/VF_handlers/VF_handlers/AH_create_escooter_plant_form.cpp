#include "Vinfast_Custom.h"
/*******************************************************************************
File         : AH_update_plant_code_escooter.cpp

Description  : update plant code based on Make/Buy, Material Type, After Sales Relevant properties
  
Input        : None
						
Output       : None

Author       : VinFast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Jan,14 2021     1.0         Duc		 Initial Creation

/*******************************************************************************/
extern int AH_create_escooter_plant_form(EPM_action_message_t msg)
{
	int result = ITK_ok;
	tag_t rootTask = NULLTAG;
	int count = 0;
	tag_t *attachments = NULLTAG;
	//Get root task
	TC_write_syslog("\nEntering handler AH_update_plant_code_escooter");
	
	CHECK_ITK(result, EPM_ask_root_task(msg.task, &rootTask));
	if(rootTask != NULLTAG)
	{
		
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
			//Find object type
			
			CHECK_ITK(result, WSOM_ask_object_type2(attachments[i], &objtype));
			if(tc_strcmp(objtype, "VF3_Scooter_partRevision") == 0)
			{
				
				CHECK_ITK(result, AOM_ask_value_tag(attachments[i], "items_tag", &item));
				
				CHECK_ITK(result, AOM_ask_value_string(item, "vf4_item_make_buy", &make_buy));
				
				
				CHECK_ITK(result, AOM_ask_value_string(item, "vf4_itm_material_type", &material_type));
				
				
				CHECK_ITK(result, AOM_ask_value_logical(item, "vf4_itm_after_sale_relevant", &is_relevant));
				
				//Set plant code 4001
				if(tc_strcasecmp(make_buy, "BUY") == 0 && tc_strcasecmp(material_type, "ZRAW") == 0 && is_relevant == true)
				{
					//create new plant form and set plant code 4001, PURCHASE
					//Create new plant form
					CHECK_ITK(result, GRM_find_relation_type("VF3_plant_rel", &plant_rel));
					CHECK_ITK(result, TCTYPE_find_type("VF3_plant_form", "Form", &newForm));
					CHECK_ITK(result, TCTYPE_construct_create_input(newForm, &createInput));
					if(result == ITK_ok && createInput != NULLTAG)
					{
						CHECK_ITK(result, AOM_ask_value_string(item, "object_name", &form_name));
						CHECK_ITK(result, AOM_set_value_string(createInput, "object_name", form_name));
						CHECK_ITK(result, AOM_set_value_string(createInput, "vf3_plant", "4001"));
						CHECK_ITK(result, AOM_set_value_string(createInput, "vf3_make_buy", "F"));
						CHECK_ITK(result, TCTYPE_create_object(createInput, &form_tag));
						if(result == ITK_ok && form_tag != NULLTAG)
						{
							CHECK_ITK(result, GRM_create_relation(item, form_tag, plant_rel, NULLTAG, &new_rel));
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
					SAFE_MEM_free(make_buy);
					SAFE_MEM_free(material_type);
					/*for(int j=0; j < count_obj; j++)
					{
						CHECK_ITK(result, WSOM_ask_object_type2(obj_forms[j], &form_type));
						if(tc_strcmp(form_type, "VF3_plant_form") == 0)
						{
							CHECK_ITK(result, AOM_set_value_string(obj_forms[j], "vf3_plant", "4001"));
							CHECK_ITK(result, AOM_set_value_string(obj_forms[j], "vf3_make_buy", "Purchase"));
						}
					}*/

				}
				/*else
				{
					//Set plant code 2001, PURCHASE
					CHECK_ITK(result, GRM_find_relation_type("VF3_plant_rel", &plant_rel));
					CHECK_ITK(result, GRM_list_secondary_objects_only(item, plant_rel, &count_obj, &obj_forms));
					for(int j=0; j < count_obj; j++)
					{
						CHECK_ITK(result, WSOM_ask_object_type2(obj_forms[j], &form_type));
						if(tc_strcmp(form_type, "VF3_plant_form") == 0)
						{
							CHECK_ITK(result, AOM_set_value_string(obj_forms[j], "vf3_plant", "2001"));
							CHECK_ITK(result, AOM_set_value_string(obj_forms[j], "vf3_make_buy", "Purchase"));
						}
					}
				}*/
			}
			SAFE_MEM_free(objtype);
			
			
			
			
		}
		TC_write_syslog("\nExiting handler AH_update_plant_code_escooter");
		
	}
	return result;
	

}