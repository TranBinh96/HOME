/***************************************************************************************
File         : RH_check_unreleased_ItemRevision.cpp

Description  : To check whether any Item Revision in BOM which is not part of Solution
			   Items is not having valid release status.

Input        : None

Output       : None

Author       : Vinfast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Nov,26 2018     1.0         Kantesh			Initial Creation

*****************************************************************************************/
#include "Vinfast_Custom.h"
#include "cfm\cfm.h"

/*Declaration*/
logical checkForAssembly(tag_t tAssyRev, vector<string> szItemRev, char* pcReqStatus, char* pcSolObjType, const char* revisionRule);

logical checkStatus(char* pcStatus, const char* pcReqStatus)
{
	int		iRetCode = ITK_ok;
	char* pcReq = NULL;
	char* pcPart = NULL;
	char* pcStsList = NULL;
	logical lValid = true;

	if (tc_strlen(pcReqStatus) > 0)
	{
		vector<string> ReqSts;
		vector<string> PartSts;

		pcStsList = (char*)MEM_alloc(((int)tc_strlen(pcReqStatus) + 1) * sizeof(char));
		tc_strcpy(pcStsList, pcReqStatus);

		pcReq = tc_strtok(pcStsList, ",");
		while (pcReq != NULL)
		{
			ReqSts.push_back(pcReq);
			pcReq = tc_strtok(NULL, ",");
		}

		pcPart = tc_strtok(pcStatus, ",");
		while (pcPart != NULL)
		{
			PartSts.push_back(pcPart);
			pcPart = tc_strtok(NULL, ",");
		}

		for (int iCnt = 0; iCnt < ReqSts.size(); iCnt++)
		{
			ltrim(ReqSts[iCnt]);
			rtrim(ReqSts[iCnt]);
			for (int iSts = 0; iSts < PartSts.size(); iSts++)
			{
				ltrim(PartSts[iSts]);
				rtrim(PartSts[iSts]);
				if (tc_strcmp(ReqSts[iCnt].c_str(), PartSts[iSts].c_str()) == 0)
				{
					lValid = true;
					break;
				}
				else
				{
					lValid = false;
				}
			}

			if (lValid == true)
			{
				break;
			}
		}

		SAFE_MEM_free(pcStsList);
	}
	
	return lValid;
}

logical checkForSubAssy(tag_t tTopLine, vector<string> szItemRev, char* pcReqStatus, char* pcSolObjType, char *ecnNewPartStatus)
{
	int		iRetCode = ITK_ok;
	int		iLines = 0;
	int		bl_item = 0;
	int		bl_revision = 0;
	int		bl_status = 0;
	int		bl_itemType = 0;
	tag_t	tChildLine = NULLTAG;
	tag_t* ptLines = NULL;
	tag_t	tItemRevision = NULLTAG;
	char* pcStatus = NULL;
	char* pcChild = NULL;
	char* pcChildRev = NULL;
	char* pcType = NULL;
	tag_t   tParentRev = NULLTAG;
	string	szChild = "";
	char	ErrorMessage[2048] = "";
	logical lReleased = true;
	Teamcenter::scoped_smptr<char> pcParent;
	Teamcenter::scoped_smptr<char> pcParentRev;

	std::string strPcStatus;

	CHECK_ITK(iRetCode, BOM_line_ask_all_child_lines(tTopLine, &iLines, &ptLines));
	CHECK_ITK(iRetCode, AOM_ask_value_tag(tTopLine, "bl_line_object", &tParentRev));
	CHECK_ITK(iRetCode, AOM_ask_value_string(tParentRev, "item_id", &pcParent));
	CHECK_ITK(iRetCode, AOM_ask_value_string(tParentRev, "item_revision_id", &pcParentRev));
	if (pcReqStatus == NULL) pcReqStatus = "";
	std::string validChildrenStatusesStr(pcReqStatus);
	if (tc_strlen(pcReqStatus) == 0)
	{
		char errorMsg[1024] = { '\0' };
		std::string latestValidStatus = getLatestValidStatus("", tParentRev, "true");
		if (latestValidStatus.size() == 0)
		{
			latestValidStatus = ecnNewPartStatus;
		}
		TC_write_syslog("\n[TC] latestValidStatus=%s", latestValidStatus.c_str());
		validChildrenStatusesStr = getValidChildrenStatusesStr(latestValidStatus);
		TC_write_syslog("\n[TC] validChildrenStatusesStr=%s", validChildrenStatusesStr.c_str());
	}

	for (int iIter2 = 0; iIter2 < iLines; iIter2++)
	{
		tChildLine = ptLines[iIter2];

		CHECK_ITK(iRetCode, BOM_line_look_up_attribute("bl_item_item_id", &bl_item));
		CHECK_ITK(iRetCode, BOM_line_look_up_attribute("bl_rev_item_revision_id", &bl_revision));
		CHECK_ITK(iRetCode, BOM_line_look_up_attribute("bl_rev_release_status_list", &bl_status));

		/*getting item id, rev id and release status of each bom line*/
		CHECK_ITK(iRetCode, BOM_line_ask_attribute_string(tChildLine, bl_item, &pcChild));
		CHECK_ITK(iRetCode, BOM_line_ask_attribute_string(tChildLine, bl_revision, &pcChildRev));

		tag_t* statusList = NULL;
		tag_t tChildRev = NULLTAG;
		int statusListCount = -1;

		CHECK_ITK(iRetCode, AOM_ask_value_tag(tChildLine, "bl_line_object", &tChildRev));
		CHECK_ITK(iRetCode, AOM_ask_value_tags(tChildRev, "release_status_list", &statusListCount, &statusList));

		strPcStatus.clear();
		for (int inx = 0; inx < statusListCount; inx++)
		{
			char* szStatusTmp = NULL;
			CHECK_ITK(iRetCode, RELSTAT_ask_release_status_type(statusList[inx], &szStatusTmp));
			if (strPcStatus.empty() == false) strPcStatus.append(",");
			strPcStatus.append(szStatusTmp);

			SAFE_SM_FREE(szStatusTmp);
		}

		SAFE_SM_FREE(statusList);


		CHECK_ITK(iRetCode, AOM_ask_value_tag(tChildLine, "bl_line_object", &tItemRevision));
		CHECK_ITK(iRetCode, WSOM_ask_object_type2(tItemRevision, &pcType));
		if (iRetCode != ITK_ok)
		{
			TC_write_syslog("\n[VF] ERROR: Failed to get Item Type for %s. RetCode is %d", pcChild, iRetCode);
		}

		szChild.assign(pcChild);
		//szChild.append("/");
		//szChild.append(pcChildRev);
		TC_write_syslog("\n[VF] Checking line %s", szChild.c_str());
		bool isChildInSolutions = false;
		for (const auto& itemRevStr : szItemRev)
		{
			if (itemRevStr.find(szChild) != string::npos)
			{
				isChildInSolutions = true;
				break;
			}
		}

		/*if relase status of bom line is equal to status specified in handler argument*/
		if (tc_strcmp(pcType, pcSolObjType) == 0 && isChildInSolutions == false)
		{
			logical lValid = true;
			lValid = checkStatus((char*)strPcStatus.c_str(), validChildrenStatusesStr.c_str());

			if (lValid == false)
			{
				lReleased = false;
				sprintf(ErrorMessage, "Assembly part \"%s/%s\" cannot be released as it's child \"%s/%s\" is not having valid Release status.", pcParent.getString(), pcParentRev.getString(), pcChild, pcChildRev);
				EMH_store_error_s1(EMH_severity_user_error, CHECK_STATUS, ErrorMessage);
				//break;
			}
		}

		if (lReleased == true)
		{
			//lReleased = checkForSubAssy(tChildLine,szItemRev,pcReqStatus,pcSolObjType);
		}

		MEM_free(pcChild);
		MEM_free(pcType);
		MEM_free(pcChildRev);
		MEM_free(pcStatus);
	}
	return lReleased;
}

logical checkForAssembly(tag_t tAssyRev, vector<string> szItemRev, char* pcReqStatus, char* pcSolObjType, const char* revisionRule, char *ecnNewPartStatus)
{
	int		iRetCode = ITK_ok;
	int		iLines = 0;
	int		iBvr = 0;
	tag_t* ptBvr = NULL;
	tag_t* ptLines = NULL;
	tag_t	tItem = NULLTAG;
	tag_t	tWindow = NULLTAG;
	tag_t	tTopLine = NULLTAG;
	tag_t	tChildRev = NULLTAG;
	logical lReleased = true;

	CHECK_ITK(iRetCode, ITEM_rev_list_bom_view_revs(tAssyRev, &iBvr, &ptBvr));
	/*checking if solution item revision is an assembly*/
	if (iBvr > 0)
	{
		/*getting all children of the assembly and bom line property of each*/
		CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(tAssyRev, &tItem));
		CHECK_ITK(iRetCode, BOM_create_window(&tWindow));

		CHECK_ITK(iRetCode, BOM_set_window_top_line(tWindow, tItem, tAssyRev, NULLTAG, &tTopLine));
		tag_t revRule = NULLTAG;
		CHECK_ITK(iRetCode, CFM_find(revisionRule, &revRule));
		if (revRule != NULL)
		{
			CHECK_ITK(iRetCode, BOM_set_window_config_rule(tWindow, revRule));
		}
		else
		{
			TC_write_syslog("[VF] WARNING: Cannot find rev rule %s.", revisionRule);
		}
		lReleased = checkForSubAssy(tTopLine, szItemRev, pcReqStatus, pcSolObjType, ecnNewPartStatus);
	}
	return lReleased;
}


EPM_decision_t RH_Check_unreleaseed_ItemRevision(EPM_rule_message_t msg)
{
	int			iRetCode = ITK_ok;
	int			iNumArg = 0;
	tag_t		tRootTask = NULLTAG;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* pcReqStatus = NULL;
	char* pcSolObjType = NULL;
	char* pcPriObjType = NULL;
	char* pcRelation = NULL;
	char		ErrorMessage[2048] = "";

	EPM_decision_t decision = EPM_go;
	vector<string> szItemRev;

	/*getting number of arguments*/
	iNumArg = TC_number_of_arguments(msg.arguments);

	if (iNumArg > 0)
	{
		for (int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue);
			/*for argument status*/
			if (tc_strcmp(pcFlag, "status") == 0 && pcValue != NULL)
			{
				pcReqStatus = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcReqStatus, pcValue);
			}
			if (tc_strcmp(pcFlag, "type") == 0 && pcValue != NULL)
			{
				pcSolObjType = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcSolObjType, pcValue);
			}
			if (tc_strcmp(pcFlag, "primary") == 0 && pcValue != NULL)
			{
				pcPriObjType = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcPriObjType, pcValue);
			}
			if (tc_strcmp(pcFlag, "relation") == 0 && pcValue != NULL)
			{
				pcRelation = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcRelation, pcValue);
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
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int			iTgt = 0;
		int         iPrefCount = 0;
		char* revisionRule = NULL;
		tag_t* ptTgt = NULL;

		// get rev rule
		//CHECK_ITK(iRetCode,PREF_set_search_scope( TC_preference_site )); //@SKIP_DEPRECATED - WILL REMOVED
		const char* REV_RULE_PREF_NAME = "VINF_Rev_Rule_for_RH_check_unreleased_ItemRevision";
		if (iRetCode == ITK_ok)
		{
			CHECK_ITK(iRetCode, PREF_ask_value_count(REV_RULE_PREF_NAME, &iPrefCount));
		}

		if ((iRetCode == ITK_ok) && iPrefCount != 0)
		{
			CHECK_ITK(iRetCode, PREF_ask_char_value(REV_RULE_PREF_NAME, 0, &revisionRule));
		}
		TC_write_syslog("\n[vf]revisionRule=%s\n", revisionRule);
		/*getting target objects*/
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		for (int iInd = 0; iInd < iTgt; iInd++)
		{
			int		iSolItems = 0;
			tag_t* ptSol = NULL;
			tag_t	tRelation = NULLTAG;
			char* pcObjType = NULL;

			CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcObjType));

			/*generic - Primary and Relation read from argument*/
			if (tc_strcmp(pcObjType, pcPriObjType) == 0)
			{

				char* ecnNewPartStatus = NULL;
				if (tc_strcmp(pcObjType, "Vf6_ECNRevision") == 0 || tc_strcmp(pcObjType, "Vf6_ECRRevision") == 0)
				{
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_new_parts_status", &ecnNewPartStatus));
				}

				/*getting solution items*/
				CHECK_ITK(iRetCode, GRM_find_relation_type(pcRelation, &tRelation));
				if (tRelation != 0)
				{
					CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[iInd], tRelation, &iSolItems, &ptSol));
					for (int iCnt = 0; iCnt < iSolItems; iCnt++)
					{
						char* pcItemRev = NULL;

						CHECK_ITK(iRetCode, WSOM_ask_id_string(ptSol[iCnt], &pcItemRev));
						szItemRev.push_back(pcItemRev);
					}

					/*iterating through tAssyRev vector*/
					for (int iIter1 = 0; iIter1 < iSolItems; iIter1++)
					{
						logical		lReleased = true;

						lReleased = checkForAssembly(ptSol[iIter1], szItemRev, pcReqStatus, pcSolObjType, revisionRule, tc_strlen(ecnNewPartStatus) > 0 ? ecnNewPartStatus : "");
						if (lReleased == false)
						{
							decision = EPM_nogo;
						}
					}

				}

				SAFE_SM_FREE(ecnNewPartStatus);
			}
			SAFE_SM_FREE(revisionRule);
			SAFE_SM_FREE(pcObjType);
		}
	}

	SAFE_MEM_free(pcReqStatus);
	SAFE_SM_FREE(pcPriObjType);
	SAFE_SM_FREE(pcRelation);
	SAFE_MEM_free(pcSolObjType);
	return decision;
}