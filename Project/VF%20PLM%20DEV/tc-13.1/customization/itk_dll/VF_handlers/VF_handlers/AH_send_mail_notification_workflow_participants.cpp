#include "Vinfast_Custom.h"

extern int AH_send_mail_notification_workflow_participants(EPM_action_message_t msg)
{
	int	iRetCode = ITK_ok;
	int	iNumArgs = 0;
	tag_t tRootTask = NULLTAG;
	char* pcMailSubject = NULL;
	char* pcNewMailSubject = NULL;
	char* pcComments = NULL;
	char* pcTaskName = NULL;
	char* pcMailRecp = NULL;
	char* pcReqType = NULL;
	char* pcProcessSts = NULL;
	boolean isDebug = false;
	boolean isNotifyAll = false;

	iNumArgs = TC_number_of_arguments(msg.arguments);

	std::stringstream mailBody;
	std::vector<string> szMailList;

	if (iNumArgs > 0)
	{
		char* pcFlag = NULL;
		char* pcValue = NULL;
		for (int i = 0; i < iNumArgs; i++)
		{
			CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue));
			if (iRetCode == ITK_ok)
			{
				if (tc_strcasecmp(pcFlag, "subject") == 0 && pcValue != NULL)
				{
					pcMailSubject = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcMailSubject, pcValue);
				}
				if (tc_strcasecmp(pcFlag, "recipient") == 0 && pcValue != NULL)
				{
					pcMailRecp = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcMailRecp, pcValue);
				}
				if (tc_strcasecmp(pcFlag, "type") == 0 && pcValue != NULL)
				{
					pcReqType = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcReqType, pcValue);
				}
				if (tc_strcasecmp(pcFlag, "status") == 0 && pcValue != NULL)
				{
					pcProcessSts = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcProcessSts, pcValue);
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

	pcNewMailSubject = (char*)MEM_alloc(((int)tc_strlen(pcMailSubject) + 1) * sizeof(char));
	tc_strcpy(pcNewMailSubject, pcMailSubject);

	/*getting root task*/
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		tag_t tInitiator = NULL;
		tag_t current_job = NULLTAG;
		int	iAllTasks = 0;
		tag_t* ptAllTaskTags = NULL;
		char* process_desc = NULL;
		int	iTgt = 0;
		tag_t* ptTgt = NULL;
		string szRejMailList = "";

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

			if (tc_strcmp(pcTaskTypeName, "EPMReviewTask") == 0 && (tc_strcmp(pcTaskState, "Started") == 0 || tc_strcmp(pcTaskState, "Completed") == 0))
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
							if (tUser != 0) {
								CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
								//fprintf(fMailAddress,"%s",pcUserMail);
								if (tc_strcmp(pcUserMail, "") != NULL)
								{
									if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
									{
										szMailList.push_back(pcUserMail);
									}
								}
							}
							SAFE_SM_FREE(pcUserMail);
						}
					}

					SAFE_SM_FREE(pcSubTaskTypeName);
				}
			}

			/*getting mail id of participants from do or condition task*/
			if ((tc_strcmp(pcTaskTypeName, "EPMDoTask") == 0 || tc_strcmp(pcTaskTypeName, "EPMConditionTask") == 0) && (tc_strcmp(pcTaskState, "Completed") == 0 || tc_strcmp(pcTaskState, "Started") == 0))
			{
				tag_t		tUser = NULLTAG;
				char* pcUserMail = NULL;

				CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAllTaskTags[iLoopAllTasks], "fnd0Performer", &tUser));
				if (tUser != NULLTAG)
				{
					CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
					if (pcUserMail != 0) {
						if (tc_strcmp(pcUserMail, "") != NULL)
						{
							if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
							{
								szMailList.push_back(pcUserMail);
							}
						}
					}

					/*getting mail id of participants from do or condition task*/
					if (tc_strcmp(pcTaskState, "Completed") == 0 && tc_strcmp(pcTaskResult, "Rejected") == 0)
					{
						CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "comments", &pcComments));
						CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "object_name", &pcTaskName));
						CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
						//fprintf(fMailAddress,"%s",pcUserMail);
						if (pcUserMail != 0) {
							if (tc_strcmp(pcUserMail, "") != NULL)
							{
								if (szRejMailList.size() > 2)
								{
									szRejMailList.append(",");
								}
								szRejMailList.append(pcUserMail);
							}

						}
					}

					SAFE_SM_FREE(pcUserMail);
				}
			}

			/*get task name and comments of rejected task*/
			if (tc_strcmp(pcTaskTypeName, "EPMReviewTask") == 0 && tc_strcmp(pcTaskResult, "Rejected") == 0)
			{
				int		iAllSubTasks = 0;
				tag_t* ptAllSubTaskTags = NULL;

				CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks], &iAllSubTasks, &ptAllSubTaskTags));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "object_name", &pcTaskName));
				for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
				{
					char* pcSubTaskTypeName = NULL;

					CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
					if (tc_strcmp(pcSubTaskTypeName, "EPMPerformSignoffTask") == 0)
					{
						CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "comments", &pcComments));
					}
					if (tc_strcmp(pcSubTaskTypeName, "EPMSelectSignoffTask") == 0)
					{
						int		iMailAttachs = 0;
						tag_t* ptMailTagList = NULL;

						//get signoff attachments
						CHECK_ITK(iRetCode, EPM_ask_attachments(ptAllSubTaskTags[iLoopAllSubTasks], EPM_signoff_attachment, &iMailAttachs, &ptMailTagList));
						for (int iUsers = 0; iUsers < iMailAttachs; iUsers++)
						{
							tag_t		tUser = NULLTAG;
							char* pcUserMail = NULL;
							CHECK_ITK(iRetCode, AOM_ask_value_tag(ptMailTagList[iUsers], "fnd0Performer", &tUser));
							if (tUser != 0)
							{
								CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
								//fprintf(fMailAddress,"%s",pcUserMail);
								if (tc_strcmp(pcUserMail, "") != NULL)
								{
									if (szRejMailList.size() > 2 && szRejMailList.find(",") == std::string::npos)
									{
										szRejMailList.append(",");
									}

									szRejMailList.append(pcUserMail);
									if (iMailAttachs > iUsers + 1)
									{
										szRejMailList.append(", ");
									}
								}
								SAFE_SM_FREE(pcUserMail);
							}
						}
					}
					SAFE_SM_FREE(pcSubTaskTypeName);
				}
			}

			if (isNotifyAll && tc_strcmp(pcTaskTypeName, "EPMAcknowledgeTask") == 0 && tc_strcmp(pcTaskState, "Completed") == 0)
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
						tag_t userTag = NULL;

						//get signoff attachments
						CHECK_ITK(iRetCode, EPM_ask_attachments(ptAllSubTaskTags[iLoopAllSubTasks], EPM_signoff_attachment, &iNumAttachs, &ptAttachsTagList));
						for (int iUsers = 0; iUsers < iNumAttachs; iUsers++)
						{
							tag_t		tUser = NULLTAG;
							char* pcUserMail = NULL;
							CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAttachsTagList[iUsers], "fnd0Performer", &tUser));
							CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
							//fprintf(fMailAddress,"%s",pcUserMail);
							if (tc_strcmp(pcUserMail, "") != NULL)
							{
								if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
								{
									CHECK_ITK(iRetCode, SA_find_user2(pcUserMail, &userTag));
									if (userTag != 0)
										szMailList.push_back(pcUserMail);
								}
							}
						}
					}
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
			tag_t userTag = NULL;
			CHECK_ITK(iRetCode, EPM_get_user_email_addr(tInitiator, &pcUserMail));
			if (tc_strcmp(pcUserMail, "") != NULL)
			{
				if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
				{
					CHECK_ITK(iRetCode, SA_find_user2(pcUserMail, &userTag));
					if (userTag != 0)
						szMailList.push_back(pcUserMail);
				}
			}
		}

		/*reading mail ids from file*/
		if (tc_strcmp(pcMailRecp, "") != 0 && tc_strlen(pcMailRecp) > 0)
		{
			string		szLine;
			ifstream	infile;
			infile.open(pcMailRecp);
			/*reading input file line by line*/
			while (getline(infile, szLine))
			{
				char* pcLine = NULL;
				tag_t userTag = NULL;
				pcLine = (char*)MEM_alloc((int)(szLine.length() + 1) * sizeof(char));
				tc_strcpy(pcLine, szLine.c_str());
				CHECK_ITK(iRetCode, SA_find_user2(pcLine, &userTag));
				if (userTag != 0)
					szMailList.push_back(pcLine);
			}
		}

		for (int iInd = 0; iInd < iTgt; iInd++)
		{
			bool check = false;
			char* pcObjType = NULL;
			CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcObjType));

			std::vector<std::string> objectTypes = split_string(pcReqType, ';');
			for (int i = 0; i < objectTypes.size(); i++)
			{
				std::string objectType = objectTypes[i];
				ltrim(objectType);
				rtrim(objectType);
				if (tc_strcmp(pcObjType, objectType.c_str()) == 0)
					check = true;
			}

			if (check) {
				char* pcDesc = NULL;
				char* pcSynopsis = NULL;
				char* pcObjString = NULL;
				char* pcItemID = NULL;
				char* pcRevID = NULL;
				tag_t tChangeItem = NULLTAG;
				tag_t tClass = NULLTAG;
				tag_t tRevClass = NULLTAG;
				string szTgtParts = "";

				CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iInd], &tChangeItem));
				CHECK_ITK(iRetCode, ITEM_ask_id2(tChangeItem, &pcItemID));
				CHECK_ITK(iRetCode, ITEM_ask_rev_id2(ptTgt[iInd], &pcRevID));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_name", &pcSynopsis));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_desc", &pcDesc));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_string", &pcObjString));

				CHECK_ITK(iRetCode, STRNG_replace_str(pcMailSubject, "$TARGET", pcItemID, &pcNewMailSubject));

				//Over view
				mailBody << "<html><meta charset='UTF-8'>";
				mailBody << genHtmlTitle("Following are the details of object:");
				//mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:#448DA6'>Following are the details of object:<o:p></o:p></span></b></p></div>";
				mailBody << "<table class=MsoNormalTable border=0 cellspacing=3 cellpadding=0>";
				mailBody << "<tr>" << genHtmlPropertyName("ID: ") << genHtmlPropertyValue(pcItemID == NULL ? "" : pcItemID) << "</tr>";
				mailBody << "<tr>" << genHtmlPropertyName("Name: ") << genHtmlPropertyValue(pcSynopsis == NULL ? "" : pcSynopsis) << "</tr>";
				mailBody << "<tr>" << genHtmlPropertyName("Description: ") << genHtmlPropertyValue(process_desc == NULL ? "" : process_desc) << "</tr>";
				//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>ID: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (pcItemID == NULL ? "" : pcItemID) << "/" << (pcRevID == NULL ? "" : pcRevID) << " <o:p></o:p></span></p></td></tr>";
				//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Name: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (pcSynopsis == NULL ? "" : pcSynopsis) << " <o:p></o:p></span></p></td></tr>";
				//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Description: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (process_desc == NULL ? "" : process_desc) << " <o:p></o:p></span></p></td></tr>";
				if (tc_strlen(process_desc) > 2)
					mailBody << "<tr>" << genHtmlPropertyName("Process Description: ") << genHtmlPropertyValue(process_desc == NULL ? "" : process_desc) << "</tr>";
				//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Process Description: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (process_desc == NULL ? "" : process_desc) << " <o:p></o:p></span></p></td></tr>";
				if (!isNotifyAll) {
					if (tc_strcmp(pcProcessSts, "Rejected") == 0)
					{
						TC_write_syslog("Rejected case");
						mailBody << "<tr>" << genHtmlPropertyName("Rejected By: ") << genHtmlPropertyValue(szRejMailList.c_str()) << "</tr>";
						mailBody << "<tr>" << genHtmlPropertyName("Rejected Task: ") << genHtmlPropertyValue(pcTaskName == NULL ? "" : pcTaskName) << "</tr>";
						mailBody << "<tr>" << genHtmlPropertyName("Rejected Comment: ") << genHtmlPropertyValue(pcComments == NULL ? "" : pcComments) << "</tr>";
						//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Rejected By: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << szRejMailList.c_str() << " <o:p></o:p></span></p></td></tr>";
						//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Rejected Task: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (pcTaskName == NULL ? "" : pcTaskName) << " <o:p></o:p></span></p></td></tr>";
						//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Rejected Comment: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (pcComments == NULL ? "" : pcComments) << " <o:p></o:p></span></p></td></tr>";
					}
					else if (tc_strcmp(pcProcessSts, "Rerouted") == 0)
					{
						TC_write_syslog("Rerouted case");
						char* reroutedPerformerMail = NULL;
						char* reroutedTaskName = NULL;
						char* reroutedComment = NULL;
						tag_t reroutedPerformer = NULLTAG;
						CHECK_ITK(iRetCode, AOM_ask_value_tag(msg.task, "fnd0Performer", &reroutedPerformer));
						CHECK_ITK(iRetCode, AOM_ask_value_string(msg.task, "object_name", &reroutedTaskName));
						CHECK_ITK(iRetCode, AOM_ask_value_string(msg.task, "comments", &reroutedComment));
						CHECK_ITK(iRetCode, EPM_get_user_email_addr(reroutedPerformer, &reroutedPerformerMail));

						mailBody << "<tr>" << genHtmlPropertyName("Rerouted By: ") << genHtmlPropertyValue(reroutedPerformerMail == NULL ? "" : reroutedPerformerMail) << "</tr>";
						mailBody << "<tr>" << genHtmlPropertyName("Rerouted Task: ") << genHtmlPropertyValue(reroutedTaskName == NULL ? "" : reroutedTaskName) << "</tr>";
						mailBody << "<tr>" << genHtmlPropertyName("Rerouted Comment: ") << genHtmlPropertyValue(reroutedComment == NULL ? "" : reroutedComment) << "</tr>";
						//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Rerouted By: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (reroutedPerformerMail == NULL ? "" : reroutedPerformerMail) << " <o:p></o:p></span></p></td></tr>";
						//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Rerouted Task: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (reroutedTaskName == NULL ? "" : reroutedTaskName) << " <o:p></o:p></span></p></td></tr>";
						//mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Rerouted Comment: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (reroutedComment == NULL ? "" : reroutedComment) << " <o:p></o:p></span></p></td></tr>";

						SAFE_MEM_free(reroutedPerformerMail);
						SAFE_MEM_free(reroutedTaskName);
						SAFE_MEM_free(reroutedComment);
					}
				}
				mailBody << "</table>";

				//Target
				mailBody << "<p><span>&nbsp;</span></p>";
				mailBody << genHtmlTitle("List of Target");
				//mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:#448DA6'>List of Target:<o:p></o:p></span></b></p></div>";
				mailBody << "<table class=MsoNormalTable border=1 cellspacing=0 cellpadding=0 width='100%' style='width:100.0%;border-collapse:collapse;border:none'>";
				mailBody << "<tr>" << genHtmlTableHeader("Name") << genHtmlTableHeader("Type") << "</tr>";
				//mailBody << "<td style='border:solid gray 1.0pt;background:#F0F0F0;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><b><span style='font-family:'Arial',sans-serif;color:gray'>Name<o:p></o:p></span></b></p></td>";
				//mailBody << "<td style='border:solid gray 1.0pt;border-left:none;background:#F0F0F0;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><b><span style='font-family:'Arial',sans-serif;color:gray'>Type<o:p></o:p></span></b></p></td></tr>";

				for (int iPrt = 0; iPrt < iTgt; iPrt++)
				{
					char* pcName = NULL;
					char* pcTgtType = NULL;

					CHECK_ITK(iRetCode, WSOM_ask_id_string(ptTgt[iPrt], &pcName));
					CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iPrt], &pcTgtType));

					mailBody << "<tr>" << genHtmlTableBody(pcName) << genHtmlTableBody(pcTgtType) << "</tr>";
					//mailBody << "<tr><td style='border:solid gray 1.0pt;border-top:none;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><span style='font-family:'Arial',sans-serif'>" << pcName << "<o:p></o:p></span></p></td>";
					//mailBody << "<td style='border-top:none;border-left:none;border-bottom:solid gray 1.0pt;border-right:solid gray 1.0pt;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><span style='font-family:'Arial',sans-serif'>" << pcTgtType << "<o:p></o:p></span></p></td></tr>";

					SAFE_MEM_free(pcName);
					SAFE_MEM_free(pcTgtType);
				}

				mailBody << "</table>";
				mailBody << "<p><span>&nbsp;</span></p>";
				mailBody << genHtmlNote("This email was sent from Teamcenter. Please do not reply.");
				//mailBody << "<div><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:gray'>This email was sent from Teamcenter. Please do not reply.<o:p></o:p></span></b></p></div>";
				mailBody << "</html>";

				SAFE_MEM_free(pcDesc);
				SAFE_MEM_free(pcSynopsis);
				SAFE_MEM_free(pcItemID);
				SAFE_MEM_free(pcObjType);
			}
		}
		SAFE_MEM_free(process_desc);
	}
	SAFE_MEM_free(pcTaskName);
	SAFE_MEM_free(pcComments);

	sendEmail(pcNewMailSubject, mailBody.str(), szMailList, isDebug);
	TC_write_syslog("\n [VF] LEAVE %s", __FUNCTION__);
	return iRetCode;
}