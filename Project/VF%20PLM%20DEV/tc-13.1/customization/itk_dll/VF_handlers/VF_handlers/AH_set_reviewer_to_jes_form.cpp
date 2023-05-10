#include "Vinfast_Custom.h"

extern int AH_set_reviewer_to_jes_form(EPM_action_message_t msg)
{
	int iRetCode = ITK_ok;
	boolean isDebug = false;

	char* pcMailSubject = NULL;
	char* pcFile = "";
	char* MailFile = "";
	char* argKey = nullptr;
	char* argValue = nullptr;
	std::string propertyName, targetObjectType;
	std::stringstream mailBody;
	std::vector<string> mailList;

	//get argument list
	const auto num_args = TC_number_of_arguments(msg.arguments);
	
	for (int i = 0; i < num_args; ++i)
	{
		CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &argKey, &argValue));
		if (iRetCode == ITK_ok)
		{
			if (tc_strcasecmp(argKey, "property") == 0 && argValue != nullptr)
			{
				propertyName.assign(argValue);
			}
			else if (tc_strcasecmp(argKey, "target_object_type") == 0 && argValue != nullptr)
			{
				targetObjectType.assign(argValue);
			}

		}
		SAFE_MEM_free(argKey);
		SAFE_MEM_free(argValue);
	}
	/*getting root task*/
	tag_t tRootTask = NULLTAG;
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int			iAllTasks = 0;
		int			iTgt = 0;
		tag_t* ptTgt = NULL;
		tag_t* ptAllTaskTags = NULL;
		char * taskType = NULL;

		CHECK_ITK(iRetCode, WSOM_ask_object_type2(msg.task, &taskType));
		if (strcmp(taskType, "EPMPerformSignoffTask") == 0) {
			int		iNumAttachs = 0;
			tag_t* ptAttachsTagList = NULL;
			tag_t* predessors = NULL;
			int predessorsCount = 0;
			CHECK_ITK(iRetCode, AOM_ask_value_tags(msg.task, "predecessors", &predessorsCount, &predessors));
			if (predessorsCount > 0)
			{
				tag_t signOffTask = predessors[0];
				//get signoff attachments
				CHECK_ITK(iRetCode, EPM_ask_attachments(signOffTask, EPM_signoff_attachment, &iNumAttachs, &ptAttachsTagList));
				for (int iUsers = 0; iUsers < iNumAttachs; iUsers++)
				{
					tag_t		tUser = NULLTAG;
					char* pcUserMail = NULL;
					CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAttachsTagList[iUsers], "fnd0Performer", &tUser));
					CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
					if (tc_strcmp(pcUserMail, "") != NULL)
					{
						if (std::find(mailList.begin(), mailList.end(), pcUserMail) == mailList.end())
						{
							mailList.push_back(pcUserMail);
						}
						SAFE_MEM_free(pcUserMail);
					}
				}
				SAFE_MEM_free(ptAttachsTagList);
				SAFE_MEM_free(predessors);
			}
		}	

		if (mailList.size() > 0) {
			//get target object
			int totalReferObj = 0;
			tag_t* allReferObj = nullptr;
			tag_t referObjNeeded = NULL;
			vector<tag_t> targetObjects;

			if (targetObjectType.empty() == false && targetObjectType[0] != '\n')
			{
				CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &totalReferObj, &allReferObj));
				for (int i = 0; i < totalReferObj && allReferObj != NULL; ++i)
				{
					char* objType = nullptr;
					CHECK_ITK(iRetCode, WSOM_ask_object_type2(allReferObj[i], &objType));
					if (tc_strcasecmp(objType, targetObjectType.c_str()) == 0)
					{
						referObjNeeded = allReferObj[i];
						targetObjects.push_back(referObjNeeded);
					}
					SAFE_MEM_free(objType);
				}
				SAFE_MEM_free(allReferObj);

				std::string szMailList;
				for (int mailListInx = 0; mailListInx < mailList.size(); mailListInx++)
				{
					szMailList.append(mailList[mailListInx]).append("\n");
				}

				for (int targetObjectInx = 0; targetObjectInx < targetObjects.size(); targetObjectInx++)
				{
					tag_t targetObject = targetObjects[targetObjectInx];
					CHECK_ITK(iRetCode, AOM_refresh(targetObject, TRUE));
					CHECK_ITK(iRetCode, AOM_set_value_string(targetObject, propertyName.c_str(), szMailList.c_str()));
					CHECK_ITK(iRetCode, AOM_save_with_extensions(targetObject));
					CHECK_ITK(iRetCode, AOM_refresh(targetObject, FALSE));
				}
				
				printf("\n-------------------ret=%d", iRetCode);
			}
		}
	}
	return iRetCode;
}
















