/*******************************************************************************
File         : AH_send_email_to_all_participants.cpp

Description  : To send mail notification all participants

Input        : None

Output       : None

Author       : Siemens

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Jul,10 2019     1.0         Kantesh		 Initial Creation

/*******************************************************************************/
#include "Vinfast_Custom.h"

extern int AH_send_email_to_all_participants(EPM_action_message_t msg)
{
	int	iRetCode = ITK_ok;
	int	iNumArgs = 0;
	int	iPrefCount = 0;
	int	portPref_count = 0;
	tag_t tRootTask = NULLTAG;
	char* pcMailSubject = NULL;
	char* pcRecipients = NULL;
	char* pcObjType = NULL;
	char* pcFile = NULL;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* MailFile = NULL;

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
				else if (tc_strcmp(pcFlag, "file") == 0)
				{
					pcFile = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcFile, pcValue);
				}
				//binhtt28 write log file mail send server
				else if (tc_strcmp(pcFlag, "maillist") == 0) {
					MailFile = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(MailFile, pcValue);
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
		int			iAllTasks = 0;
		int			iTgt = 0;
		tag_t* ptTgt = NULL;
		tag_t* ptAllTaskTags = NULL;
		tag_t		 tInitiator = NULL;

		CHECK_ITK(iRetCode, EPM_ask_sub_tasks(tRootTask, &iAllTasks, &ptAllTaskTags));
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		CHECK_ITK(iRetCode, AOM_ask_value_tag(msg.task, "owning_user", &tInitiator));
		/*getting mail id of all participants from review task*/
		for (int iLoopAllTasks = 0; iLoopAllTasks < iAllTasks; iLoopAllTasks++)
		{
			char* pcTaskTypeName = NULL;
			char* pcTaskName = NULL;
			char* pcTaskState = NULL;

			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_type", &pcTaskTypeName));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "current_name", &pcTaskName));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_state", &pcTaskState));

			if (tc_strcmp(pcTaskTypeName, "EPMReviewTask") == 0 && tc_strcmp(pcTaskState, "Completed") == 0)
			{
				int	iAllSubTasks = 0;
				tag_t* ptAllSubTaskTags = NULL;

				CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks], &iAllSubTasks, &ptAllSubTaskTags));
				for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
				{
					char* pcSubTaskTypeName = NULL;

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
							}
						}
					}
				}
			}

			/*getting mail id of participants from do task*/
			if (tc_strcmp(pcTaskTypeName, "EPMDoTask") == 0 && tc_strcmp(pcTaskState, "Completed") == 0)
			{
				tag_t tUser = NULLTAG;
				char* pcUserMail = NULL;

				CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAllTaskTags[iLoopAllTasks], "fnd0Performer", &tUser));
				CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
				if (tc_strcmp(pcUserMail, "") != NULL)
				{
					if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
					{
						szMailList.push_back(pcUserMail);
					}
				}
			}
			//getting mail id of participants from ConditionTask
			if (tc_strcmp(pcTaskTypeName, "EPMConditionTask") == 0 && tc_strcmp(pcTaskState, "Completed") == 0)
			{
				tag_t tUser = NULLTAG;
				char* pcUserMail = NULL;
				CHECK_ITK(iRetCode, AOM_ask_value_tag(ptAllTaskTags[iLoopAllTasks], "fnd0Performer", &tUser));
				CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &pcUserMail));
				if (tc_strcmp(pcUserMail, "") != NULL)
				{
					if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
					{
						szMailList.push_back(pcUserMail);
					}
				}
			}
			//Acknowledge Task
			if (tc_strcmp(pcTaskTypeName, "EPMAcknowledgeTask") == 0 && tc_strcmp(pcTaskState, "Completed") == 0)
			{
				int	iAllSubTasks = 0;
				tag_t* ptAllSubTaskTags = NULL;

				CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks], &iAllSubTasks, &ptAllSubTaskTags));
				for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
				{
					char* pcSubTaskTypeName = NULL;

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
			if (tc_strcmp(pcUserMail, "") != NULL)
			{
				if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end())
				{
					szMailList.push_back(pcUserMail);
				}
			}
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
				szMailList.push_back(pcLine);
			}
		}

		/*binhtt28 reading mail from */
		if (tc_strcmp(MailFile, "") != 0)
		{
			char* next_p;
			char seps[] = "; ,\t\n";
			char* chars_array = strtok_s(MailFile, seps, &next_p);
			while (chars_array)
			{
				if (std::find(szMailList.begin(), szMailList.end(), chars_array) == szMailList.end())
				{
					szMailList.push_back(chars_array);
				}
				chars_array = strtok_s(NULL, ",", &next_p);
			}
		}

		vector<string> szProbType;
		vector<string> szSolType;
		vector<string> szMEType;

		for (int iInd = 0; iInd < iTgt; iInd++)
		{
			char* pcObjType = NULL;
			char* pcDesc = NULL;
			char* pcSynopsis = NULL;
			char* pcObjString = NULL;
			char* pcItemID = NULL;
			char* pcRevID = NULL;
			char* initior = NULL;
			char* changeReason = NULL;
			char* ecnClassification = NULL;
			char* ecnPriority = NULL;
			char* model = NULL;
			//date_t	applyTimmingProposal;
			char* exchangeabilityNewPart = NULL;
			char* exchangeabilityOldPart = NULL;
			tag_t tChangeItem = NULLTAG;
			tag_t tSolRelation = NULLTAG;
			tag_t tSolItemRelation = NULLTAG;
			tag_t tProblemRel = NULLTAG;
			tag_t tClass = NULLTAG;
			tag_t tRevClass = NULLTAG;
			logical lItemRevision = false;

			CHECK_ITK(iRetCode, POM_class_of_instance(ptTgt[iInd], &tClass));
			CHECK_ITK(iRetCode, POM_class_id_of_class("ItemRevision", &tRevClass));
			CHECK_ITK(iRetCode, POM_is_descendant(tRevClass, tClass, &lItemRevision));

			if (lItemRevision == true)
			{
				/*get description, synopsis, module group*/
				CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iInd], &tChangeItem));
				CHECK_ITK(iRetCode, ITEM_ask_id2(tChangeItem, &pcItemID));
				CHECK_ITK(iRetCode, ITEM_ask_rev_id2(ptTgt[iInd], &pcRevID));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_name", &pcSynopsis));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_desc", &pcDesc));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_string", &pcObjString));
				CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcObjType));

				/*thanhpn-13Jan2020-get initior, change reason, ECN classification, ECN priority, apply timing proposal, exchangeability of new part*/
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_initiator", &initior));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_change_reason", &changeReason));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_es_classification", &ecnClassification));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_es_priority", &ecnPriority));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_model_es", &model));

				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_es_exchange_newpart", &exchangeabilityNewPart));
				std::string exchangeabilityNewPartStr = exchangeabilityNewPart;

				if (strcmp(exchangeabilityNewPart, "X") == 0)
				{
					exchangeabilityNewPartStr.append(" - New parts may be used on old models as they are");
				}
				if (strcmp(exchangeabilityNewPart, "Y") == 0)
				{
					exchangeabilityNewPartStr.append(" - New parts may be used on old models in combination with other parts");
				}
				if (strcmp(exchangeabilityNewPart, "Z") == 0)
				{
					exchangeabilityNewPartStr.append(" - New parts must not be used on old models");
				}

				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_es_exchange_oldpart", &exchangeabilityOldPart));
				std::string exchangeabilityOldPartStr = exchangeabilityOldPart;

				if (strcmp(exchangeabilityOldPart, "1") == 0)
				{
					exchangeabilityOldPartStr.append(" - Old parts may be used on new models as they are");
				}
				if (strcmp(exchangeabilityOldPart, "2") == 0)
				{
					exchangeabilityOldPartStr.append(" - Old parts may be used on new models in combination with other parts");
				}
				if (strcmp(exchangeabilityOldPart, "3") == 0)
				{
					exchangeabilityOldPartStr.append(" - Old parts must not be used on new models");
				}
				if (strcmp(exchangeabilityOldPart, "4") == 0)
				{
					exchangeabilityOldPartStr.append(" - Old parts must not be used either on old models or new models");
				}

				CHECK_ITK(iRetCode, GRM_find_relation_type("CMHasSolutionItem", &tSolRelation));
				CHECK_ITK(iRetCode, GRM_find_relation_type("EC_solution_item_rel", &tSolItemRelation));
				CHECK_ITK(iRetCode, GRM_find_relation_type("CMHasProblemItem", &tProblemRel));

				/*get solution items*/
				if (tc_strcmp(pcObjType, "Vf6_ECNRevision") == 0 || tc_strcmp(pcObjType, "Vf6_MCNRevision") == 0
					|| tc_strcmp(pcObjType, "Vf6_ECRRevision") == 0 || tc_strcmp(pcObjType, "VF4_ERNRevision") == 0)
				{
					int			iSol = 0;
					int			iSolItems = 0;
					int			iProbItems = 0;
					tag_t* ptSol = NULL;
					tag_t* ptProblem = NULL;
					tag_t* ptSolItems = NULL;

					CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[iInd], tSolRelation, &iSol, &ptSol));
					CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[iInd], tSolItemRelation, &iSolItems, &ptSolItems));
					CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[iInd], tProblemRel, &iProbItems, &ptProblem));

					/*getting problem items*/
					for (int iProbCnt = 0; iProbCnt < iProbItems; iProbCnt++)
					{
						char* pcProbItemRev = NULL;

						CHECK_ITK(iRetCode, WSOM_ask_id_string(ptProblem[iProbCnt], &pcProbItemRev));

						szProbType.push_back(pcProbItemRev);
					}

					/*getting items from solutions folder*/
					for (int iCnt = 0; iCnt < iSol; iCnt++)
					{
						char* pcSolutionRev = NULL;

						CHECK_ITK(iRetCode, WSOM_ask_id_string(ptSol[iCnt], &pcSolutionRev));

						szSolType.push_back(pcSolutionRev);
					}

					/*getting items from solution item folder*/
					for (int iMECnt = 0; iMECnt < iSolItems; iMECnt++)
					{
						char* pcSolItemRev = NULL;

						CHECK_ITK(iRetCode, WSOM_ask_id_string(ptSolItems[iMECnt], &pcSolItemRev));

						szMEType.push_back(pcSolItemRev);
					}
				}

				if (tc_strstr(pcObjType, "Revision") != NULL)
				{
					mailBody << "<p><strong>FOLLOWING ARE THE DETAILS OF OBJECT RELEASED:</strong></p>";
					mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>OBJECT ID</strong></p></td><td width='283'><p>" << pcItemID << "/" << pcRevID << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>OBJECT NAME</strong></p></td><td width='283'><p>" << pcSynopsis << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>INITIOR</strong></p></td><td width='283'><p>" << initior << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>CHANGE REASON</strong></p></td><td width='283'><p>" << changeReason << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>MODEL</strong></p></td><td width='283'><p>" << model << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>ECN CLASSIFICATION</strong></p></td><td width='283'><p>" << ecnClassification << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>ECN PRIORITY</strong></p></td><td width='283'><p>" << ecnPriority << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>EXCHANGEABILITY OF NEW PART</strong></p></td><td width='283'><p>" << exchangeabilityNewPartStr.c_str() << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>EXCHANGEABILITY OF OLD PART</strong></p></td><td width='283'><p>" << exchangeabilityOldPartStr.c_str() << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>OBJECT DESCRIPTION</strong></p></td><td width='283'><p>" << pcDesc << "</p></td></tr></tbody></table>";

					if (tc_strcmp(pcObjType, "Vf6_ECNRevision") == 0)
					{
						string		szSolParts = "";
						for (int iSol = 0; iSol < szSolType.size(); iSol++)
						{
							szSolParts.append(szSolType[iSol]);
							if (szSolType.size() > iSol + 1)
							{
								szSolParts.append(", ");
							}
							else
							{
								mailBody << "<p><strong><u>FOLLOWING EBOM PARTS ARE RELEASED:</u></strong></p>";
								mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>EBOM Parts</strong></p></td><td width='283'><p>" << szSolParts.c_str() << "</p></td></tr></tbody></table>";
							}
						}
					}
					else if (tc_strcmp(pcObjType, "Vf6_ECRRevision") == 0)
					{
						string		szProbParts = "";
						for (int iProb = 0; iProb < szProbType.size(); iProb++)
						{
							szProbParts.append(szProbType[iProb]);
							if (szProbType.size() > iProb + 1)
							{
								szProbParts.append(", ");
							}
							else
							{
								mailBody << "<p><strong><u>FOLLOWING ARE PROBLEM ITEMS FOR ECR:</u></strong></p>";
								mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>EBOM Parts</strong></p></td><td width='283'><p>" << szProbParts.c_str() << "</p></td></tr></tbody></table>";
							}
						}
					}
					else if (tc_strcmp(pcObjType, "Vf6_MCNRevision") == 0)
					{
						string		szMEParts = "";
						for (int iMESol = 0; iMESol < szMEType.size(); iMESol++)
						{
							szMEParts.append(szMEType[iMESol]);
							if (szMEType.size() > iMESol + 1)
							{
								szMEParts.append(", ");
							}
							else
							{
								mailBody << "<p><strong><u>FOLLOWING MBOM PARTS ARE RELEASED:</u></strong></p>";
								mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>MBOM Parts</strong></p></td><td width='283'><p>" << szMEParts.c_str() << "</p></td></tr></tbody></table>";
							}
						}
					}
					else if (tc_strcmp(pcObjType, "VF4_ERNRevision") == 0)
					{
						string		szSolItemParts = "";
						for (int iSolItem = 0; iSolItem < szMEType.size(); iSolItem++)
						{
							szSolItemParts.append(szMEType[iSolItem]);
							if (szMEType.size() > iSolItem + 1)
							{
								szSolItemParts.append(", ");
							}
							else
							{
								mailBody << "<p><strong><u>FOLLOWING SCP PARTS ARE RELEASED:</u></strong></p>";
								mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>MBOM Parts</strong></p></td><td width='283'><p>" << szSolItemParts.c_str() << "</p></td></tr></tbody></table>";
							}
						}
					}
					else
					{
						if (tc_strstr(pcObjType, "Revision") != NULL)
						{
							mailBody << "<p><strong><u>FOLLOWING OBJECTS ARE RELEASED:</u></strong></p>";
							mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>Name</strong></p></td><td width='283'><p><strong>Type</strong></p></td></tr></tbody></table>";
							for (int iCntTgt = 0; iCntTgt < iTgt; iCntTgt++)
							{
								char* pcName = NULL;
								char* pcType = NULL;

								CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iCntTgt], &pcType));
								CHECK_ITK(iRetCode, WSOM_ask_id_string(ptTgt[iCntTgt], &pcName));

								mailBody << "<table border='1'><tbody><tr><td width='283'><p>%s</p></td><td width='283'><p>%s</p></td></tr></tbody></table>";
							}
						}
					}
					mailBody << "<p>Note: This is Teamcenter generated mail. Please do not reply.</p>";
				}
			}
			SAFE_MEM_free(pcDesc);
			SAFE_MEM_free(pcSynopsis);
			SAFE_MEM_free(pcItemID);
			SAFE_MEM_free(pcObjType);
			SAFE_MEM_free(initior);
			SAFE_MEM_free(changeReason);
			SAFE_MEM_free(ecnClassification);
			SAFE_MEM_free(ecnPriority);
			SAFE_MEM_free(exchangeabilityNewPart);
			SAFE_MEM_free(exchangeabilityOldPart);
			SAFE_MEM_free(model);
			SAFE_MEM_free(pcObjString);
			SAFE_MEM_free(pcRevID);
		}
	}
	sendEmail(pcMailSubject, mailBody.str(), szMailList, false);
	return iRetCode;
}
/*split string by sign ';'*/
string split(string str, string delimiter) {
	size_t pos = 0;
	string token;
	while ((pos = str.find(delimiter)) != string::npos) {
		token = str.substr(0, pos);
		cout << token << endl;
		str.erase(0, pos + delimiter.length());
	}
	return str;
}