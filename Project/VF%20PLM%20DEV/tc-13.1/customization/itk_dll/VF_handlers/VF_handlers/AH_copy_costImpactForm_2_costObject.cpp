/***************************************************************************************
Function     : AH_copy_costImpactForm_2_costObject()
Description  : check and copy cost impact form to cost object
Input        : None
Output       : None
Author       : thanhpn6
******************************************************************************************/
#include "Vinfast_Custom.h"

extern int AH_copy_costImpactForm_2_costObject(EPM_action_message_t msg)
{
	int retCode = ITK_ok;
	char ErrorMessage [2048] = "";
	tag_t rootTskObj = NULLTAG;

	/*get root task*/
	CHECK_ITK(retCode, EPM_ask_root_task(msg.task, &rootTskObj));
	if(rootTskObj !=  NULLTAG)
	{
		map<std::string, std::vector<tag_t>> id2CostForm;
		char* ecrName = nullptr;
		CHECK_ITK(retCode, WSOM_ask_name2(rootTskObj, &ecrName));
		TC_write_syslog("[VF_handler]__%d__%s__Processing ECR: %s\n", __LINE__, __FUNCTION__, ecrName);

		/*get cost impact form*/
		int tgCounter = 0;
		tag_t *tgObj = nullptr;
		CHECK_ITK(retCode, EPM_ask_attachments(rootTskObj, EPM_target_attachment, &tgCounter, &tgObj));

		tag_t relaObj = NULLTAG;
		CHECK_ITK(retCode, GRM_find_relation_type("VF4_ECRCostImpactRelation", &relaObj));
		for(int i = 0; i < tgCounter; ++i)
		{
			char* objName = nullptr;
			CHECK_ITK(retCode, WSOM_ask_name2(tgObj[i], &objName));

			char* objType = nullptr;
			CHECK_ITK(retCode, WSOM_ask_object_type2(tgObj[i], &objType));
			if(tc_strcasecmp(objType, "Vf6_engineering") != 0 && tc_strcasecmp(objType, "Vf6_logistics") != 0 &&
				tc_strcasecmp(objType, "Vf6_manufacturing") != 0 && tc_strcasecmp(objType, "Vf6_purchasing") != 0)
			{
				/*Ignore all item is not Cost Impact Form*/
				SAFE_MEM_free(objType);
				continue;
			}
			char* formUID = nullptr;
			ITK__convert_tag_to_uid(tgObj[i], &formUID);
			TC_write_syslog("[VF_handler]__%d__%s__Processing form: %s-%s\n", __LINE__, __FUNCTION__, formUID, objName);

			/*check whether Cost Form has been attached to a Cost Object?*/

			int objCounter = 0;
			tag_t *costObj = NULL_TAG;
			CHECK_ITK(retCode, GRM_list_primary_objects_only(tgObj[i], relaObj, &objCounter, &costObj));
			if(objCounter == 0)
			{
				TC_write_syslog("[VF_handler]__%d__%s__Cost form has not attached to cost object: %s-%s\n", __LINE__, __FUNCTION__, formUID, objName);
				/*create a mapping between id and cost form*/
				if(std::string(objName).find('-') != std::string::npos)
				{
					std::vector<std::string> prefixAndRev = split_string2(std::string(objName), "-");
					if(prefixAndRev.at(1).find('/') != std::string::npos)
					{
						std::vector<std::string> idAndRev = split_string2(prefixAndRev.at(1), "/");
						if(id2CostForm.find(idAndRev.at(0)) != id2CostForm.end())
						{
							auto itr = id2CostForm.find(idAndRev.at(0));
							itr->second.push_back(tgObj[i]);
						}
						else
						{
							std::vector<tag_t> vecCostform;
							vecCostform.push_back(tgObj[i]);
							id2CostForm.insert(std::pair<std::string, std::vector<tag_t>>(idAndRev.at(0), vecCostform));
						}
					}
				}

			}
			SAFE_MEM_free(objName);
			SAFE_MEM_free(objType);
			SAFE_MEM_free(costObj);
		}
		
		for(auto it = id2CostForm.begin(); it != id2CostForm.end(); it++)
		{
			std::string id = it->first;
			std::vector<tag_t> lstCostForm = it->second;

			tag_t queryObj = NULLTAG;
			CHECK_ITK(retCode, QRY_find2( "Admin - Latest Cost Revision", &queryObj))
				if(retCode != ITK_ok)
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Cannot find query [Admin - Latest Cost Revision]: %d\n", __LINE__, __FUNCTION__, retCode);
					return -1;
				}

				const int entryCounter = 1;
				char  *entries[1] = {"Item ID"};
				char **values = nullptr;

				values = (char**) MEM_alloc(entryCounter * sizeof(char*));

				values[0] = (char*)MEM_alloc(strlen(id.c_str()) + 1);
				tc_strcpy(values[0], id.c_str());

				int resCounter = 0;
				tag_t* costObj = nullptr;

				/*query cost item*/
				CHECK_ITK(retCode, QRY_execute(queryObj, entryCounter, entries, values, &resCounter, &costObj))
					if(retCode != ITK_ok || resCounter == 0 || resCounter > 1)
					{
						TC_write_syslog("[VF_handlers]__%d__%s__Cannot find cost item: %s\n", __LINE__, __FUNCTION__, id.c_str());
					}
					else
					{
						if(resCounter > 0)
						{
							/*create relation from Cost Form to Cost Object*/
							for(tag_t costForm : lstCostForm)
							{
								tag_t newRelation = NULLTAG;
								CHECK_ITK(retCode, GRM_create_relation(costObj[0], costForm, relaObj, NULLTAG, &newRelation));
								CHECK_ITK(retCode, GRM_save_relation (newRelation));
							}
						}
					}
			SAFE_MEM_free(costObj)
		}

		SAFE_MEM_free(ecrName);
		SAFE_MEM_free(tgObj);
	}
	return retCode;
}