#include "Vinfast_Custom.h"
#include <sstream>

extern int AH_send_email_to_all_participants_car(EPM_action_message_t msg)
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
	char* pcProgram = NULL;
	char* pcValue = NULL;
	char* process_desc = NULL;
	tag_t current_job = NULLTAG;

	iNumArgs = TC_number_of_arguments(msg.arguments);

	std::stringstream mailBody;
	std::vector<string> szMailList;

	if (iNumArgs == 2)
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
			}
		}
		SAFE_MEM_free(pcFlag);
		SAFE_MEM_free(pcValue);
	}
	else
	{
		iRetCode = EPM_invalid_argument;
	}

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
		CHECK_ITK(iRetCode, EPM_ask_job(msg.task, &current_job));
		CHECK_ITK(iRetCode, AOM_ask_value_string(current_job, "current_desc", &process_desc));
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
				tag_t		tUser = NULLTAG;
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
				tag_t		tUser = NULLTAG;
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
		if (tc_strcmp(pcFile, "") != 0 && tc_strlen(pcFile) > 0)
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
			char* vehicle_group = NULL;
			char* module_group = NULL;
			char* effective_proposal = NULL;
			tag_t tChangeItem = NULLTAG;
			tag_t tSolRelation = NULLTAG;
			tag_t tSolItemRelation = NULLTAG;
			tag_t tProblemRel = NULLTAG;
			tag_t tClass = NULLTAG;
			tag_t tRevClass = NULLTAG;
			FILE* fMailBodyFile = NULL;
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
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_vehicle_group", &vehicle_group));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_module_group_comp", &module_group));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "vf6_effective_proposal", &effective_proposal));

				CHECK_ITK(iRetCode, GRM_find_relation_type("CMHasSolutionItem", &tSolRelation));
				CHECK_ITK(iRetCode, GRM_find_relation_type("EC_solution_item_rel", &tSolItemRelation));
				CHECK_ITK(iRetCode, GRM_find_relation_type("CMHasProblemItem", &tProblemRel));

				/*get solution items*/
				if (tc_strcmp(pcObjType, "Vf6_ECNRevision") == 0 || tc_strcmp(pcObjType, "Vf6_MCNRevision") == 0
					|| tc_strcmp(pcObjType, "Vf6_ECRRevision") == 0 || tc_strcmp(pcObjType, "VF4_ERNRevision") == 0)
				{
					int	iSol = 0;
					int	iSolItems = 0;
					int	iProbItems = 0;
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
					mailBody << "<p><strong>FOLLOWING ARE THE DETAILS OF OBJECT RELEASED:</strong></p><br><br>";
					mailBody << "<table border='1'><tbody>";
					mailBody << "<tr><td width='283'><p><strong>OBJECT ID</strong></p></td><td width='283'><p>" << pcItemID << "/" << pcRevID << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>OBJECT NAME</strong></p></td><td width='283'><p>" << pcSynopsis << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>INITIOR</strong></p></td><td width='283'><p>" << initior << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>VEHICLE GROUP</strong></p></td><td width='283'><p>" << vehicle_group << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>MODULE GROUP</strong></p></td><td width='283'><p>" << module_group << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>PROCESS DESCRIPTION</strong></p></td><td width='283'><p>" << process_desc << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>EFFECTIVE PROPOSAL</strong></p></td><td width='283'><p>" << effective_proposal << "</p></td></tr>";
					mailBody << "</tbody></table><br><br>";

					if (tc_strcmp(pcObjType, "Vf6_ECNRevision") == 0)
					{
						string szSolParts = "";
						for (int iSol = 0; iSol < szSolType.size(); iSol++)
						{
							szSolParts.append(szSolType[iSol]);
							if (szSolType.size() > iSol + 1)
							{
								szSolParts.append(", ");
							}
							else
							{
								mailBody << "<p><strong><u>FOLLOWING EBOM PARTS ARE RELEASED:</u></strong></p><br>";
								mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>Targets</strong></p></td><td width='283'><p>";
								mailBody << szSolParts.c_str();
								mailBody << "</p></td></tr></tbody></table><br>";
							}
						}
					}

					else if (tc_strcmp(pcObjType, "Vf6_ECRRevision") == 0)
					{
						string szProbParts = "";
						for (int iProb = 0; iProb < szProbType.size(); iProb++)
						{
							szProbParts.append(szProbType[iProb]);
							if (szProbType.size() > iProb + 1)
							{
								szProbParts.append(", ");
							}
							else
							{
								mailBody << "<p><strong><u>FOLLOWING ARE PROBLEM ITEMS FOR ECR:</u></strong></p><br>";
								mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>Targets</strong></p></td><td width='283'><p>";
								mailBody << szProbParts.c_str();
								mailBody << "</p></td></tr></tbody></table><br><br>";
							}
						}
					}

					else if (tc_strcmp(pcObjType, "Vf6_MCNRevision") == 0)
					{
						string szMEParts = "";
						for (int iMESol = 0; iMESol < szMEType.size(); iMESol++)
						{
							szMEParts.append(szMEType[iMESol]);
							if (szMEType.size() > iMESol + 1)
							{
								szMEParts.append(", ");
							}
							else
							{
								mailBody << "<p><strong><u>FOLLOWING MBOM PARTS ARE RELEASED:</u></strong></p><br>";
								mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>Targets</strong></p></td><td width='283'><br>";
								mailBody << szMEParts.c_str();
								mailBody << "</p></td></tr></tbody></table><br><br>";
							}
						}
					}

					else if (tc_strcmp(pcObjType, "VF4_ERNRevision") == 0)
					{
						string szSolItemParts = "";
						for (int iSolItem = 0; iSolItem < szMEType.size(); iSolItem++)
						{
							szSolItemParts.append(szMEType[iSolItem]);
							if (szMEType.size() > iSolItem + 1)
							{
								szSolItemParts.append(", ");
							}
							else
							{
								mailBody << "<p><strong><u>FOLLOWING SCP PARTS ARE RELEASED:</u></strong></p><br>";
								mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>Targets</strong></p></td><td width='283'><p>";
								mailBody << szSolItemParts.c_str();
								mailBody << "</p></td></tr></tbody></table><br><br>";
							}
						}
					}

					else
					{
						if (tc_strstr(pcObjType, "Revision") != NULL)
						{
							mailBody << "<p><strong><u>FOLLOWING OBJECTS ARE RELEASED:</u></strong></p><br>";
							mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>Name</strong></p></td><td width='283'><p><strong>Type</strong></p></td></tr></tbody></table>";
							for (int iCntTgt = 0; iCntTgt < iTgt; iCntTgt++)
							{
								char* pcName = NULL;
								char* pcType = NULL;

								CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iCntTgt], &pcType));
								CHECK_ITK(iRetCode, WSOM_ask_id_string(ptTgt[iCntTgt], &pcName));

								mailBody << "<table border='1'><tbody><tr><td width='283'><p>";
								mailBody << pcName;
								mailBody << "</p></td><td width='283'><p>";
								mailBody << pcType;
								mailBody << "</p></td></tr></tbody></table><br><br>";
							}
						}
					}
					mailBody << "<p>Note: This is Teamcenter generated mail. Please do not reply.</p><br>";
				}
			}
			SAFE_MEM_free(pcDesc);
			SAFE_MEM_free(pcSynopsis);
			SAFE_MEM_free(pcItemID);
			SAFE_MEM_free(pcObjType);
			SAFE_MEM_free(initior);
			SAFE_MEM_free(process_desc);
			SAFE_MEM_free(pcObjString);
			SAFE_MEM_free(pcRevID);
			SAFE_MEM_free(vehicle_group);
			SAFE_MEM_free(module_group);
			SAFE_MEM_free(effective_proposal);
		}
	}

	sendEmail(pcMailSubject, mailBody.str(), szMailList, false);
	return iRetCode;
}