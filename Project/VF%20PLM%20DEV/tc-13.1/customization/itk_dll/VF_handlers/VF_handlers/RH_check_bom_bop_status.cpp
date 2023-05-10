/***************************************************************************************
File         : RH_check_unreleased_ItemRevision.cpp

Description  : To check whether any Item Revision in MBOM and BOP which is not part of Solution 
			   Items is not having valid release status. 
  
Input        : None
						
Output       : None

Author       : Vinfast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Mar,01 2019     1.0         Kantesh			Initial Creation

*****************************************************************************************/
#include "Vinfast_Custom.h"

/*Declaration*/
logical checkForAssemblyStructure(tag_t tAssyRev, vector<string> szItemRev, char *pcReqStatus, char *pcSolObjType, char *pcRevRule);

logical checkChildStatus(char *pcStatus, char *pcReqStatus)
{
	int		iRetCode	= ITK_ok;
	char	*pcReq		= NULL;
	char	*pcPart		= NULL;
	char	*pcStsList	= NULL;
	logical lValid		= true;

	vector<string> ReqSts;
	vector<string> PartSts;

	pcStsList = (char*) MEM_alloc(( (int)tc_strlen(pcReqStatus) + 1) * sizeof(char));
	tc_strcpy(pcStsList,pcReqStatus);

	pcReq = tc_strtok (pcStsList,",");
	while (pcReq != NULL)
	{
		ReqSts.push_back(pcReq);
		pcReq = tc_strtok(NULL,",");
	}

	pcPart = tc_strtok (pcStatus,",");
	while (pcPart != NULL)
	{
		PartSts.push_back(pcPart);
		pcPart = tc_strtok(NULL,",");
	}

	for(int iCnt = 0; iCnt < ReqSts.size(); iCnt++)
	{
		ltrim(ReqSts[iCnt]);
		rtrim(ReqSts[iCnt]);
		for(int iSts = 0; iSts < PartSts.size(); iSts++)
		{
			ltrim(PartSts[iSts]);
			rtrim(PartSts[iSts]);
			if(tc_strcmp(ReqSts[iCnt].c_str(),PartSts[iSts].c_str()) == 0)
			{
				lValid = true;
				break;
			}
			else
			{
				lValid = false;
			}
		}

		if(lValid == true)
		{
			break;
		}
	}

	SAFE_MEM_free(pcStsList);
	return lValid;
}

logical checkForSubAssyStructure(tag_t tTopLine, vector<string> szItemRev, char *pcReqStatus)
{
	int		iRetCode			= ITK_ok;
	int		iLines				= 0;
	int		bl_item				= 0;
	int		bl_revision			= 0;
	int		bl_status			= 0;
	int		bl_itemType			= 0;
	tag_t	tChildLine			= NULLTAG;
	tag_t	*ptLines			= NULL;
	tag_t	tItemRevision		= NULLTAG;
	char	*pcChild			= NULL;
	char	*pcChildRev			= NULL;
	char	*pcType				= NULL;
	string	szChild				= "";
	char	ErrorMessage [2048] = "";
	logical lReleased			= true;
	vector<string>::iterator Iter;
	std::string strPcStatus;

	CHECK_ITK(iRetCode,BOM_line_ask_all_child_lines(tTopLine,&iLines,&ptLines));

	for(int iIter2 = 0; iIter2 < iLines; iIter2++)
	{
		tChildLine = ptLines[iIter2];

		CHECK_ITK(iRetCode,BOM_line_look_up_attribute( "bl_item_item_id", &bl_item));
		CHECK_ITK(iRetCode,BOM_line_look_up_attribute( "bl_rev_item_revision_id", &bl_revision));
		CHECK_ITK(iRetCode,BOM_line_look_up_attribute( "bl_rev_release_status_list", &bl_status));

		/*getting item id, rev id and release status of each bom line*/
		CHECK_ITK(iRetCode,BOM_line_ask_attribute_string(tChildLine,bl_item,&pcChild));
		CHECK_ITK(iRetCode,BOM_line_ask_attribute_string(tChildLine,bl_revision,&pcChildRev));
		
		tag_t *statusList = NULL;
		tag_t tChildRev = NULLTAG;
		int statusListCount = -1;

		CHECK_ITK(iRetCode,AOM_ask_value_tag(tChildLine,"bl_line_object",&tChildRev));
		CHECK_ITK(iRetCode,AOM_ask_value_tags(tChildRev, "release_status_list", &statusListCount, &statusList));
		
		for (int inx = 0; inx < statusListCount; inx++)
		{
			char *szStatusTmp = NULL;
			CHECK_ITK(iRetCode,RELSTAT_ask_release_status_type(statusList[inx], &szStatusTmp));
			if (strPcStatus.empty() == false) strPcStatus.append(",");
			strPcStatus.append(szStatusTmp);

			SAFE_SM_FREE(szStatusTmp);
		}

		SAFE_SM_FREE(statusList);

		CHECK_ITK(iRetCode,AOM_ask_value_tag(tChildLine,"bl_line_object",&tItemRevision));
		CHECK_ITK(iRetCode,WSOM_ask_object_type2(tItemRevision,&pcType));

		szChild.assign(pcChild);
		szChild.append("/");
		szChild.append(pcChildRev);

		Iter = find (szItemRev.begin(), szItemRev.end(), szChild);

		/*if relase status of bom line is equal to status specified in handler argument*/
		if(tc_strcmp(pcType,"VF3_car_partRevision") != 0 && Iter == szItemRev.end())
		{
			logical lValid = true;
			lValid = checkChildStatus((char*)strPcStatus.c_str(),pcReqStatus);

			if(lValid == false)
			{
				lReleased = false;
				sprintf( ErrorMessage, "%s/%s is not having PR or PPR Release status. All child parts must have PR or PPR Release Status to continue with the workflow",pcChild,pcChildRev);
				EMH_store_error_s1(EMH_severity_user_error, CHECK_STATUS, ErrorMessage);
				break;
			}
		}

		if(lReleased == true)
		{
			lReleased = checkForSubAssyStructure(tChildLine,szItemRev,pcReqStatus);
		}

		MEM_free(pcChild);
		MEM_free(pcChildRev);
	}
	return lReleased;
}

logical checkForAssemblyStructure(tag_t tAssyRev, vector<string> szItemRev, char *pcReqStatus, char *pcRevRule)
{
	int		iRetCode	= ITK_ok;
	int		iLines		= 0;
	int		iBvr		= 0;
	tag_t	*ptBvr		= NULL;
	tag_t	*ptLines	= NULL;
	tag_t	tItem		= NULLTAG;
	tag_t	tWindow		= NULLTAG;
	tag_t	tTopLine	= NULLTAG;
	tag_t	tChildRev	= NULLTAG;
	tag_t   tRule		= NULLTAG;
	logical lReleased	= true;

	CHECK_ITK(iRetCode,ITEM_rev_list_bom_view_revs(tAssyRev,&iBvr,&ptBvr));
	/*checking if solution item revision is an assembly*/
	if(iBvr > 0)
	{
		/*getting all children of the assembly and bom line property of each*/
		CHECK_ITK(iRetCode,ITEM_ask_item_of_rev(tAssyRev,&tItem));
		CHECK_ITK(iRetCode,BOM_create_window(&tWindow));
		CHECK_ITK(iRetCode,BOM_set_window_top_line(tWindow, tItem, tAssyRev, NULLTAG, &tTopLine));
		
		//added revision rule
		CHECK_ITK(iRetCode,CFM_find(pcRevRule, &tRule));
		if(tRule != NULLTAG)
		{
			CHECK_ITK(iRetCode,BOM_set_window_config_rule(tWindow, tRule));
		}
		
		
		lReleased = checkForSubAssyStructure(tTopLine,szItemRev,pcReqStatus);
	}
	return lReleased;
}


EPM_decision_t RH_check_bom_bop_status(EPM_rule_message_t msg)
{
	int			iRetCode			= ITK_ok;
	int			iNumArg				= 0;
	tag_t		tRootTask			= NULLTAG;
	char		*pcFlag				= NULL;	
	char		*pcValue			= NULL;	
	char		*pcReqStatus		= NULL;
	char		*pcRevisionRule		= NULL;
	char		ErrorMessage [2048] = "";
	
	EPM_decision_t decision			= EPM_go;
	vector<string> szItemRev;

	/*getting number of arguments*/
	iNumArg = TC_number_of_arguments(msg.arguments);

	if(iNumArg > 0)
	{
		for(int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue );
			/*for argument status*/
			if( tc_strcmp(pcFlag,"status") == 0 && pcValue != NULL)
			{
				pcReqStatus = (char*) MEM_alloc(( (int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy( pcReqStatus, pcValue);
			}
			//For Revision Rule
			if( tc_strcmp(pcFlag,"revisionRule") == 0 && pcValue != NULL)
			{
				pcRevisionRule = (char*) MEM_alloc(( (int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy( pcRevisionRule, pcValue);
			}
			

		}
		MEM_free(pcValue);
		MEM_free(pcFlag);
	}
	else
	{
		iRetCode = EPM_wrong_number_of_arguments;  
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
			int		iSolItems	= 0;
			tag_t	*ptSol		= NULL;
			tag_t	tRelation	= NULLTAG;
			char	*pcObjType	= NULL;

			CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptTgt[iInd],&pcObjType));
			/*for object of ECR Revision*/
			if(tc_strcmp(pcObjType,"Vf6_MCNRevision") == 0)
			{
				/*getting solution items*/
				CHECK_ITK(iRetCode,GRM_find_relation_type("EC_solution_item_rel",&tRelation));
				if(tRelation != 0)
				{
					CHECK_ITK(iRetCode,GRM_list_secondary_objects_only(ptTgt[iInd],tRelation,&iSolItems,&ptSol));
					for(int iCnt = 0; iCnt < iSolItems; iCnt++)
					{
						char	*pcItemRev	= NULL;

						CHECK_ITK(iRetCode,WSOM_ask_id_string(ptSol[iCnt],&pcItemRev));
						szItemRev.push_back(pcItemRev);
					}

					/*iterating through tAssyRev vector*/
					for(int iIter1 = 0; iIter1 < iSolItems; iIter1++) 
					{
						logical		lReleased = true;

						lReleased = checkForAssemblyStructure(ptSol[iIter1],szItemRev,pcReqStatus,pcRevisionRule);
						if(lReleased == false)
						{
							decision = EPM_nogo;
							return decision;
						}
					}
					
				}
			}
			MEM_free(pcObjType);
		}
	}
	return decision;
}