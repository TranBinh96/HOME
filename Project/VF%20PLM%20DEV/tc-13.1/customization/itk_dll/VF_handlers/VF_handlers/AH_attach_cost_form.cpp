/*******************************************************************************
File         : AH_attach_cost_form.cpp

Description  : To create 4 cost form under cost forms folder of ECR revision
               based on no of Item Rev present in Propsal folder
  
Input        : None
                        
Output       : None

Author       : Siemens

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Dec,26 2018     1.0         Kantesh		     Initial Creation
Nov,13 2019     1.1         Nguyen           Move filer types to preference VF_ECR_Proposed_Types_To_Create_Cost_Form

*******************************************************************************/
#include "Vinfast_Custom.h"
#include <map>

tag_t queryCost(char *partID)
{
	TC_write_syslog("\n[VF] ENTER %s", __FUNCTION__);
	tag_t costRev = NULLTAG;
	tag_t query_object = NULLTAG;
	int ret_code = ITK_ok;

	const char *queryName = "VF Cost";
	CHECK_ITK(ret_code, QRY_find2(queryName, &query_object))
	if(ret_code != ITK_ok)
	{
		TC_write_syslog("\n[VF] not found query \"%s\"", queryName);
		return NULLTAG;
	}
	
	const int n_entries = 1;
	char  *entries[1] = {"ID"};
	char **values = nullptr;

	TC_write_syslog("\n[VF] partID=%s", partID);
	values = (char**) MEM_alloc(n_entries * sizeof(char*));

	values[0] = (char*)MEM_alloc(tc_strlen(partID) + 1);
	tc_strcpy(values[0], partID );
	TC_write_syslog("\n[VF] partID=%s", values[0]);

	auto num_found = 0;
	tag_t* total_results = nullptr;
	
	CHECK_ITK(ret_code, QRY_execute(query_object, n_entries, entries, values, &num_found, &total_results))
	if (num_found == 1)
	{
		costRev = total_results[0];
		char *costRevUID = NULL;
		ITK__convert_tag_to_uid(costRev, &costRevUID);
		TC_write_syslog("\n[VF] found costRev=%s", costRevUID);
		SAFE_SM_FREE(costRevUID);
	}
	else
	{
		TC_write_syslog("\n[VF] found more than one cost rev for part ID \"%s\"", partID);
	}

	SAFE_SM_FREE(values[0]);
	SAFE_SM_FREE(values);

	TC_write_syslog("\n[VF] LEAVE %s", __FUNCTION__);
	return costRev;
}

tag_t createCostRev(const char *costID, const char *costName)
{
	TC_write_syslog("\n[VF] ENTER %s", __FUNCTION__);

	int   ret_code                 = ITK_ok;
	tag_t item_type_tag            = NULLTAG;
	tag_t itemRev_type_tag         = NULLTAG;
	tag_t item_create_input_tag    = NULLTAG;
	tag_t itemRev_create_input_tag = NULLTAG;
	tag_t item_tag                 = NULLTAG;


	CHECK_ITK(ret_code, TCTYPE_find_type("VF4_Cost", "VF4_Cost", &item_type_tag))
	CHECK_ITK(ret_code, TCTYPE_find_type("VF4_CostRevision", "VF4_CostRevision", &itemRev_type_tag))
	CHECK_ITK(ret_code, TCTYPE_construct_create_input (item_type_tag, &item_create_input_tag))
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "item_id", 1, &costID))
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "object_name", 1, &costName))

	CHECK_ITK(ret_code, TCTYPE_construct_create_input(itemRev_type_tag,&itemRev_create_input_tag))
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "object_name", 1, &costName))
	CHECK_ITK(ret_code, TCTYPE_create_object(item_create_input_tag, &item_tag))
	CHECK_ITK(ret_code, AOM_save_with_extensions(item_tag))

	tag_t revTag = NULLTAG;
	CHECK_ITK(ret_code, ITEM_ask_latest_rev(item_tag, &revTag));

	TC_write_syslog("\n[VF] LEAVE %s", __FUNCTION__);
	return revTag;
}

void copyCostFormsToCostObject(const std::vector<tag_t> &costForms, const tag_t &partRev, char *itemID, vector<tag_t> &costRevs)
{
	TC_write_syslog("\n[VF] ENTER %s", __FUNCTION__);
	
	int iRetCode = ITK_ok;
	// search cost object
	tag_t costRev = queryCost(itemID);	
		
	// if not found create
	if (costRev == NULL)
	{
		char *partRevName = NULL;
		CHECK_ITK(iRetCode, AOM_ask_value_string(partRev, "object_name", &partRevName));
		costRev = createCostRev(itemID, partRevName);
		if (costRev != NULL) costRevs.push_back(costRev);
		SAFE_SM_FREE(partRevName);
	}

	// copy cost forms to cost object rel VF4_ECRCostImpactRelation
	if (costRev != NULL)
	{
		tag_t tFormRel = NULLTAG;
		CHECK_ITK(iRetCode, GRM_find_relation_type ("VF4_ECRCostImpactRelation", &tFormRel));

		for (int i = 0; i < costForms.size(); i++)
		{
			tag_t tRel = NULLTAG;
			CHECK_ITK(iRetCode, GRM_create_relation(costRev, costForms[i], tFormRel, NULLTAG, &tRel));
			if(iRetCode == ITK_ok && tRel != NULLTAG)
			{
				CHECK_ITK(iRetCode, GRM_save_relation (tRel));
			}
		}
	}
	else
	{
		// throw error
		// workflow should stop???

		TC_write_syslog("\n[VF] ERROR: CANNOT FOUND AND CREATE COST \"%s\"", itemID);
	}
	// link cost object to rev???

	TC_write_syslog("\n[VF] LEAVE %s", __FUNCTION__);
}

void fixCostFormsName(const vector<tag_t> &costRevs)
{
	TC_write_syslog("\n[VF] ENTER %s", __FUNCTION__);
	tag_t totalCostForms[1024];
	int   totalCostFormsNum = 0;
	int iRetCode = ITK_ok;

	for (int j = 0; j < costRevs.size(); j++)
	{
		tag_t costRev = costRevs[j];
		tag_t *costForms = NULL;
		int costFormsNum = 0;
		CHECK_ITK(iRetCode, AOM_ask_value_tags(costRev, "VF4_SourcingCostFormRela", &costFormsNum, &costForms));	
	
		for (int i = 0; i < costFormsNum; i++)
		{
			totalCostForms[totalCostFormsNum++] = costForms[i];
		}

		SAFE_SM_FREE(costForms);
	}

	tag_t costFormAdjustmentTemplate = NULLTAG;
	int attachmentTypes[1024];
	tag_t createdProcess             = NULLTAG;
	for (int i = 0; i < 1024; i++) attachmentTypes[i] = 1;
	CHECK_ITK(iRetCode, EPM_find_process_template("VF_IT_Adjust_Cost_Form_name", &costFormAdjustmentTemplate));
	CHECK_ITK(iRetCode, EPM_create_process("Adjust cost forms name", "System-Auto-Triggered", costFormAdjustmentTemplate, totalCostFormsNum, totalCostForms, attachmentTypes, &createdProcess));

	for (int j = 0; j < costRevs.size(); j++)
	{
		tag_t costRev = costRevs[j];
		tag_t *costForms = NULL;
		char *costID = NULL;
		int costFormsNum = 0;

		CHECK_ITK(iRetCode, AOM_ask_value_string(costRev, "item_id", &costID));
		CHECK_ITK(iRetCode, AOM_ask_value_tags(costRev, "VF4_SourcingCostFormRela", &costFormsNum, &costForms));	
	
		for (int i = 0; i < costFormsNum; i++)
		{
			char *costFormName = NULL;
			CHECK_ITK(iRetCode, AOM_ask_value_string(costForms[i], "object_name", &costFormName));
			if (tc_strstr(costFormName, costID) == NULL) 
			{
				SAFE_SM_FREE(costFormName);
				char *uid = NULL;
				ITK__convert_tag_to_uid(costForms[i], &uid);
				std::string errMsg("[VF] Cannot adjust cost form name of ");
				errMsg.append(uid);
				CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, -1, errMsg.c_str()));
				SAFE_SM_FREE(uid);	
			}

			SAFE_SM_FREE(costFormName);
		}

		SAFE_SM_FREE(costID);
		SAFE_SM_FREE(costForms);
	}

	TC_write_syslog("\n[VF] LEAVE %s", __FUNCTION__);
}

void createCostForms(tag_t ecrRev, tag_t tFormRel, std::vector<std::string> relations, std::map<tag_t, int> &itemsCreatedCostForms, bool checkIfUnique)
{
	TC_write_syslog("\n[VF] ENTER %s", __FUNCTION__);

	std::map<tag_t, std::string> processedParts;
	vector<tag_t> costRevs;
	int iRetCode = ITK_ok;
	int		iProp			= 0;
	tag_t	tPropType		= NULLTAG;
	tag_t	*ptProp			= NULL;
	tag_t	tItem			= NULLTAG;
    tag_t	tChangeItem		= NULLTAG;
	tag_t   tCrtdLogRel		= NULLTAG;
	char    *ecrNumber      = NULLTAG;

	CHECK_ITK(iRetCode,AOM_ask_value_string(ecrRev, "item_id", &ecrNumber));
    for (auto relationStr : relations)
	{
		/*get all item revisions from input relations*/
		CHECK_ITK(iRetCode,GRM_find_relation_type(relationStr.c_str(), &tPropType));
		CHECK_ITK(iRetCode,GRM_list_secondary_objects_only(ecrRev,tPropType,&iProp,&ptProp));
		for(int iNum = 0; iNum < iProp; iNum++)
		{
			tag_t   tManuForm					= NULLTAG;
			tag_t   tLogForm					= NULLTAG;
			tag_t   tEngForm					= NULLTAG;
			tag_t   tPurForm					= NULLTAG;
			tag_t   tCreateInputManuForm		= NULLTAG;
			tag_t   tCreateInputLogForm			= NULLTAG;
			tag_t   tCreateInputEngForm			= NULLTAG;
			tag_t   tCreateInputPurForm			= NULLTAG;
			tag_t   tManuCostForm				= NULLTAG;
			tag_t   tLogCostForm				= NULLTAG;
			tag_t   tEngCostForm				= NULLTAG;
			tag_t   tPurCostForm				= NULLTAG;
			tag_t   tCrtdManuRel				= NULLTAG;
			tag_t   tCrtdEngRel					= NULLTAG;
			tag_t   tCrtdPurRel					= NULLTAG;
			char	*pcPropType					= NULL;
			char	*pcItem						= NULL;
			char	*pcRev						= NULL;
			string   szItemRev					= "";
			string   szManuForm					= "";
			string   szEngForm					= "";
			string   szLogForm					= "";
			string   szPurcForm					= "";
						
			CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptProp[iNum],&pcPropType));
						
			char **szTypes = NULL;
			int iTypesNum = -1;
			CHECK_ITK(iRetCode, PREF_ask_char_values("VF_ECR_Proposed_Types_To_Create_Cost_Form", &iTypesNum, &szTypes));
			logical isCreateForms = false;
			for (int inxTypes = 0; inxTypes < iTypesNum; inxTypes++)
			{
				if (tc_strcmp(szTypes[inxTypes], pcPropType) == 0)
				{
					isCreateForms = true;
					break;
				}
			}
			SAFE_MEM_free(szTypes);
						
			CHECK_ITK(iRetCode,ITEM_ask_item_of_rev (ptProp[iNum],&tItem));
			if(isCreateForms && processedParts.find(ptProp[iNum]) == processedParts.end() && (!checkIfUnique || itemsCreatedCostForms.find(tItem) == itemsCreatedCostForms.end()))
			{
				processedParts[ptProp[iNum]] = "done";
				if(iRetCode == ITK_ok && tItem != NULLTAG)
				{
					/*get item and rev id*/
					CHECK_ITK(iRetCode,ITEM_ask_id2 (tItem,&pcItem));
					CHECK_ITK(iRetCode,ITEM_ask_rev_id2 (ptProp[iNum],&pcRev));
					itemsCreatedCostForms[tItem] = 1;

					/*append item and rev*/
					szItemRev.assign(pcItem);
					szItemRev.append("/");
					szItemRev.append(pcRev);

					szManuForm.assign("Manufacturing Cost - ");
					szManuForm.append(szItemRev);

					szLogForm.assign("Logistics Cost - ");
					szLogForm.append(szItemRev);

					szEngForm.assign("Engineering Cost - ");
					szEngForm.append(szItemRev);

					szPurcForm.assign("Purchasing Cost - ");
					szPurcForm.append(szItemRev);
                            
					/*create manufacturing form*/
					CHECK_ITK(iRetCode,TCTYPE_find_type ("Vf6_manufacturing","Form",&tManuForm));
					if(iRetCode == ITK_ok && tManuForm != NULLTAG)
					{
						CHECK_ITK(iRetCode,TCTYPE_construct_create_input (tManuForm, &tCreateInputManuForm));
						if(iRetCode == ITK_ok && tCreateInputManuForm != NULLTAG)
						{
							/*set name*/
							CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputManuForm, "object_name", szManuForm.c_str()));
							CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputManuForm, "object_desc", ecrNumber));
							if(iRetCode == ITK_ok)
							{
								/*create object*/
								CHECK_ITK(iRetCode,TCTYPE_create_object (tCreateInputManuForm, &tManuCostForm));
								if(iRetCode == ITK_ok && tManuCostForm != NULLTAG)
								{
									/*create relation between form and ecr rev*/
									CHECK_ITK(iRetCode,GRM_create_relation(ecrRev,tManuCostForm,tFormRel,NULLTAG,&tCrtdManuRel));
									if(iRetCode == ITK_ok && tCrtdManuRel != NULLTAG)
									{
										/*save form*/
										CHECK_ITK(iRetCode,AOM_save (tManuCostForm));
										if(iRetCode == ITK_ok)
										{
											CHECK_ITK(iRetCode,AOM_refresh (tManuCostForm, 0));
											if(iRetCode == ITK_ok)
											{
												/*save relation*/
												CHECK_ITK(iRetCode,GRM_save_relation (tCrtdManuRel));
											}
										}	
									}
								}
							}
						}
					}
    
					/*create logistics form*/
					CHECK_ITK(iRetCode,TCTYPE_find_type ("Vf6_logistics","Form",&tLogForm));
					if(iRetCode == ITK_ok && tLogForm != NULLTAG)
					{
						CHECK_ITK(iRetCode,TCTYPE_construct_create_input (tLogForm, &tCreateInputLogForm));
						if(iRetCode == ITK_ok && tCreateInputLogForm != NULLTAG)
						{
							/*set name*/
							CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputLogForm, "object_name", szLogForm.c_str()));
							CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputLogForm, "object_desc", ecrNumber));
							if(iRetCode == ITK_ok)
							{
								/*create object*/
								CHECK_ITK(iRetCode,TCTYPE_create_object (tCreateInputLogForm, &tLogCostForm));
								if(iRetCode == ITK_ok && tLogCostForm != NULLTAG)
								{
									/*create relation between form and ecr rev*/
									CHECK_ITK(iRetCode,GRM_create_relation(ecrRev,tLogCostForm,tFormRel,NULLTAG,&tCrtdLogRel));
									if(iRetCode == ITK_ok && tCrtdLogRel != NULLTAG)
									{
										/*save form*/
										CHECK_ITK(iRetCode,AOM_save (tLogCostForm));
										if(iRetCode == ITK_ok)
										{
											CHECK_ITK(iRetCode,AOM_refresh (tLogCostForm, 0));
											if(iRetCode == ITK_ok)
											{
												/*save relation*/
												CHECK_ITK(iRetCode,GRM_save_relation (tCrtdLogRel));
											}
										}
									}
								}
							}
						}
					}

					/*create engineering form*/
					CHECK_ITK(iRetCode,TCTYPE_find_type ("Vf6_engineering","Form",&tEngForm));
					if(iRetCode == ITK_ok && tEngForm != NULLTAG)
					{
						CHECK_ITK(iRetCode,TCTYPE_construct_create_input (tEngForm, &tCreateInputEngForm));
						if(iRetCode == ITK_ok && tCreateInputEngForm != NULLTAG)
						{
							/*set name*/
							CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputEngForm, "object_name", szEngForm.c_str()));
							CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputEngForm, "object_desc", ecrNumber));
							if(iRetCode == ITK_ok)
							{
								/*create object*/
								CHECK_ITK(iRetCode,TCTYPE_create_object (tCreateInputEngForm, &tEngCostForm));
								if(iRetCode == ITK_ok && tEngCostForm != NULLTAG)
								{
									/*create relation between form and ecr rev*/
									CHECK_ITK(iRetCode,GRM_create_relation(ecrRev,tEngCostForm,tFormRel,NULLTAG,&tCrtdEngRel));
									if(iRetCode == ITK_ok && tCrtdEngRel != NULLTAG)
									{
										/*save form*/
										CHECK_ITK(iRetCode,AOM_save (tEngCostForm));
										if(iRetCode == ITK_ok)
										{
											CHECK_ITK(iRetCode,AOM_refresh (tEngCostForm, 0));
											if(iRetCode == ITK_ok)
											{
												/*save relation*/
												CHECK_ITK(iRetCode,GRM_save_relation (tCrtdEngRel));
											}
										}
									}
								}
							}
						}
					}
                        
					/*create purchasing form*/
					CHECK_ITK(iRetCode,TCTYPE_find_type ("Vf6_purchasing","Form",&tPurForm));
					if(iRetCode == ITK_ok && tPurForm != NULLTAG)
					{
						CHECK_ITK(iRetCode,TCTYPE_construct_create_input (tPurForm, &tCreateInputPurForm));
						if(iRetCode == ITK_ok && tCreateInputPurForm != NULLTAG)
						{
							/*set name*/
							CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputPurForm, "object_name", szPurcForm.c_str()));
							CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputPurForm, "object_desc", ecrNumber));
							if(iRetCode == ITK_ok)
							{
								/*create object*/
								CHECK_ITK(iRetCode,TCTYPE_create_object (tCreateInputPurForm, &tPurCostForm));
								if(iRetCode == ITK_ok && tPurCostForm != NULLTAG)
								{
									/*create relation between form and ecr rev*/
									CHECK_ITK(iRetCode,GRM_create_relation(ecrRev,tPurCostForm,tFormRel,NULLTAG,&tCrtdPurRel));
									if(iRetCode == ITK_ok && tCrtdPurRel != NULLTAG)
									{
										/*save form*/
										CHECK_ITK(iRetCode,AOM_save (tPurCostForm));
										if(iRetCode == ITK_ok)
										{
											CHECK_ITK(iRetCode,AOM_refresh (tPurCostForm, 0));
											if(iRetCode == ITK_ok)
											{
												/*save relation*/
												CHECK_ITK(iRetCode,GRM_save_relation (tCrtdPurRel));
											}
										}
									}
								}
							}
						}
					}
					else
					{
						TC_write_syslog("Cost forms already existed\n");
					}
				}
			}

			if (tManuCostForm && tPurCostForm && tLogCostForm && tEngCostForm && pcItem) 
			{
				std::vector<tag_t> costForms;
				costForms.push_back(tManuCostForm);costForms.push_back(tLogCostForm);costForms.push_back(tEngCostForm);costForms.push_back(tPurCostForm);
				copyCostFormsToCostObject(costForms, ptProp[iNum], pcItem, costRevs);
			}

			SAFE_MEM_free(pcItem);
			SAFE_MEM_free(pcRev);
		}
	}

	TC_write_syslog("\n[VF]cost revs num=%d", costRevs.size());
	//fixCostFormsName(costRevs);

	SAFE_SM_FREE(ecrNumber);

	TC_write_syslog("\n[VF] LEAVE %s", __FUNCTION__);
}

extern int AH_attach_cost_form(EPM_action_message_t msg)
{
    int			iRetCode			= ITK_ok;
    tag_t		tRootTask			= NULLTAG;
	char *arg_key			        = NULL;	
	char *arg_value	                = NULL;
	char *relation                  = NULL;
	char *secondRelation            = NULL;
	char *costFormRelation          = "Vf6_change_forms";
	char *changeType               = "Vf6_ECRRevision";
	const char *defaultRelation     = "Cm0HasProposal";
	const char *defaultSecondRelation = "CMHasProblemItem";
	
	TC_write_syslog("\n[VF] ENTER %s", __FUNCTION__);

	//TC_write_syslog("\n ENTER AH_attach_cost_form");
	int num_args = TC_number_of_arguments(msg.arguments);
	if (num_args > 0)
	{
		for(int i = 0; i < num_args; ++i)
		{
			CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &arg_key, &arg_value));
			if (iRetCode == ITK_ok )
			{
				if (tc_strcasecmp(arg_key, "relation") == 0 && arg_value != NULL)
				{
					relation = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(relation, arg_value);
				}
				else if (tc_strcasecmp(arg_key, "relation_create_cost_form_if_unique_partID") == 0 && arg_value != NULL)
				{
					secondRelation = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(secondRelation, arg_value);
				}
				else if (tc_strcasecmp(arg_key, "cost_form_relation") == 0 && arg_value != NULL)
				{
					costFormRelation = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(costFormRelation, arg_value);
				}
				else if (tc_strcasecmp(arg_key, "type") == 0 && arg_value != NULL)
				{
					changeType = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(changeType, arg_value);
				}
			}
		}
	}
	//TC_write_syslog("\n changeType=%s",changeType);
	//TC_write_syslog("\n costFormRelation=%s",costFormRelation);
	std::vector<std::string> relations = split_string(relation != NULL ? relation : defaultRelation, ',');
	std::vector<std::string> secondRelations = split_string(secondRelation != NULL ? secondRelation : defaultSecondRelation, ',');
	std::map<tag_t, int> itemsCreatedCostForms;
	
	SAFE_SM_FREE(relation);
	SAFE_SM_FREE(secondRelation);

	for (int i = 0; i < relations.size(); i++)
	{
		std::string relationTmp = relations[i];
		ltrim(relationTmp);
		rtrim(relationTmp);
		relations[i] = relationTmp;
	}

    /*getting root task*/
    CHECK_ITK(iRetCode,EPM_ask_root_task(msg.task,&tRootTask));
    if(tRootTask != 0)
    {
        int			iTgt				= 0;
        tag_t		*ptTgt				= NULL;

        /*getting target objects*/
        CHECK_ITK(iRetCode,EPM_ask_attachments(tRootTask,EPM_target_attachment,&iTgt,&ptTgt));
        for(int iInd = 0; iInd < iTgt; iInd++)
		{
            
            int		iOldForm		= 0;
            tag_t   tFormRel		= NULLTAG;
            
            GRM_relation_t   *ptOldForm		= NULL;
            char	*pcObjType		= NULL;

            CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptTgt[iInd],&pcObjType));
            /*for object of ECR Revision*/
            if(tc_strcmp(pcObjType,changeType) == 0 )
            {
				//TC_write_syslog("\n c11111111111");
                CHECK_ITK(iRetCode,GRM_find_relation_type (costFormRelation, &tFormRel));
                CHECK_ITK(iRetCode,GRM_list_secondary_objects(ptTgt[iInd],tFormRel,&iOldForm,&ptOldForm));
				if (iOldForm > 0)
				{
					TC_write_syslog("\n [VF] Deleting old cost forms ...");
					for (int iOldFormInx = 0; iOldFormInx < iOldForm; iOldFormInx++)
					{
						CHECK_ITK(iRetCode,GRM_delete_relation(ptOldForm[iOldFormInx].the_relation));
					}

					SAFE_SM_FREE(ptOldForm);
					iOldForm = 0;
					CHECK_ITK(iRetCode,GRM_list_secondary_objects(ptTgt[iInd],tFormRel,&iOldForm,&ptOldForm));
					TC_write_syslog("\n [VF] Deleting old cost complete.");
				}

                if(iRetCode == ITK_ok && iOldForm == 0)
                {
					//TC_write_syslog("\n 222222222222");
					createCostForms(ptTgt[iInd], tFormRel, relations, itemsCreatedCostForms, false);
					createCostForms(ptTgt[iInd], tFormRel, secondRelations, itemsCreatedCostForms, true);
                }
                else
                {
                    TC_write_syslog("Cost forms relation not found\n");
                }
            }
        }
    }

	TC_write_syslog("\n[VF] LEAVE %s", __FUNCTION__);
    return iRetCode;
}