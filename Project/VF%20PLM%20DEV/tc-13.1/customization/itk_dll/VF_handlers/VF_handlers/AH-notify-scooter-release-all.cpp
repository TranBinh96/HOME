#include "Vinfast_Custom.h"

extern int AH_notify_scooter_relase_all(EPM_action_message_t msg)
{
	int	iRetCode = ITK_ok;
	int	iNumArgs = 0;
	tag_t tRootTask = NULLTAG;
	char* pcMailSubject = NULL;
	char* pcFile = NULL;
	char* processName = NULL;
	char* taskUid = NULL;

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
				else if (tc_strcmp(pcFlag, "file") == 0)
				{
					pcFile = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcFile, pcValue);
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

	/*getting root task*/
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int	iAllTasks = 0;
		int	iTgt = 0;
		tag_t* ptTgt = NULL;
		tag_t* ptAllTaskTags = NULL;
		tag_t tInitiator = NULL;
		string initiatorMail;

		CHECK_ITK(iRetCode, EPM_ask_sub_tasks(tRootTask, &iAllTasks, &ptAllTaskTags));
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		CHECK_ITK(iRetCode, AOM_ask_value_tag(tRootTask, "fnd0WorkflowInitiator", &tInitiator));
		CHECK_ITK(iRetCode, AOM_ask_value_string(tRootTask, "object_name", &processName));
		ITK__convert_tag_to_uid(msg.task, &taskUid);

		for (int iLoopAllTasks = 0; iLoopAllTasks < iAllTasks; iLoopAllTasks++)
		{
			char* pcTaskTypeName = NULL;
			char* pcTaskName = NULL;
			char* pcTaskState = NULL;

			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_type", &pcTaskTypeName));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "current_name", &pcTaskName));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_state", &pcTaskState));

			if (tc_strcmp(pcTaskState, "Completed") == 0)
			{
				if (tc_strcmp(pcTaskTypeName, "EPMReviewTask") == 0)
				{
					int iAllSubTasks = 0;
					tag_t* ptAllSubTaskTags = NULL;

					CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks], &iAllSubTasks, &ptAllSubTaskTags));
					for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
					{
						char* pcSubTaskTypeName = "";
						CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
						if (tc_strcmp(pcSubTaskTypeName, "EPMSelectSignoffTask") == 0)
						{
							int	iNumAttachs = 0;
							tag_t* ptAttachsTagList = NULL;

							//get signoff attachments
							CHECK_ITK(iRetCode, EPM_ask_attachments(ptAllSubTaskTags[iLoopAllSubTasks], EPM_signoff_attachment, &iNumAttachs, &ptAttachsTagList));
							for (int iUsers = 0; iUsers < iNumAttachs; iUsers++)
							{
								tag_t tUser = NULLTAG;
								char* pcUserMail = NULL;
								CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAttachsTagList[iUsers], "fnd0Performer", &tUser));
								CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
								if (tc_strcmp(pcUserMail, "") != NULL)
									szMailList.push_back(pcUserMail);
							}
						}
						SAFE_MEM_free(pcSubTaskTypeName);
					}
				}
				/*getting mail id of participants from do task*/
				else if (tc_strcmp(pcTaskTypeName, "EPMDoTask") == 0)
				{
					tag_t		tUser = NULLTAG;
					char* pcUserMail = NULL;

					CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAllTaskTags[iLoopAllTasks], "fnd0Performer", &tUser));
					CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
					if (tc_strcmp(pcUserMail, "") != NULL)
					{
						szMailList.push_back(pcUserMail);
					}
				}
				//getting mail id of participants from ConditionTask
				else if (tc_strcmp(pcTaskTypeName, "EPMConditionTask") == 0)
				{
					tag_t		tUser = NULLTAG;
					char* pcUserMail = NULL;
					CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAllTaskTags[iLoopAllTasks], "fnd0Performer", &tUser));
					CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
					if (tc_strcmp(pcUserMail, "") != NULL)
					{
						szMailList.push_back(pcUserMail);
					}
				}
				//Acknowledge Task
				else if (tc_strcmp(pcTaskTypeName, "EPMAcknowledgeTask") == 0)
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
								if (tc_strcmp(pcUserMail, "") != NULL)
								{
									szMailList.push_back(pcUserMail);
								}
							}
						}
					}
				}
			}
		}

		/*getting mail id of initiator of workflow*/
		if (tInitiator != NULLTAG)
		{
			char* pcUserMail = NULL;
			CHECK_ITK(iRetCode, EPM_get_user_email_addr(tInitiator, &pcUserMail));
			initiatorMail.assign(pcUserMail);
			if (tc_strcmp(pcUserMail, "") != NULL)
			{
				szMailList.push_back(pcUserMail);
			}
			SAFE_MEM_free(pcUserMail);
		}

		/*reading mail ids from file*/
		if (tc_strcmp(pcFile, "") != 0)
		{
			string		szLine;
			ifstream	infile;
			infile.open(pcFile);
			/*reading input file line by line*/
			while (getline(infile, szLine))
			{
				char* pcLine = NULL;

				pcLine = (char*)MEM_alloc((int)(szLine.length() + 1) * sizeof(char));
				tc_strcpy(pcLine, szLine.c_str());
				szMailList.push_back(szLine);
			}
			infile.close();
		}

		char* vehicle_line = NULL;

		for (int iInd = 0; iInd < iTgt; iInd++)
		{
			char* pcObjType = NULL;
			CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcObjType));
			if (tc_strcmp(pcObjType, "VF3_Scooter_partRevision") == 0)
			{
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf4_es_model_veh_line_cp", &vehicle_line));
				break;
			}
			SAFE_MEM_free(pcObjType);
		}
		//Over view
		mailBody << "<html><meta charset='UTF-8'>";
		mailBody << genHtmlTitle("Following are the details of product release process:");// "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:#448DA6'>Following are the details of product release process:<o:p></o:p></span></b></p></div>";
		mailBody << "<table class=MsoNormalTable border=0 cellspacing=3 cellpadding=0>";
		mailBody << "<tr>" << genHtmlPropertyName("Process Name: ") << genHtmlPropertyValue(processName == NULL ? "" : processName) << "</tr>";
		mailBody << "<tr>" << genHtmlPropertyName("Process Initiator: ") << genHtmlPropertyValue(initiatorMail) << "</tr>";
		mailBody << "<tr>" << genHtmlPropertyName("EScooter Model/Vehicle Line: ") << genHtmlPropertyValue((vehicle_line == NULL ? "" : vehicle_line)) << "</tr>";
		/*mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Process Name: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (processName == NULL ? "" : processName) << "<o:p></o:p></span></p></td></tr>";
		mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>Process Initiator: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << initiatorMail << " <o:p></o:p></span></p></td></tr>";
		mailBody << "<tr><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif;color:gray'>EScooter Model/Vehicle Line: <o:p></o:p></span></p></td><td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'>" << (vehicle_line == NULL ? "" : vehicle_line) << "<o:p></o:p></span></p></td></tr>";*/
		mailBody << "</table>";

		//Select the preferred
		char* awcUrl = NULL;
		CHECK_ITK(iRetCode, PREF_ask_char_value("ActiveWorkspaceHosting.URL", 0, &awcUrl));
		std::string awc_link = "https://tcweb.vinfast.vn/#/com.siemens.splm.clientfx.tcui.xrt.showObject?uid=";
		if (tc_strlen(awcUrl) > 2)
		{
			awc_link = awcUrl;
			if (awc_link.at(awc_link.size() - 1) != '/') awc_link.append("/");
			awc_link.append("#/com.siemens.splm.clientfx.tcui.xrt.showObject?uid=");
		}
		TC_write_syslog("before awc_link: %s", awc_link.c_str());
		awc_link.append(awc_link.c_str());
		mailBody << "<p><span>&nbsp;</span></p>";
		mailBody << genHtmlTitle("Select the preferred client to view the task:");
		mailBody << genHtmlLink(awc_link.c_str(), "Active Workspace");
		/*mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:#448DA6'>Select the preferred client to view the task:<o:p></o:p></span></b></p></div>";
		mailBody << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><span style='font-family:'Arial',sans-serif'><a href='"<< awc_link.c_str() <<"'>Active Workspace</a><o:p></o:p></span></p></div>";*/

		//Target
		mailBody << "<p><span>&nbsp;</span></p>";
		mailBody << genHtmlTitle("Target:");// "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:#448DA6'>Target:<o:p></o:p></span></b></p></div>";
		mailBody << "<table class=MsoNormalTable border=1 cellspacing=0 cellpadding=0 width='100%' style='width:100.0%;border-collapse:collapse;border:none'><tr>";
		mailBody << "<tr>" << genHtmlTableHeader("ID") << genHtmlTableHeader("Type") << "</tr>";// "<td style='border:solid gray 1.0pt;background:#F0F0F0;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><b><span style='font-family:'Arial',sans-serif;color:gray'>ID<o:p></o:p></span></b></p></td>";
		//mailBody << "<td style='border:solid gray 1.0pt;border-left:none;background:#F0F0F0;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><b><span style='font-family:'Arial',sans-serif;color:gray'>Type<o:p></o:p></span></b></p></td></tr>";

		for (int iPrt = 0; iPrt < iTgt; iPrt++)
		{
			char* itemID = NULL;
			char* objectType = NULL;

			CHECK_ITK(iRetCode, WSOM_ask_id_string(ptTgt[iPrt], &itemID));
			CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iPrt], &objectType));

			mailBody << "<tr>" << genHtmlTableBody(itemID == NULL ? "" : itemID) << genHtmlTableBody(objectType == NULL ? "" : objectType) << "</tr>";
			//mailBody << "<tr><td style='border:solid gray 1.0pt;border-top:none;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><span style='font-family:'Arial',sans-serif'>" << (itemID == NULL ? "" : itemID) << "<o:p></o:p></span></p></td>";
			//mailBody << "<td style='border-top:none;border-left:none;border-bottom:solid gray 1.0pt;border-right:solid gray 1.0pt;padding:1.5pt 1.5pt 1.5pt 1.5pt'><p class=MsoNormal align=center style='text-align:center'><span style='font-family:'Arial',sans-serif'>" << (objectType == NULL ? "" : objectType) << "<o:p></o:p></span></p></td></tr>";

			SAFE_MEM_free(itemID);
			SAFE_MEM_free(objectType);
		}

		mailBody << "</table>";
		mailBody << "<p><span>&nbsp;</span></p>";
		mailBody << genHtmlNote("This email was sent from Teamcenter.");// "<div><p class=MsoNormal><b><span style='font-family:'Arial',sans-serif;color:gray'>This email was sent from Teamcenter.<o:p></o:p></span></b></p></div>";
		mailBody << "</html>";

		SAFE_MEM_free(awcUrl);
		SAFE_MEM_free(taskUid);
		SAFE_MEM_free(processName);
	}

	sendEmail(pcMailSubject, mailBody.str(), szMailList, false);
	TC_write_syslog("\n [VF] LEAVE %s", __FUNCTION__);
	return iRetCode;
}