/*******************************************************************************
File         : AH_send_email_to_all.cpp

Description  : To send mail notification

Input        : None

Output       : None

Author       : Siemens

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Feb,27 2019     1.0         Kantesh		Initial Creation
April, 21 2019	2.0			Mohammed	Adding Acar RecipientList

*******************************************************************************/
#include "Vinfast_Custom.h"

//std::string replace_str(std::string & str, const std::string & from, const std::string & to);

extern int AH_notify(EPM_action_message_t msg)
{
	int			iRetCode				= ITK_ok;
	int			iNumArgs				= 0;
	int			iPrefCount				= 0;
	int			portPref_count			= 0;
	tag_t		tRootTask				= NULLTAG;
	char		*pcMailSubject			= NULL;
	char		*pcRecipients			= NULL;
	char		*pcFlag					= NULL;	
	char		*pcValue				= NULL;	
	char		*portNo					= NULL;
	char		*pcServerName			= NULL;
	char		*pcMailBodyFileName		= NULL;
	char		*pcRecipient			= NULL;
	char		*current_task_name		= NULL;
	char		*object_type			= NULL;
	char		*sub_object_type		= NULL;
	char		*process_desc			= NULL;
	char		*process_job_name		= NULL;
	char		*date_string			= NULL;
	int			count_task				= NULL;
	int			signoff_count			= NULL;
	tag_t		current_job			= NULLTAG;
	tag_t		*subtasks				= NULLTAG;
	tag_t		*attachment				= NULLTAG;
	date_t		task_due_date			;
	tag_t			_performer			= NULLTAG;
	char		caMailDetails[BUFSIZ +1];
	char		mailDetails[BUFSIZ +1];
	char		pcRecipientsList[BUFSIZ +1];

	TC_write_syslog("\n[vf] ENTER %s", __FUNCTION__);
	iNumArgs = TC_number_of_arguments(msg.arguments);

	std::stringstream mailBody;
	std::vector<string> szMailList;

	if (iNumArgs >= 1)
	{
		for(int i = 0; i < iNumArgs; i++)
		{
			/*getting arguments*/
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue );
			if ( iRetCode == ITK_ok )
			{
				if (tc_strcasecmp(pcFlag, "subject") == 0 && pcValue != NULL)
				{
					pcMailSubject = (char*) MEM_alloc(( (int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy( pcMailSubject, pcValue);
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

	//Send notify review for review task, do task, condition task, acknowledge task only
	FILE	*fMailBodyFile			= NULL;
	int obj_count					= 0;
	tag_t	*root_attachments		= NULLTAG;
	char	*obj_type1				= NULL;
	char	*effective_proposal		= NULL;
	char	*instruction			= NULL;
	char	*uid					= NULL;
	char	*object_name			= NULL;
	char	*taskName    			= NULL;
	char	*usr_email = NULL;

	map<string, string> targetObjectMaps;

	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));

	CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &obj_count, &root_attachments));
	for(int k=0;k< obj_count; k++)
	{
		char *type = NULL;

		CHECK_ITK(iRetCode, AOM_ask_value_string(root_attachments[k],"object_name", &object_name));
		CHECK_ITK(iRetCode, WSOM_ask_object_type2(root_attachments[k], &obj_type1));
		CHECK_ITK(iRetCode, AOM_UIF_ask_value(root_attachments[k], "object_type", &type));
		targetObjectMaps[object_name] = type;
		if(tc_strcmp(obj_type1, "Vf6_ECRRevision") == 0 || tc_strcmp(obj_type1, "Vf6_ECNRevision") == 0 )
		{
			CHECK_ITK(iRetCode, AOM_ask_value_string(root_attachments[k], "vf6_effective_proposal", &effective_proposal));
		}
		SAFE_MEM_free(obj_type1);
		SAFE_MEM_free(object_name);
		SAFE_MEM_free(type);
	}

	CHECK_ITK(iRetCode, WSOM_ask_object_type2(msg.task, &object_type));
	CHECK_ITK(iRetCode, EPM_ask_job(msg.task, &current_job));
	CHECK_ITK(iRetCode, AOM_ask_value_string(msg.task, "current_desc", &instruction));
	string instructionMailContent(instruction);
	//instructionMailContent = replace_str(instructionMailContent, "\n", "</br>");
	ITK__convert_tag_to_uid(msg.task, &taskName);
	CHECK_ITK(iRetCode, AOM_ask_value_string(current_job, "current_desc", &process_desc));
	CHECK_ITK(iRetCode, AOM_ask_value_string(current_job, "current_name", &process_job_name));
	if(tc_strcmp(object_type, "EPMDoTask") == 0 || tc_strcmp(object_type, "EPMConditionTask") == 0)
	{

		//get object_name, due_date, performer
		CHECK_ITK(iRetCode, AOM_ask_value_string(msg.task, "object_name", &current_task_name));
		CHECK_ITK(iRetCode, AOM_ask_value_date(msg.task, "due_date", &task_due_date));
		CHECK_ITK(iRetCode, DATE_date_to_string(task_due_date, "%Y-%m-%d %H:%M:%S", &date_string));
		CHECK_ITK(iRetCode, AOM_ask_value_tag(msg.task, "fnd0Performer", &_performer));
		CHECK_ITK(iRetCode, EPM_get_user_email_addr(_performer, &usr_email));
		ITK__convert_tag_to_uid(msg.task, &uid);

		//get process name, process description
	}
	else if(tc_strcmp(object_type, "EPMPerformSignoffTask") == 0)
	{
		tag_t parent_task = NULLTAG;
		CHECK_ITK(iRetCode, EPM_ask_parent_task(msg.task, &parent_task));
		CHECK_ITK(iRetCode, EPM_ask_sub_tasks(parent_task, &count_task, &subtasks));
		for(int i = 0; i < count_task; i++)
		{
			CHECK_ITK(iRetCode, WSOM_ask_object_type2(subtasks[i], &sub_object_type));
			if(tc_strcmp(sub_object_type, "EPMPerformSignoffTask") == 0)
			{
				CHECK_ITK(iRetCode, AOM_ask_value_string(subtasks[i], "object_name", &current_task_name));
				CHECK_ITK(iRetCode, AOM_ask_value_date(subtasks[i], "due_date", &task_due_date));
				CHECK_ITK(iRetCode, DATE_date_to_string(task_due_date, "%Y-%m-%d %H:%M:%S", &date_string));
				ITK__convert_tag_to_uid(subtasks[i], &uid);
			}
			if(tc_strcmp(sub_object_type, "EPMSelectSignoffTask") == 0)
			{
				CHECK_ITK(iRetCode, EPM_ask_attachments(subtasks[i], EPM_signoff_attachment, &signoff_count, &attachment));
				for(int j=0;j < signoff_count; j++)
				{
					tag_t tUser = NULLTAG;
					char *user_email = NULL;
					CHECK_ITK(iRetCode, AOM_ask_value_tag(attachment[j], "fnd0Performer", &tUser));
					CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &user_email));
					szMailList.push_back(user_email);
				}
			}
		}

	}
	else if (tc_strcmp(object_type,"EPMAcknowledgeTask") == 0) 
	{
		int		iAllSubTasks			= 0;
		tag_t	*ptAllSubTaskTags		= NULL;

		tag_t parent_task = NULLTAG;
		CHECK_ITK(iRetCode, EPM_ask_parent_task(msg.task, &parent_task));

		CHECK_ITK(iRetCode,EPM_ask_sub_tasks(parent_task,&iAllSubTasks,&ptAllSubTaskTags));
		for(int iLoopAllSubTasks =0 ; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
		{	
			char *pcSubTaskTypeName		= NULL;

			CHECK_ITK(iRetCode,AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
			if(tc_strcmp(pcSubTaskTypeName,"EPMSelectSignoffTask")==0)
			{
				int		iNumAttachs				= 0;
				tag_t	*ptAttachsTagList		= NULL;

				//get signoff attachments
				CHECK_ITK(iRetCode,EPM_ask_attachments(ptAllSubTaskTags[iLoopAllSubTasks], EPM_signoff_attachment, &iNumAttachs, &ptAttachsTagList));
				for(int iUsers = 0; iUsers < iNumAttachs; iUsers++)
				{
					tag_t		tUser				= NULLTAG;
					char		*pcUserMail			= NULL;
					CHECK_ITK(iRetCode,AOM_ask_value_tag(ptAttachsTagList[iUsers],"fnd0Performer",&tUser));
					CHECK_ITK(iRetCode,EPM_get_user_email_addr(tUser, &pcUserMail));
					//fprintf(fMailAddress,"%s\n",pcUserMail);
					if(tc_strcmp(pcUserMail,"") != NULL)
					{
						szMailList.push_back(pcUserMail);
					}
				}
			}
		}
	}

	if (usr_email != NULL)
	{
		szMailList.push_back(usr_email);
	}

	//prepare email to sending
	char *fullpath = NULL;

	char *awcUrl = NULL;
	CHECK_ITK(iRetCode, PREF_ask_char_value("ActiveWorkspaceHosting.URL", 0, &awcUrl));
	std::string awc_link  = "https://tcweb.vinfast.vn/#/com.siemens.splm.clientfx.tcui.xrt.showObject?uid=";
	if (tc_strlen(awcUrl) > 2)
	{
		awc_link = awcUrl;
		if (awc_link.at(awc_link.size() - 1) != '/') awc_link.append("/");
		awc_link.append("#/com.siemens.splm.clientfx.tcui.xrt.showObject?uid=");
	}

	mailBody << "<!DOCTYPE html><html><title>Change Process Email</title><body>";
	mailBody << "<p><strong>Overview:</strong></br>";
	mailBody << "<table border='1'><tbody><tr><td width='283'><strong>Current Task</strong></td><td width='500'>";
	mailBody << current_task_name ? current_task_name : "";
	mailBody << "</td></tr>";
	mailBody << "<tr><td width='283'><strong>Process Name</strong></td><td width='500'>";
	mailBody << process_job_name ? process_job_name : "";
	mailBody << "</td></tr>";
	mailBody << "<tr><td width='283'><strong>Due Date</strong></td><td width='500'>";
	mailBody << date_string ? date_string : "";
	mailBody << "</td></tr>";
	mailBody << "<tr><td width='283'><strong>Process Description</strong></td><td width='500'>";
	mailBody << process_desc ? process_desc : "";
	mailBody << "</td></tr>";
	mailBody << "<tr><td width='283'><strong>Instruction</strong></td><td width='500'>";
	mailBody << instructionMailContent;
	mailBody << "</td></tr>";
	mailBody << "<tr><td width='283'><strong>Target Implementation Date</strong></td><td width='500'>";
	mailBody << effective_proposal ? effective_proposal : "";
	mailBody << "</td></tr>";
	mailBody << "</tbody></table></p></br>";
	mailBody << "<p><strong>Process Targets:</strong></br>";
	mailBody << "<table border='1'><tr><td><strong>Target Name</strong></td><td><strong>Target Type</strong></td></tr>";
	for(auto targetObjecEntry : targetObjectMaps)
	{
		mailBody << "<tr>";
		mailBody << "<td>" << targetObjecEntry.first.c_str() << "</td>";
		mailBody << "<td>" << targetObjecEntry.second.c_str() << "</td>";
		mailBody << "</tr>";
	}
	mailBody << "</table></p></br>";
	mailBody << "<p><strong>Please select below link to open the task.</strong></br>";
	mailBody << "<a href='";
	mailBody << awc_link.c_str();
	mailBody << "'>Active Workspace</a></p></br>";
	mailBody << "<p><strong>This is auto generated email. Please don't reply</strong></p>";
	mailBody << "</body></html>";

	SAFE_MEM_free(current_task_name);
	SAFE_MEM_free(process_job_name);
	SAFE_MEM_free(date_string);
	SAFE_MEM_free(process_desc);
	SAFE_MEM_free(instruction);
	SAFE_MEM_free(effective_proposal);
	SAFE_MEM_free(object_name);
	SAFE_MEM_free(uid);
	SAFE_MEM_free(usr_email);
	SAFE_MEM_free(object_type);
	SAFE_MEM_free(sub_object_type);
	SAFE_MEM_free(fullpath);
	SAFE_MEM_free(pcMailSubject);
	SAFE_MEM_free(awcUrl);

	sendEmail(pcMailSubject, mailBody.str(), szMailList, false);
	TC_write_syslog("[vf] LEAVE %s", __FUNCTION__);
	return iRetCode;
}

