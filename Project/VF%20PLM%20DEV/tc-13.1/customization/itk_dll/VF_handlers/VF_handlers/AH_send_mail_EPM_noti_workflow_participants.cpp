#include "Vinfast_Custom.h"

extern int AH_send_mail_EPM_noti_workflow_participants(EPM_action_message_t msg)
{
	int iRetCode = ITK_ok;
	int iNumArgs = 0;
	int	iPrefCount = 0;
	int	portPref_count = 0;
	tag_t tRootTask = NULLTAG;
	char* pcMailSubject = NULL;
	char* pcRecipients = NULL;
	char* pcObjType = NULL;
	char* pcFile = NULL;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* pcReqType = NULL;

	boolean isDebug = false;
	boolean isNotifyAll = false;

	iNumArgs = TC_number_of_arguments(msg.arguments);

	std::stringstream mailBody;
	std::vector<string> szMailList;

	if (iNumArgs > 0)
	{
		for (int i = 0; i < iNumArgs; i++)
		{
			/*getting arguments*/
			CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue));
			if (iRetCode == ITK_ok)
			{
				if (tc_strcasecmp(pcFlag, "subject") == 0 && pcValue != NULL)
				{
					pcMailSubject = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcMailSubject, pcValue);
				}
				if (tc_strcasecmp(pcFlag, "type") == 0 && pcValue != NULL)
				{
					pcReqType = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcReqType, pcValue);
				}
				if (tc_strcasecmp(pcFlag, "recipient") == 0 && pcValue != NULL)
				{
					pcRecipients = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcRecipients, pcValue);
				}
				if (tc_strcasecmp(pcFlag, "debug") == 0 && pcValue != NULL)
				{
					isDebug = true;
				}
				if (tc_strcasecmp(pcFlag, "notifyAll") == 0 && pcValue != NULL)
				{
					isNotifyAll = true;
				}
			}
		}
		SAFE_MEM_free(pcFlag);
		SAFE_MEM_free(pcValue);
	}
	else
	{
		iRetCode = EPM_invalid_argument;
	}

	/*check manadatory arguments*/
	if (tc_strcmp(pcMailSubject, "") == 0 || tc_strcmp(pcReqType, "") == 0)
	{
		iRetCode = EPM_invalid_argument;
	}

	/*getting root task*/
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int	iAllTasks = 0;
		int	iTgt = 0;
		tag_t* ptTgt = NULL;
		tag_t  current_job = NULLTAG;
		tag_t* ptAllTaskTags = NULL;
		tag_t tInitiator = NULL;
		string szRejMailList = "";
		char* process_desc = NULL;

		CHECK_ITK(iRetCode, AOM_ask_value_tag(msg.task, "owning_user", &tInitiator));
		CHECK_ITK(iRetCode, EPM_ask_job(msg.task, &current_job));
		CHECK_ITK(iRetCode, EPM_ask_sub_tasks(tRootTask, &iAllTasks, &ptAllTaskTags));
		CHECK_ITK(iRetCode, AOM_ask_value_string(current_job, "current_desc", &process_desc));
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		/*getting mail id of all participants from review task*/
		for (int iLoopAllTasks = 0; iLoopAllTasks < iAllTasks; iLoopAllTasks++)
		{
			char* pcTaskTypeName = NULL;
			char* pcTaskState = NULL;
			char* pcTaskResult = NULL;

			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_type", &pcTaskTypeName));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_state", &pcTaskState));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_result", &pcTaskResult));

			//if (tc_strcmp(pcTaskTypeName, "EPMReviewTask") == 0 && (tc_strcmp(pcTaskState, "Started") == 0 || tc_strcmp(pcTaskState, "Completed") == 0))
			if (tc_strcmp(pcTaskTypeName, "EPMReviewTask") == 0)
			{
				int		iAllSubTasks = 0;
				tag_t* ptAllSubTaskTags = NULL;

				CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks], &iAllSubTasks, &ptAllSubTaskTags));
				for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
				{
					char* pcSubTaskTypeName = NULL;

					CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
					if (tc_strcmp(pcSubTaskTypeName, "EPMSelectSignoffTask") == 0)
					{
						int		iNumAttachs = 0;
						tag_t* ptAttachsTagList = NULL;

						//get signoff attachments
						CHECK_ITK(iRetCode, EPM_ask_attachments(ptAllSubTaskTags[iLoopAllSubTasks], EPM_signoff_attachment, &iNumAttachs, &ptAttachsTagList));
						for (int iUsers = 0; iUsers < iNumAttachs; iUsers++)
						{
							tag_t		tUser = NULLTAG;
							char* pcUserMail = NULL;
							CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAttachsTagList[iUsers], "fnd0Performer", &tUser));
							CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
							//fprintf(fMailAddress,"%s\n",pcUserMail);
							if (tc_strcmp(pcUserMail, "") != NULL)
							{
								if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
								{
									szMailList.push_back(pcUserMail);
								}
							}
							SAFE_SM_FREE(pcUserMail);
						}
					}
					SAFE_SM_FREE(pcSubTaskTypeName);
				}
			}

			SAFE_SM_FREE(pcTaskTypeName);
			SAFE_SM_FREE(pcTaskState);
			SAFE_SM_FREE(pcTaskResult);
		}

		/*getting mail id of initiator of workflow*/
		if (tInitiator != NULLTAG)
		{
			char* pcUserMail = NULL;

			CHECK_ITK(iRetCode, EPM_get_user_email_addr(tInitiator, &pcUserMail));
			if (tc_strcmp(pcUserMail, "") != NULL)
			{
				if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
				{
					szMailList.push_back(pcUserMail);
				}
			}
		}

		//get from recepi
		char* next_p;
		char* chars_array = strtok_s(pcRecipients, ",", &next_p);
		while (chars_array)
		{
			szMailList.push_back(chars_array);
			chars_array = strtok_s(NULL, ",", &next_p);
		}

		for (int iInd = 0; iInd < iTgt; iInd++)
		{
			char* pcObjType = NULL;
			char* pcDesc = NULL;
			char* pcSynopsis = NULL;
			char* pcObjString = NULL;
			char* pcItemID = NULL;
			char* pcRevID = NULL;
			char* pcName = NULL;
			tag_t	tChangeItem = NULLTAG;
			tag_t	tClass = NULLTAG;
			tag_t	tRevClass = NULLTAG;
			string	szTgtParts = "";

			/*get description, synopsis, module group*/

			CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcObjType));

			/*get solution items*/
			if (tc_strcmp(pcObjType, pcReqType) == 0)
			{
				CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iInd], &tChangeItem));
				CHECK_ITK(iRetCode, ITEM_ask_id2(tChangeItem, &pcItemID));
				CHECK_ITK(iRetCode, ITEM_ask_rev_id2(ptTgt[iInd], &pcRevID));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_name", &pcSynopsis));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_desc", &pcDesc));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_string", &pcObjString));

				//Over view
				mailBody << "<html><meta charset='UTF - 8'>";
				mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:#448DA6'>Overview:<o:p></o:p></span></b></p></div>";
				mailBody << "<table class=MsoNormalTable border=0 cellspacing=3 cellpadding=0>";
				mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Current Task: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>Notify <o:p></o:p></span></p></td></tr>";
				mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Process Name: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << pcItemID << "/" << pcRevID << " - " << pcSynopsis << " <o:p></o:p></span></p></td></tr>";
				mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Due Date: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>None <o:p></o:p></span></p></td></tr>";
				mailBody << "</table>";

				//Select the preferred
				mailBody << "<p class=MsoNormal><span style='font-family:'Arial',sans-serif'><o:p>&nbsp;</o:p></span></p>";
				mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:#448DA6'>Select the preferred client to view the task:<o:p></o:p></span></b></p></div>";
				mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'><a href='http://tcweb.vinfast.vn/tc/launchapp?-attach=true&amp;-s=226TCSession&amp;-o=mhMA2ZnP47MsRA'>Rich Client</a><o:p></o:p></span></p></div>";
				mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'><a href='https://tcweb.vinfast.vn/#com.siemens.splm.clientfx.tcui.xrt.showObject?uid=mlKA2ZnP47MsRA'>Active Workspace</a><o:p></o:p></span></p></div>";

				//Target
				mailBody << "<p class=MsoNormal><span style='font-family:'Arial',sans-serif'><o:p>&nbsp;</o:p></span></p>";
				mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:#448DA6'>Target:<o:p></o:p></span></b></p></div>";
				mailBody << "<table class=MsoNormalTable border=1 cellspacing=0 cellpadding=0 width='100%' style='width:100.0%;border-collapse:collapse;border:none'><tr>";
				mailBody << "<td style='border:solid gray 1.0pt;background:#F0F0F0;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><b><span style='font-family:'Arial',sans-serif;color:gray'>Name<o:p></o:p></span></b></p></td>";
				mailBody << "<td style='border:solid gray 1.0pt;border-left:none;background:#F0F0F0;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><b><span style='font-family:'Arial',sans-serif;color:gray'>Type<o:p></o:p></span></b></p></td></tr>";

				for (int iPrt = 0; iPrt < iTgt; iPrt++)
				{
					char* pcName = NULL;
					char* pcTgtType = NULL;

					CHECK_ITK(iRetCode, WSOM_ask_id_string(ptTgt[iPrt], &pcName));
					CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iPrt], &pcTgtType));

					mailBody << "<tr><td style='border:solid gray 1.0pt;border-top:none;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><span style='font-family:'Arial',sans-serif'>" << pcName << "<o:p></o:p></span></p></td>";
					mailBody << "<td style='border-top:none;border-left:none;border-bottom:solid gray 1.0pt;border-right:solid gray 1.0pt;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><span style='font-family:'Arial',sans-serif'>" << pcTgtType << "<o:p></o:p></span></p></td></tr>";

					SAFE_MEM_free(pcName);
					SAFE_MEM_free(pcTgtType);
				}

				mailBody << "</table>";
				mailBody << "<p class=MsoNormal><span style='font-family:'Arial',sans-serif'><o:p>&nbsp;</o:p></span></p>";
				mailBody << "<div><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:gray'>This email was sent from Teamcenter.<o:p></o:p></span></b></p></div>";
				mailBody << "</html>";
			}

			SAFE_MEM_free(pcDesc);
			SAFE_MEM_free(pcSynopsis);
			SAFE_MEM_free(pcItemID);
			SAFE_MEM_free(pcObjType);
			SAFE_MEM_free(process_desc);
		}
	}

	sendEmail(pcMailSubject, mailBody.str(), szMailList, isDebug);
	return iRetCode;
}