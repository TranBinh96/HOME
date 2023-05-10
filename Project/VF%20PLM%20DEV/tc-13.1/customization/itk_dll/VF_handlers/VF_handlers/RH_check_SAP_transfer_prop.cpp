/***************************************************************************************
File         : RH_check_SAP_transfer_prop.cpp

Description  : To check whether any properties of Item Revision in BOM is filled or not
  
Input        : None
                        
Output       : None

Author       : Vinfast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Nov,26 2018     1.0         Kantesh			Initial Creation

*****************************************************************************************/
#include "Vinfast_Custom.h"

logical checkSolProperties(tag_t tItemRev, char *pcSapPlant)
{
	int		iRetCode				= ITK_ok;
	int		iPlantCount				= 0;
	char	*pcSolType				= NULL;
	char	*pcMatType				= NULL;
	char	*pcTraceablePart		= NULL;
	char	*pcCarMatType			= NULL;
	char	*pcCarTraceablePart		= NULL;
	char	*pcItemId				= NULL;
	char	*pcRevId				= NULL;
	tag_t	tPlantRel				= NULLTAG;
	tag_t	Item					= NULLTAG;
	tag_t	*ptPlantForm			= NULL;
	char	ErrorMessage [2048]		= "";
	logical lProcess				= true;

	CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(tItemRev , &Item ));
	CHECK_ITK(iRetCode, ITEM_ask_id2(Item,&pcItemId));
	CHECK_ITK(iRetCode, ITEM_ask_rev_id2(tItemRev,&pcRevId));
	CHECK_ITK(iRetCode, GRM_find_relation_type("VF3_plant_rel",&tPlantRel));
	CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(Item,tPlantRel,&iPlantCount,&ptPlantForm));
	CHECK_ITK(iRetCode,WSOM_ask_object_type2(tItemRev,&pcSolType));

	if(iPlantCount > 0)
	{
		for(int iForm = 0; iForm < iPlantCount; iForm++)
		{
			char	*pcMake_Buy				= NULL;
			char	*pcPPAP					= NULL;
			char	*pcPlantCode			= NULL;
			logical  lPlantCode				= false;		

			CHECK_ITK(iRetCode, AOM_ask_value_string(ptPlantForm[iForm] , "vf3_plant" , &pcPlantCode));
			if(tc_strcmp(pcSapPlant,pcPlantCode) == 0)
			{
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptPlantForm[iForm] , "vf3_ppap" , &pcPPAP));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptPlantForm[iForm] , "vf3_make_buy" , &pcMake_Buy));
				if(tc_strcmp(pcPPAP,"") == 0)
				{
					lProcess = false;
					sprintf( ErrorMessage, "Please enter PPAP attribute value for %s to continue with the workflow.",pcItemId);
					EMH_store_error_s1(EMH_severity_user_error, CHECK_PLANT_PPAP, ErrorMessage);
					break;
				}
				if(tc_strcmp(pcMake_Buy,"") == 0)
				{
					lProcess = false;
					sprintf( ErrorMessage, "Please enter Make/Buy attribute value for %s to continue with the workflow.",pcItemId);
					EMH_store_error_s1(EMH_severity_user_error, CHECK_PLANT_MAKE_BUY, ErrorMessage);
					break;
				}
				lPlantCode = true;
				break;
			}
			
			if(lPlantCode == false || tc_strcmp(pcPlantCode,"") == 0)
			{
				lProcess = false;
				sprintf( ErrorMessage, "Please enter valid Plant value for %s to continue with the workflow.",pcItemId);
				EMH_store_error_s1(EMH_severity_user_error, CHECK_PLANT_CODE, ErrorMessage);
				break;
			}
			SAFE_MEM_free(pcPPAP);
			SAFE_MEM_free(pcPlantCode);
			SAFE_MEM_free(pcMake_Buy);
		}
	}
	else
	{
		sprintf( ErrorMessage, "Please confirm Plant Form is available for %s and required attributes are filled.",pcItemId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_PLANT_FORM, ErrorMessage);
	}

	if(tc_strcmp(pcSolType,"VF3_Scooter_partRevision") == 0 || tc_strcmp(pcSolType,"VF3_me_scooter_Revision") == 0 
		|| tc_strcmp(pcSolType,"VF3_manuf_partRevision") == 0)
	{
		CHECK_ITK(iRetCode, AOM_ask_value_string(tItemRev , "vf3_material_type" , &pcMatType));
		CHECK_ITK(iRetCode, AOM_ask_value_string(tItemRev , "vf3_traceable_part_index" , &pcTraceablePart));
	}

	if(tc_strcmp(pcSolType,"VF3_car_partRevision") == 0 )
	{
		CHECK_ITK(iRetCode, AOM_ask_value_string(tItemRev , "vf3_material_type_s" , &pcCarMatType));
		CHECK_ITK(iRetCode, AOM_ask_value_string(tItemRev , "vf3_traceable_part_index_s" , &pcCarTraceablePart));
	}

	if(tc_strcmp(pcMatType,"") == 0 || tc_strcmp(pcCarMatType,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please enter Material Type attribute value for %s/%s to continue with the workflow.",pcItemId,pcRevId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_MATERIAL_TYPE, ErrorMessage);
	}

	if(tc_strcmp(pcTraceablePart,"") == 0 || tc_strcmp(pcCarTraceablePart,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please enter Traceable Part Number attribute value for %s/%s to continue with the workflow.",pcItemId,pcRevId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_TRACEABLE_PART, ErrorMessage);
	}

	SAFE_MEM_free(pcTraceablePart);
	SAFE_MEM_free(pcMatType);
	SAFE_MEM_free(pcCarTraceablePart);
	SAFE_MEM_free(pcCarMatType);
	SAFE_MEM_free(pcItemId);
	SAFE_MEM_free(pcRevId);
	SAFE_MEM_free(pcSolType);

	return lProcess;
}

logical checkForChangeItem(tag_t tChangeItem)
{
	int		iRetCode		= ITK_ok;
	char	*pcSapPlant		= NULL;
	char	*pcChgReason	= NULL;
	char	*pcChgType		= NULL;
	char	*pcDispCode		= NULL;
	char	*pcModelYear	= NULL;
	char	*pcAction		= NULL;
	char	*pcDesc			= NULL;
	char	*pcItemId		= NULL;
	logical	lProcess		= true;
	char	ErrorMessage [2048]		= "";

	CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem , "vf6_sap_plant" , &pcSapPlant));
	CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem , "vf6_change_reason" , &pcChgReason));
	CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem , "vf6_change_type" , &pcChgType));
	CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem , "vf6_disposal_code" , &pcDispCode));
	CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem , "vf6_model_year" , &pcModelYear));
	CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem , "vf6_action" , &pcAction));
	CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem , "object_desc" , &pcDesc));

	CHECK_ITK(iRetCode,ITEM_ask_id2(tChangeItem,&pcItemId));

	if(tc_strcmp(pcSapPlant,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please select valid SAP Plant for %s to continue with the workflow.",pcItemId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_SAP_PLANT, ErrorMessage);
	}
	if(tc_strcmp(pcChgReason,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please enter Change Reason for %s to continue with the workflow.",pcItemId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_CHANGE_REASON, ErrorMessage);
	}
	if(tc_strcmp(pcChgType,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please enter Change Type for %s to continue with the workflow.",pcItemId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_CHANGE_TYPE, ErrorMessage);
	}
	if(tc_strcmp(pcDispCode,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please enter Disposal Code attribute value for %s to continue with the workflow.",pcItemId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_DISP_CODE, ErrorMessage);
	}
	if(tc_strcmp(pcModelYear,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please enter Model Year attribute value for %s to continue with the workflow.",pcItemId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_MODEL_YEAR, ErrorMessage);
	}
	if(tc_strcmp(pcAction,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please enter Action for %s to continue with the workflow.",pcItemId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_ACTION, ErrorMessage);
	}
	if(tc_strcmp(pcDesc,"") == 0)
	{
		lProcess = false;
		sprintf( ErrorMessage, "Please enter Description for %s to continue with the workflow.",pcItemId);
		EMH_store_error_s1(EMH_severity_user_error, CHECK_DISC, ErrorMessage);
	}

	SAFE_MEM_free(pcSapPlant);
	SAFE_MEM_free(pcChgReason);
	SAFE_MEM_free(pcChgType);
	SAFE_MEM_free(pcDispCode);
	SAFE_MEM_free(pcModelYear);
	SAFE_MEM_free(pcAction);
	SAFE_MEM_free(pcDesc);

	return lProcess;
}

logical checkForSubAssy(tag_t tTopLine, char *pcSapPlant)
{
	int		iRetCode			= ITK_ok;
	int		iLines				= 0;
	tag_t	tChildLine			= NULLTAG;
	tag_t	tItemRevision		= NULLTAG;
	tag_t	*ptLines			= NULL;
	logical	lProcess			= true;

	CHECK_ITK(iRetCode,BOM_line_ask_all_child_lines(tTopLine,&iLines,&ptLines));

	for(int iIter2 = 0; iIter2 < iLines; iIter2++)
	{
		tChildLine = ptLines[iIter2];

		CHECK_ITK(iRetCode, AOM_ask_value_tag(tChildLine,"bl_line_object",&tItemRevision));

		lProcess = checkSolProperties(tItemRevision,pcSapPlant);

		if(lProcess == true)
		{
			lProcess = checkForSubAssy(tChildLine,pcSapPlant);
		}
	}
	return lProcess;
}

logical checkForAssembly(tag_t tAssyRev, char *pcSapPlant)
{
	int		iRetCode	= ITK_ok;
	int		iBvr		= 0;
	tag_t	*ptBvr		= NULL;
	tag_t	tItem		= NULLTAG;
	tag_t	tWindow		= NULLTAG;
	tag_t	tTopLine	= NULLTAG;
	logical lProcess	= true;

	CHECK_ITK(iRetCode,ITEM_rev_list_bom_view_revs(tAssyRev,&iBvr,&ptBvr));
	/*checking if solution item revision is an assembly*/
	if(iBvr > 0)
	{
		/*getting all children of the assembly and bom line property of each*/
		CHECK_ITK(iRetCode,ITEM_ask_item_of_rev(tAssyRev,&tItem));
		CHECK_ITK(iRetCode,BOM_create_window(&tWindow));
		CHECK_ITK(iRetCode,BOM_set_window_top_line(tWindow, tItem, tAssyRev, NULLTAG, &tTopLine));

		lProcess = checkForSubAssy(tTopLine,pcSapPlant);
	}
	return lProcess;
}

EPM_decision_t RH_check_SAP_transfer_prop(EPM_rule_message_t msg)
{
	int			iRetCode			= ITK_ok;
	tag_t		tRootTask			= NULLTAG;
	char		ErrorMessage [2048] = "";
	logical		lProcess			= true;
	EPM_decision_t decision			= EPM_go;

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
			int		iSol			= 0;
			tag_t	tItem			= NULLTAG;
			tag_t	tChangeItem		= NULLTAG;
			tag_t	tSolType		= NULLTAG;
			tag_t	*ptSol			= NULL;
			char	*pcObjType		= NULL;
			char	*pcSapPlant		= NULL;

			CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptTgt[iInd],&pcObjType));
			/*for object of ECN Revision*/
			if(tc_strcmp(pcObjType,"Vf6_ECNRevision") == 0 || tc_strcmp(pcObjType,"Vf6_MCNRevision") == 0)
			{
				CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iInd] , &tChangeItem ));
				CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem , "vf6_sap_plant" , &pcSapPlant));
				lProcess = checkForChangeItem(tChangeItem);
				if(lProcess == false)
				{
					decision = EPM_nogo;
					return decision;
				}

				CHECK_ITK(iRetCode,GRM_find_relation_type("CMHasSolutionItem",&tSolType));
				CHECK_ITK(iRetCode,GRM_list_secondary_objects_only(ptTgt[iInd],tSolType,&iSol,&ptSol));
				for(int iNum = 0; iNum < iSol; iNum++)
				{
					char	*pcSolType				= NULL;

					CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptSol[iNum],&pcSolType));
					if(tc_strcmp(pcSolType,"VF3_Scooter_partRevision") == 0 || tc_strcmp(pcSolType,"VF3_car_partRevision") == 0 
						|| tc_strcmp(pcSolType,"VF3_manuf_partRevision") == 0 || tc_strcmp(pcSolType,"VF3_me_scooterRevision") == 0)
					{
						lProcess = checkSolProperties(ptSol[iNum],pcSapPlant);
						if(lProcess == false)
						{
							decision = EPM_nogo;
							return decision;
						}
						else
						{
							lProcess = checkForAssembly(ptSol[iNum],pcSapPlant);
							if(lProcess == false)
							{
								decision = EPM_nogo;
								return decision;
							}
						}
					}
				}
			}
			SAFE_MEM_free(pcObjType);
			SAFE_MEM_free(pcSapPlant);
		}
	}

	return decision;
}