/*******************************************************************************
File         : AH_add_release_sts_sol_items.cpp

Description  : To add prev revision status on latest revision

Input        : None

Output       : None

Author       : Siemens

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Nov,12 2019     1.0         Kantesh		Initial Creation
Feb,25 2020     1.1         Nguyen      Added logging and added logic that get only allowed statuses (in sequence) in previous revision.

*******************************************************************************/
#include "Vinfast_Custom.h"

#include <map>

extern int AH_add_release_sts_sol_items_scooter(EPM_action_message_t msg)
{
	int			iRetCode = ITK_ok;
	int			iNumArg = 0;
	tag_t		tRootTask = NULLTAG;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* pcSolRel = NULL;
	char* pcRelList = NULL;
	char* pcObjType = NULL;
	char* pcNextStatus = NULL;
	//char        *status                 = NULL;
std:string status("");
	std::map<std::string, int> allowedStatusesAndAge;

	char* isDebug = NULL;
	int ret_code = ITK_ok;
	CHECK_ITK(ret_code, PREF_ask_char_value("VF_PLM_DEBUG", 0, &isDebug));

	LOG(TC_write_syslog("\n[VF] @@@Enter %s", __FUNCTION__));

	/*getting number of arguments*/
	iNumArg = TC_number_of_arguments(msg.arguments);

	if (iNumArg > 0)
	{
		for (int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue);
			/*for argument template name*/
			if (tc_strcmp(pcFlag, "type") == 0)
			{
				pcObjType = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcObjType, pcValue);
			}
			else if (tc_strcmp(pcFlag, "solution") == 0)
			{
				pcSolRel = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcSolRel, pcValue);
			}
			else if (tc_strcmp(pcFlag, "relation") == 0)
			{
				pcRelList = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcRelList, pcValue);
			}
			else if (tc_strcmp(pcFlag, "status") == 0)
			{
				status.assign(pcValue);
				LOG(TC_write_syslog("\n[VF] status: %s", status.c_str()));
			}
			else if (tc_strcmp(pcFlag, "nextstatus") == 0)
			{
				pcNextStatus = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcNextStatus, pcValue);
			}
		}
		MEM_free(pcValue);
		MEM_free(pcFlag);
	}
	else
	{
		iRetCode = EPM_wrong_number_of_arguments;
	}

	/*prepare status list*/
	if (status.empty())
	{
		status.assign("VF4_Sourcing,VF4_BCR,VF4_SCR,Vl5_I,VF4_IECR,PR4D_PR");
	}

	//TODO: put this to preference
	std::map<std::string, std::string> currentAndNextStatus;
	//currentAndNextStatus["VF4_Sourcing"] = "VF4_BCR";
	//currentAndNextStatus["VF4_BCR"] = "VF4_BCR";
	//currentAndNextStatus["VF4_SCR"] = "VF4_SCR";
	currentAndNextStatus["Vl5_I"] = "VF4_IECR";
	currentAndNextStatus["VF4_IECR"] = "VF4_IECR";
	//currentAndNextStatus["PR4D_PR"] = "PR4D_PR";

	std::vector<std::string> statuses = split_string(status, ',');
	for (int i = 0; i < statuses.size(); i++)
	{
		std::string statusTmp = statuses[i];
		LOG(TC_write_syslog("\n[VF] before trim: %s", statusTmp.c_str()));
		ltrim(statusTmp);
		rtrim(statusTmp);
		allowedStatusesAndAge[statusTmp] = i;
		LOG(TC_write_syslog("\n[VF] %s", statusTmp.c_str()));
	}

	LOG(TC_write_syslog("\n[VF] Prepare status list done."));

	/*getting root task*/
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int			iTgt = 0;
		tag_t* ptTgt = NULL;
		char* pcRelTypeList = NULL;
		char* pcReqRel = NULL;
		vector<string> RelList;

		pcRelTypeList = (char*)MEM_alloc(((int)tc_strlen(pcRelList) + 1) * sizeof(char));
		tc_strcpy(pcRelTypeList, pcRelList);

		pcReqRel = tc_strtok(pcRelTypeList, ",");
		while (pcReqRel != NULL)
		{
			RelList.push_back(pcReqRel);
			pcReqRel = tc_strtok(NULL, ",");
		}

		/*getting target objects*/
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		tag_t ecnRev = NULLTAG;
		for (int iInd = 0; iInd < iTgt; iInd++)
		{
			char* pcTgtType = NULL;

			CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcTgtType));
			if (tc_strcmp(pcObjType, pcTgtType) == NULL)
			{
				/*if given object type is matched*/
				int		iSolItems = 0;
				tag_t* ptSol = NULL;
				tag_t	tRelation = NULLTAG;

				CHECK_ITK(iRetCode, GRM_find_relation_type(pcSolRel, &tRelation));
				if (tRelation != 0)
				{
					CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[iInd], tRelation, &iSolItems, &ptSol));
					for (int iCnt = 0; iCnt < iSolItems; iCnt++)
					{
						int			iTemp = 0;
						char* pcItemRev = NULL;
						tag_t		tItem = NULLTAG;
						tag_t		tLatestValidReleasedRev = NULLTAG;
						char* pcItem = NULL;
						string		szItemRev = "";
						string		szPrevRev = "";

						CHECK_ITK(iRetCode, ITEM_ask_rev_id2(ptSol[iCnt], &pcItemRev));
						CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptSol[iCnt], &tItem));
						CHECK_ITK(iRetCode, ITEM_ask_id2(tItem, &pcItem));

						LOG(TC_write_syslog("\n[VF] processes %s-%s.", pcItem, pcItemRev));

						/*get previous revision*/
						char errorMsg[1024] = { '\0' };
						szItemRev.assign(pcItemRev);
						iTemp = stoi(szItemRev);
						iTemp--;

						if (to_string(iTemp).length() == 1)
						{
							szPrevRev.assign("0");
							szPrevRev.append(to_string(iTemp));
						}
						else
						{
							szPrevRev.assign(to_string(iTemp));
						}

						LOG(TC_write_syslog("\n[VF] previous rev: %s.", szPrevRev.c_str()));

						int			iSts = 0;
						int			iBvr = 0;
						tag_t* ptRelSts = NULLTAG;
						tag_t* ptBvr = NULLTAG;
						char* pcStsName = NULL;

						/*getting latest release status of prev rev*/
						tag_t tRevRule = NULLTAG;
						char* szHowConfigured = NULL;
						CHECK_ITK(iRetCode, CFM_find(__ADMIN_LATEST_VALID_STATUS_SCOOTER, &tRevRule));
						if (iRetCode == ITK_ok && tRevRule != NULLTAG)
						{
							CHECK_ITK(iRetCode, CFM_item_ask_configured(tRevRule, tItem, &tLatestValidReleasedRev, &szHowConfigured));
							LOG(TC_write_syslog("\n[VF] %s", szHowConfigured));
							SAFE_MEM_free(szHowConfigured);
						}
						else
						{
							TC_write_syslog("\n[VF] WARNING: CANNOT FIND REV RULE %s", __ADMIN_LATEST_VALID_STATUS_SCOOTER);
							iRetCode = ITK_ok;
						}

						if (tLatestValidReleasedRev != NULLTAG)
						{
							CHECK_ITK(iRetCode, AOM_refresh(tLatestValidReleasedRev, FALSE));
							CHECK_ITK(iRetCode, WSOM_ask_release_status_list(tLatestValidReleasedRev, &iSts, &ptRelSts));
							SAFE_SM_FREE(pcItemRev);
							CHECK_ITK(iRetCode, ITEM_ask_rev_id2(tLatestValidReleasedRev, &pcItemRev));
							LOG(TC_write_syslog("\n[VF] No. status of part %s/%s=%d.", pcItem, pcItemRev, iSts));
						}
						else
						{
							CHECK_ITK(iRetCode, AOM_refresh(ptSol[iCnt], FALSE));
							CHECK_ITK(iRetCode, WSOM_ask_release_status_list(ptSol[iCnt], &iSts, &ptRelSts));
							LOG(TC_write_syslog("\n[VF] No. status of part %s/%s=%d.", pcItem, pcItemRev, iSts));
							if ((iSts == 0 || ptRelSts == NULLTAG) && szPrevRev.size() > 0)
							{
								iSts = 0;
								ptRelSts = NULLTAG;
								CHECK_ITK(iRetCode, ITEM_find_revision(tItem, szPrevRev.c_str(), &tLatestValidReleasedRev));
								if (tLatestValidReleasedRev == NULLTAG)
								{
									std::string szRevIdTemp("0");
									szRevIdTemp.append(szPrevRev);
									CHECK_ITK(iRetCode, ITEM_find_revision(tItem, szRevIdTemp.c_str(), &tLatestValidReleasedRev));
								}

								if (tLatestValidReleasedRev != NULLTAG)
								{
									CHECK_ITK(iRetCode, WSOM_ask_release_status_list(tLatestValidReleasedRev, &iSts, &ptRelSts));
									LOG(TC_write_syslog("\n[VF] No. status of part %s/%s=%d (current rev does not have status).", pcItem, szPrevRev.c_str(), iSts));
								}
							}
						}

						//if(szPrevRev.length() > 0 || )
						if (ptRelSts != NULLTAG && iSts > 0)
						{
							// loop status list to find the latest allowed status
							int oldestAge = -999999;
							std::string latestValidStatus("");
							for (int i = 0; i < iSts; i++)
							{
								CHECK_ITK(iRetCode, RELSTAT_ask_release_status_type(ptRelSts[i], &pcStsName));
								if (allowedStatusesAndAge.find(pcStsName) != allowedStatusesAndAge.end())
								{
									int age = allowedStatusesAndAge[pcStsName];
									if (age > oldestAge)
									{
										oldestAge = age;
										latestValidStatus = pcStsName;
									}
								}
								SAFE_SM_FREE(pcStsName);
							}

							LOG(TC_write_syslog("\n[VF] latestValidStatus: %s.", latestValidStatus.c_str()));
							if (!latestValidStatus.empty())
							{
								CHECK_ITK(iRetCode, ITEM_rev_list_bom_view_revs(ptSol[iCnt], &iBvr, &ptBvr));
								if (currentAndNextStatus.find(latestValidStatus) != currentAndNextStatus.end())
								{
									pcStsName = (char*)currentAndNextStatus[latestValidStatus].c_str();
								}
								else
								{
									pcStsName = pcNextStatus;
									//sprintf_s(errorMsg, "Cannot find next status for status %s (%s/%s)", latestValidStatus.c_str(), pcItem, pcItemRev);
									//TC_write_syslog("\n [VF] ERROR: %s", errorMsg);
								}


								tag_t	tRelSts = NULLTAG;

								CHECK_ITK(iRetCode, RELSTAT_create_release_status(pcStsName, &tRelSts));
								CHECK_ITK(iRetCode, RELSTAT_add_release_status(tRelSts, 1, &ptSol[iCnt], true));
								/*to add rel status on BVR, Specificiation objects and Rendering Objects*/
								if (iBvr > 0)
								{
									CHECK_ITK(iRetCode, RELSTAT_add_release_status(tRelSts, 1, &ptBvr[0], true));
								}
								for (int iIter1 = 0; iIter1 < RelList.size(); iIter1++)
								{
									int			iRelCnt = 0;
									tag_t		tArgRel = NULLTAG;
									tag_t* ptRelTags = NULL;

									CHECK_ITK(iRetCode, GRM_find_relation_type(RelList[iIter1].c_str(), &tArgRel));
									CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptSol[iCnt], tArgRel, &iRelCnt, &ptRelTags));
									for (int iIter2 = 0; iIter2 < iRelCnt; iIter2++)
									{
										CHECK_ITK(iRetCode, RELSTAT_add_release_status(tRelSts, 1, &ptRelTags[iIter2], true));
									}

									SAFE_SM_FREE(ptRelTags);
								}

								SAFE_SM_FREE(ptBvr);
							}
							else
							{
								sprintf_s(errorMsg, "Cannot find latest valid status for %s/%s", pcItem, pcItemRev);
								TC_write_syslog("\n [VF] ERROR: %s", errorMsg);
							}
						}
						else
						{
							sprintf_s(errorMsg, "There are some errors with %s/%s. Cannot find previous revision.", pcItem, pcItemRev);
							TC_write_syslog("\n [VF] ERROR: %s", errorMsg);
						}

						if (tc_strlen(errorMsg) > 2)
						{
							EMH_store_error_s1(EMH_severity_user_error, EMH_GENERIC_ERROR, errorMsg);
						}

						SAFE_MEM_free(pcItemRev);
						SAFE_MEM_free(pcItem);
					}
				}

				SAFE_SM_FREE(ptSol);
			}
			SAFE_MEM_free(pcTgtType);
		}
		SAFE_MEM_free(pcRelTypeList);
	}

	LOG(TC_write_syslog("\n[VF] @@@Leave %s", __FUNCTION__));
	return iRetCode;
}