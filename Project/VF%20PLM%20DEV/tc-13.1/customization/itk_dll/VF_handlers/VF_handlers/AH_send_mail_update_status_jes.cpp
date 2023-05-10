
#include "Vinfast_Custom.h"

extern int AH_send_mail_update_status_jes(EPM_action_message_t msg)
{

	int iRetCode = ITK_ok;
	boolean isDebug = false;
	std:string subject = "";
	std::string mailBody = "";
	std::vector<string> szMailList;
	/*getting root task*/
	tag_t tRootTask = NULLTAG;
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int			iAllTasks = 0;
		int			iTgt = 0;
		tag_t* ptTgt = NULL;
		tag_t* ptAllTaskTags = NULL;
		tag_t		 tInitiator = NULL;

		CHECK_ITK(iRetCode, EPM_ask_sub_tasks(tRootTask, &iAllTasks, &ptAllTaskTags));
		/*getting mail id of all participants from review task*/
		for (int iLoopAllTasks = 0; iLoopAllTasks < iAllTasks; iLoopAllTasks++)
		{
			char* pcTaskTypeName = NULL;
			int	iAllSubTasks = 0;
			tag_t* ptAllSubTaskTags = NULL;
			CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks], &iAllSubTasks, &ptAllSubTaskTags));
			for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
			{
				char* pcSubTaskTypeName = NULL;
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_type", &pcTaskTypeName));
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
						//fprintf(fMailAddress,"%s",pcUserMail);
						if (tc_strcmp(pcUserMail, "") != NULL)
						{
							if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
							{
								szMailList.push_back(pcUserMail);
							}
							SAFE_MEM_free(pcUserMail);
						}
					}

				}
			}
			SAFE_MEM_free(ptAllSubTaskTags);
		}


		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		for (int iInd = 0; iInd < iTgt; iInd++) {
			tag_t iman_rela_type = NULLTAG;
			char* rev_id = NULL;
			char* CMImplements = NULL;
			char* DCR_Number = NULL;
			char* object_string = NULL;
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iTgt], "current_revision_id", &rev_id));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iTgt], "CMImplements", &CMImplements));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iTgt], "object_string", &object_string));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iTgt], "vf6_Released_DCR_Number", &DCR_Number));

			CHECK_ITK(iRetCode, GRM_find_relation_type("IMAN_requirement", &iman_rela_type));
			if (strcmp(rev_id, "") != 0)
			{
				if (iman_rela_type != NULLTAG)
				{
					subject.append("[JES-SOS-Update-Notification]");
					subject.append("<");
					subject.append(rev_id);
					subject.append("::");
					subject.append(CMImplements);
					subject.append("::");
					subject.append(object_string);
					subject.append(">");
					subject.append("implemented in Teamcenter");
				}
				else if(strcmp(DCR_Number,"") != 0) {
					char* dcr = NULL;
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iTgt], "vf6_Released_DCR_Number", &dcr));
					subject.append("[JES-SOS-Update-Notification]");
					subject.append("<");
					subject.append(rev_id);
					subject.append("::");
					subject.append(DCR_Number);
					subject.append("::");
					subject.append(object_string);
					subject.append(">");
					subject.append("implemented in Teamcenter");

				}
				mailBody.append("<");
				mailBody.append(rev_id);
				mailBody.append("::");
				mailBody.append(CMImplements);
				mailBody.append("::");
				mailBody.append(object_string);
				mailBody.append(">");
				mailBody.append("implemented in Teamcenter, please checkand update JES / SOS");
				char* _subject = const_cast<char*>(subject.c_str());
				sendEmail(_subject, mailBody, szMailList, isDebug);
			}
			
		}
		
		SAFE_MEM_free(ptAllTaskTags);
		SAFE_MEM_free(ptTgt);
		return iRetCode;
	}
}