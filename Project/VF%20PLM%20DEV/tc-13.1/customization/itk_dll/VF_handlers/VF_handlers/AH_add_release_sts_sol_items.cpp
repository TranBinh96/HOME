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


bool checkStampByNewPartStatus(tag_t rev, char* isDebug)
{
	LOG(TC_write_syslog("\n[VF] @@@Enter %s", __FUNCTION__));

	int retcode = ITK_ok;
	int revsCount = 0;
	tag_t* revs = NULL;
	tag_t item = NULLTAG;

	CHECK_ITK(retcode, ITEM_ask_item_of_rev(rev, &item));
	CHECK_ITK(retcode, ITEM_list_all_revs(item, &revsCount, &revs));

	bool isStampByNewPartStatus = false;
	if (revsCount == 1)
	{
		isStampByNewPartStatus = true;
	}
	else if (revsCount > 1)
	{
		char* revID = NULL;
		char* howConfigure = NULL;
		tag_t revRule = NULLTAG;
		tag_t tItem = NULLTAG;
		tag_t latestValidReleaseRev = NULLTAG;
		CHECK_ITK(retcode, CFM_find(__ADMIN_LATEST_VALID_STATUS_ONLY, &revRule));

		if (revRule != NULLTAG)
		{
			CHECK_ITK(retcode, ITEM_ask_item_of_rev(rev, &tItem));
			CHECK_ITK(retcode, CFM_item_ask_configured(revRule, tItem, &latestValidReleaseRev, &howConfigure));
			SAFE_MEM_free(howConfigure);
			if (latestValidReleaseRev == NULL) isStampByNewPartStatus = true;
		}
		else
		{
			LOG(TC_write_syslog("\n[VF] WARNING: Cannot find revision rule %s", __ADMIN_LATEST_VALID_STATUS_ONLY));
			retcode = ITK_ok;
			CHECK_ITK(retcode, ITEM_ask_rev_id2(rev, &revID));
			isStampByNewPartStatus = (tc_strcmp("01", revID) == 0 || tc_strcmp("001", revID) == 0);
			SAFE_SM_FREE(revID);
		}
	}

	LOG(TC_write_syslog("\n[VF] @@@LEAVE %s", __FUNCTION__));

	return isStampByNewPartStatus;
}

extern int AH_add_release_sts_sol_items(EPM_action_message_t msg)
{
	int			iRetCode = ITK_ok;
	int			iNumArg = 0;
	tag_t		tRootTask = NULLTAG;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* pcSolRel = NULL;
	char* pcRelList = NULL;
	char* pcObjType = NULL;

	char* isDebug = NULL;
	int ret_code = ITK_ok;

	std::string status;

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
		char* newPartsStatus = NULL;
		tag_t ecnRev = NULLTAG;
		for (int iInd = 0; iInd < iTgt; iInd++)
		{
			char* pcTgtType = NULL;

			CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcTgtType));
			if (tc_strcmp(pcTgtType, "Vf6_ECNRevision") == 0)
			{
				// new part status
				if (ecnRev == NULL)
				{
					ecnRev = ptTgt[iInd];
					tag_t lovTag = NULL;
					CHECK_ITK(iRetCode, AOM_ask_value_string(ecnRev, "vf6_new_parts_status", &newPartsStatus));
				}
				else
				{
					// 2 MCNs in target
				}
			}

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
						int iBvr = 0;
						tag_t* ptBvr = NULL;
						char* pcStsName = NULL;

						CHECK_ITK(iRetCode, ITEM_ask_rev_id2(ptSol[iCnt], &pcItemRev));
						CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptSol[iCnt], &tItem));
						CHECK_ITK(iRetCode, ITEM_ask_id2(tItem, &pcItem));

						LOG(TC_write_syslog("\n[VF] processes %s-%s.", pcItem, pcItemRev));
						bool isFirstRevision = checkStampByNewPartStatus(ptSol[iCnt], isDebug);
						if (!isFirstRevision)
						{
							char errorMsg[1024] = { '\0' };
							std::string latestValidStatus = getLatestValidStatus(status, ptSol[iCnt], isDebug);

							LOG(TC_write_syslog("\n[VF] latestValidStatus: %s.", latestValidStatus.c_str()));
							if (!latestValidStatus.empty())
							{
								CHECK_ITK(iRetCode, ITEM_rev_list_bom_view_revs(ptSol[iCnt], &iBvr, &ptBvr));
								std::string nextStatus = getNextStatus(latestValidStatus);
								if (tc_strlen(nextStatus.c_str()) > 0)
								{
									pcStsName = (char*)nextStatus.c_str();
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
									sprintf_s(errorMsg, "Cannot find next status for status %s (%s/%s)", latestValidStatus.c_str(), pcItem, pcItemRev);
									TC_write_syslog("\n [VF] ERROR: %s", errorMsg);
								}
							}
							else
							{
								sprintf_s(errorMsg, "Cannot find latest valid status for %s/%s", pcItem, pcItemRev);
								TC_write_syslog("\n [VF] ERROR: %s", errorMsg);
							}

							if (tc_strlen(errorMsg) > 2)
							{
								EMH_store_error_s1(EMH_severity_user_error, EMH_GENERIC_ERROR, errorMsg);
							}
						}
						else
						{
							// Case adding status for new parts
							bool isFailInAddingNewPartStatus = false;
							if (newPartsStatus != NULL && tc_strlen(newPartsStatus) > 0)
							{
								tag_t newPartsStatusTag = NULLTAG;
								CHECK_ITK(iRetCode, RELSTAT_create_release_status(newPartsStatus, &newPartsStatusTag));
								if (newPartsStatusTag != NULLTAG)
								{
									CHECK_ITK(iRetCode, RELSTAT_add_release_status(newPartsStatusTag, 1, &ptSol[iCnt], true));
									/*to add rel status on BVR and related Objects inputed by relation param*/
									int iBvr = -1;
									tag_t* ptBvr = NULL;
									CHECK_ITK(iRetCode, ITEM_rev_list_bom_view_revs(ptSol[iCnt], &iBvr, &ptBvr));
									if (iBvr > 0)
									{
										CHECK_ITK(iRetCode, RELSTAT_add_release_status(newPartsStatusTag, 1, &ptBvr[0], true));
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
											CHECK_ITK(iRetCode, RELSTAT_add_release_status(newPartsStatusTag, 1, &ptRelTags[iIter2], true));
											if (iRetCode != ITK_ok) isFailInAddingNewPartStatus = false;
										}
									}
								}
								else
								{
									// new part status get from mcn revision is wrong
									isFailInAddingNewPartStatus = true;
								}
							}
							else
							{
								isFailInAddingNewPartStatus = true;
							}

							if (isFailInAddingNewPartStatus)
							{
								char* failRevStr = NULL;
								CHECK_ITK(iRetCode, AOM_ask_value_string(ptSol[iCnt], "object_string", &failRevStr));
								char errorMsg[1024] = { '\0' };
								if (failRevStr)
								{
									sprintf_s(errorMsg, "Error occured in adding status for new parts: %s", failRevStr);
									EMH_store_error_s1(EMH_severity_user_error, EMH_GENERIC_ERROR, errorMsg);
									MEM_free(failRevStr);
								}
							}
						}
						SAFE_MEM_free(pcItemRev);
						SAFE_MEM_free(pcItem);
					}
				}

				SAFE_SM_FREE(ptSol);
			}
			SAFE_MEM_free(pcTgtType);
		}
		SAFE_SM_FREE(newPartsStatus);
		SAFE_MEM_free(pcRelTypeList);
	}

	LOG(TC_write_syslog("\n[VF] @@@Leave %s", __FUNCTION__));
	return iRetCode;
}