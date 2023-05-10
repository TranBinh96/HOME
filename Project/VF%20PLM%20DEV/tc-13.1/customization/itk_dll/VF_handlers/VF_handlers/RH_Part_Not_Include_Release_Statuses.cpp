#include "Vinfast_Custom.h"


logical checkStatus1(char *pcStatus, char *pcReqStatus)
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
EPM_decision_t RH_Part_Not_Include_Release_Statuses(EPM_rule_message_t msg)
{
	int iNumArg  = 0;
	int iRetCode =  ITK_ok;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* pcObjectType = NULL;
	char* pcStatusList = NULL;
	int count_tasks = NULL;
	tag_t *all_tasks = NULLTAG;
	tag_t tRootTask = NULLTAG;
	char *name_task = NULL;
	char *name_release = NULL;
	logical result = true;
	char ErrorMessage[2048] = "";
	EPM_decision_t decision  = EPM_go;
	 //TC_write_syslog("//////////////////////////////////////////////////////////////////////////// s RH_Part_Not_Include_Release_Statuses");
	iNumArg = TC_number_of_arguments(msg.arguments);
	 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////1");
	if(iNumArg > 0)
	{
		for(int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////2");
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue);
			 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////2");
			if(tc_strcmp(pcFlag, "type") == 0)
			{
				 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////3");
				pcObjectType = (char*) MEM_alloc(((int) tc_strlen(pcValue) +1) * sizeof(char) );
				 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////3");
				tc_strcpy(pcObjectType, pcValue);
			}
			if(tc_strcmp(pcFlag, "status") == 0)
			{
				pcStatusList = (char*) MEM_alloc(((int) tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcStatusList, pcValue);
			}
			
			SAFE_MEM_free(pcValue);
			SAFE_MEM_free(pcFlag);
		}		
	}
	else
	{
		iRetCode = EPM_wrong_number_of_arguments;
	}
	 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////4");
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if(tRootTask != 0)
	{
		int iTgt = 0;
		tag_t *ptTgt = NULL;
		 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////4");
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////5");
		for(int iInd = 0; iInd < iTgt; iInd++)
		{
			char  *pcTgtType = NULL;
			 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////5");
			CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcTgtType));
			 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////6");
			if(tc_strcmp(pcObjectType, pcTgtType) == NULL)
			{
				//Find Item from ItemRevision
				tag_t item = NULLTAG;
				int count = NULL;
				tag_t *rev_list = NULL;
				char *name = NULL;
				char *item_id = NULL;
				char *item_rev = NULL;
				CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iInd], &item));
				CHECK_ITK(iRetCode, ITEM_list_all_revs(item, &count, &rev_list));
				CHECK_ITK(iRetCode, ITEM_ask_id2(item, &item_id));
				logical *rev_is_readable_list = NULL;
				CHECK_ITK(iRetCode, AM_check_read_privilege_in_bulk(count, rev_list, &rev_is_readable_list));
				//TC_write_syslog("////////////////////////////////////////////////////////////////////////////6");
				for(int iItemRev = 0; iItemRev < count && rev_is_readable_list[iItemRev]; iItemRev++)
				//for(int iItemRev = 0; iItemRev < count; iItemRev++)
				{
					//TC_write_syslog("////////////////////////////////////////////////////////////////////////////7");
					tag_t *status_list = NULL;
					int status_count = 0;
					CHECK_ITK(iRetCode, WSOM_ask_release_status_list(rev_list[iItemRev], &status_count, &status_list));
					if (iRetCode != ITK_ok)
					{
						char *tmpUID = NULL;
						ITK__convert_tag_to_uid(rev_list[iItemRev], &tmpUID);
						TC_write_syslog("////////////////////////////////////////////////////////////////////////////tmpUID=%s",tmpUID);
						SAFE_SM_FREE(tmpUID);
					}
					else 
					{
						CHECK_ITK(iRetCode, ITEM_ask_rev_id2(rev_list[iItemRev], &item_rev));
					}
					 
					 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////7");
					 if(status_count > 0)
					 {
						 for(int iStatusCount = 0; iStatusCount < status_count; iStatusCount++)
						 {
							 //get status_list
							 
							 CHECK_ITK(iRetCode,RELSTAT_ask_release_status_type(status_list[iStatusCount], &name));
							 TC_write_syslog("////////////////////////////////////////////////////////////////////////////8");
							 TC_write_syslog("\t %s \n", name);
							 TC_write_syslog("\t %s \n", pcStatusList);
							 //TC_write_syslog("////////////////////////////////////////////////////////////////////////////8");
							 logical lValid = true;
						     lValid = checkStatus1(name, pcStatusList);
							 if(lValid == true)
							 {
								 sprintf(ErrorMessage, "Part %s already has the release status in revision %s. All part revisions should not have the release status \"%s\".", item_id, item_rev, pcStatusList);
								 EMH_store_error_s1(EMH_severity_user_error, CHECK_STATUS, ErrorMessage);
								 result = false;
								 SAFE_SM_FREE(name);
								 break;
							 }

							 SAFE_SM_FREE(name);
						 }
					 }
					 SAFE_SM_FREE(status_list);
					 SAFE_SM_FREE(item_rev);
				}
				SAFE_SM_FREE(item_id);
				SAFE_SM_FREE(rev_list);
				SAFE_SM_FREE(rev_is_readable_list);
			}
			SAFE_SM_FREE(pcTgtType);
		}

		SAFE_SM_FREE(ptTgt);
	}

	//TC_write_syslog("//////////////////////////////////////////////////////////////////////////// e RH_Part_Not_Include_Release_Statuses");

	SAFE_SM_FREE(pcObjectType);
	SAFE_SM_FREE(pcStatusList);


	if(result == false)
	{
		//TC_write_syslog("EPM_NOGO");
		decision = EPM_nogo;
		//TC_write_syslog("EPM_NOGO");
		return decision;
	}
	else
	{
		//TC_write_syslog("EPM_GO");
		decision = EPM_go;
		//TC_write_syslog("EPM_GO");
		return decision;
	}
	
	
	
	
}