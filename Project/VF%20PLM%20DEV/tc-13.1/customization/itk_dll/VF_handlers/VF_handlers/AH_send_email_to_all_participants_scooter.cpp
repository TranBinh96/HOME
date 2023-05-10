#include "Vinfast_Custom.h"

extern int AH_send_email_to_all_participants_scooter(EPM_action_message_t msg)
{
	int iRetCode = ITK_ok;
	boolean isDebug = false;

	char* pcMailSubject = NULL;
	char* pcFile = "";
	char* MailFile = "";
	std::stringstream mailBody;
	std::vector<string> szMailList;

	int	iNumArgs = 0;
	iNumArgs = TC_number_of_arguments(msg.arguments);
	if (iNumArgs > 0) {
		char* pcFlag = NULL;
		char* pcValue = NULL;
		for (int i = 0; i < iNumArgs; i++) {
			/*getting arguments*/
			CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue));
			if (iRetCode == ITK_ok) {
				if (tc_strcasecmp(pcFlag, "subject") == 0 && pcValue != NULL) {
					pcMailSubject = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcMailSubject, pcValue);
				}
				else if (tc_strcmp(pcFlag, "file") == 0) {
					pcFile = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcFile, pcValue);
				}
				else if (tc_strcasecmp(pcFlag, "debug") == 0 && pcValue != NULL) {
					isDebug = true;
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
	else {
		iRetCode = EPM_invalid_argument;
	}

	/*getting root task*/
	tag_t tRootTask = NULLTAG;
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0) {
		int	iAllTasks = 0;
		int	iTgt = 0;
		tag_t* ptTgt = NULL;
		tag_t* ptAllTaskTags = NULL;
		tag_t tInitiator = NULL;

		CHECK_ITK(iRetCode, EPM_ask_sub_tasks(tRootTask, &iAllTasks, &ptAllTaskTags));
		CHECK_ITK(iRetCode, AOM_ask_value_tag(msg.task, "owning_user", &tInitiator));
		/*getting mail id of all participants from review task*/
		for (int iLoopAllTasks = 0; iLoopAllTasks < iAllTasks; iLoopAllTasks++) {
			char* pcTaskTypeName = NULL;
			char* pcTaskName = NULL;
			char* pcTaskState = NULL;

			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_state", &pcTaskState));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_type", &pcTaskTypeName));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "current_name", &pcTaskName));

			if (tc_strcmp(pcTaskState, "Completed") == 0) {
				if (tc_strcmp(pcTaskTypeName, "EPMReviewTask") == 0) {
					int	iAllSubTasks = 0;
					tag_t* ptAllSubTaskTags = NULL;

					CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks], &iAllSubTasks, &ptAllSubTaskTags));
					for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++) {
						char* pcSubTaskTypeName = NULL;

						CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
						if (tc_strcmp(pcSubTaskTypeName, "EPMSelectSignoffTask") == 0) {
							int		iNumAttachs = 0;
							tag_t* ptAttachsTagList = NULL;

							//get signoff attachments
							CHECK_ITK(iRetCode, EPM_ask_attachments(ptAllSubTaskTags[iLoopAllSubTasks], EPM_signoff_attachment, &iNumAttachs, &ptAttachsTagList));
							for (int iUsers = 0; iUsers < iNumAttachs; iUsers++) {
								char* email = getUserMailOfTask(ptAttachsTagList[iUsers]);
								if (tc_strcmp(email, "") != NULL) {
									if (std::find(szMailList.begin(), szMailList.end(), email) == szMailList.end()) {
										szMailList.push_back(email);
									}
								}
							}
						}
					}
				}
				else if (tc_strcmp(pcTaskTypeName, "EPMDoTask") == 0) {
					char* email = getUserMailOfTask(ptAllTaskTags[iLoopAllTasks]);
					if (tc_strcmp(email, "") != NULL) {
						if (std::find(szMailList.begin(), szMailList.end(), email) == szMailList.end()) {
							szMailList.push_back(email);
						}
					}
				}
				else if (tc_strcmp(pcTaskTypeName, "EPMConditionTask")) {
					char* email = getUserMailOfTask(ptAllTaskTags[iLoopAllTasks]);
					if (tc_strcmp(email, "") != NULL) {
						if (std::find(szMailList.begin(), szMailList.end(), email) == szMailList.end()) {
							szMailList.push_back(email);
						}
					}
				}
				else if (tc_strcmp(pcTaskTypeName, "EPMAcknowledgeTask")) {
					int	iAllSubTasks = 0;
					tag_t* ptAllSubTaskTags = NULL;

					CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks], &iAllSubTasks, &ptAllSubTaskTags));
					for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++) {
						char* pcSubTaskTypeName = NULL;

						CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
						if (tc_strcmp(pcSubTaskTypeName, "EPMSelectSignoffTask") == 0) {
							int		iNumAttachs = 0;
							tag_t* ptAttachsTagList = NULL;

							//get signoff attachments
							CHECK_ITK(iRetCode, EPM_ask_attachments(ptAllSubTaskTags[iLoopAllSubTasks], EPM_signoff_attachment, &iNumAttachs, &ptAttachsTagList));
							for (int iUsers = 0; iUsers < iNumAttachs; iUsers++) {
								char* email = getUserMailOfTask(ptAllTaskTags[iLoopAllTasks]);
								if (tc_strcmp(email, "") != NULL) {
									if (std::find(szMailList.begin(), szMailList.end(), email) == szMailList.end()) {
										szMailList.push_back(email);
									}
								}
							}
						}
					}
				}
			}
		}

		/*getting mail id of initiator of workflow*/
		if (tInitiator != NULLTAG) {
			char* pcUserMail = NULL;

			CHECK_ITK(iRetCode, EPM_get_user_email_addr(tInitiator, &pcUserMail));
			if (tc_strcmp(pcUserMail, "") != NULL) {
				if (std::find(szMailList.begin(), szMailList.end(), pcUserMail) == szMailList.end()) {
					szMailList.push_back(pcUserMail);
				}
			}
		}

		/*reading mail ids from file*/
		if (tc_strcmp(pcFile, "") != 0) {
			string		szLine;
			ifstream	infile;
			infile.open(pcFile, ios::in);
			if (!infile)
				return -1;
			else {
				/*reading input file line by line*/
				while (getline(infile, szLine)) {
					char* pcLine = NULL;
					pcLine = (char*)MEM_alloc((int)(szLine.length() + 1) * sizeof(char));
					tc_strcpy(pcLine, szLine.c_str());
					szMailList.push_back(pcLine);
				}
			}
		}

		/*binhtt28 reading mail from */
		if (tc_strcmp(MailFile, "") != 0)
		{
			tag_t userTag = NULLTAG;
			char* pcUserMail = NULL;
			int count = 0;
			char* next_p;
			char** member = NULL;
			tag_t a_list = NULLTAG;
			if (string(MailFile).find("@vin", 0) == -1) {
				CHECK_ITK(iRetCode, MAIL_find_alias_list2(MailFile, &a_list));
				CHECK_ITK(iRetCode, MAIL_ask_alias_list_members(a_list, &count, &member));
				for (int iCnt = 0; iCnt < count; iCnt++) {
					if (std::find(szMailList.begin(), szMailList.end(), member[iCnt]) == szMailList.end())
					{
						string checkmail(member[iCnt]);
						string::size_type loc = checkmail.find("@", 0);
						CHECK_ITK(iRetCode, SA_find_user2(member[iCnt], &userTag));
						if (userTag != 0)
							szMailList.push_back(member[iCnt]);
						if (loc != string::npos) {
							szMailList.push_back(member[iCnt]);
						}
					}
				}
			}
			else {
				char* next_p;
				tag_t* group = NULLTAG;
				int		iSolItems = 0;
				tag_t** member = NULLTAG;
				char seps[] = "; ,\t\n";
				char* chars_array = strtok_s(MailFile, seps, &next_p);
				while (chars_array)
				{
					if (std::find(szMailList.begin(), szMailList.end(), chars_array) == szMailList.end())
					{
						szMailList.push_back(chars_array);
					}
					chars_array = strtok_s(NULL, seps, &next_p);
				}
			}
		}
		vector<string> szSolType;
		vector<string> classifiInfo;
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		for (int iInd = 0; iInd < iTgt; iInd++) {
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
			string model = "";
			//date_t	applyTimmingProposal;
			char* exchangeabilityNewPart = NULL;
			char* exchangeabilityOldPart = NULL;
			char* combine = NULL;
			tag_t	tChangeItem = NULLTAG;
			tag_t	tSolRelation = NULLTAG;
			tag_t	tClass = NULLTAG;
			tag_t	tRevClass = NULLTAG;
			logical lItemRevision = false;

			CHECK_ITK(iRetCode, POM_class_of_instance(ptTgt[iInd], &tClass));
			CHECK_ITK(iRetCode, POM_class_id_of_class("ItemRevision", &tRevClass));
			CHECK_ITK(iRetCode, POM_is_descendant(tRevClass, tClass, &lItemRevision));

			if (lItemRevision == true) {
				CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcObjType));
				if (tc_strcmp(pcObjType, "VF4_ChangeReq_ESRevision") == 0 || tc_strcmp(pcObjType, "VF4_ChangeReq_EERevision") == 0) {
					char* change_reason_attr = NULL;
					char* classification_attr = NULL;
					char* priority_attr = NULL;
					char* combine_release_status_attr = NULL;
					char* model_attr = NULL;
					char* exchangeability_newpart_attr = NULL;
					char* exchangeability_oldpart_attr = NULL;

					if (tc_strcmp(pcObjType, "VF4_ChangeReq_ESRevision") == 0) {
						change_reason_attr = "vf4_change_reason";
						classification_attr = "vf4_classification";
						priority_attr = "vf4_priority";
						combine_release_status_attr = "vf4_combine_release_status";
						model_attr = "vf4_model";
						exchangeability_newpart_attr = "vf4_exchangeability_newpart";
						exchangeability_oldpart_attr = "vf4_exchangeability_oldpart";
					}
					else if (tc_strcmp(pcObjType, "VF4_ChangeReq_EERevision") == 0) {
						change_reason_attr = "vf4_change_reason_pr";
						classification_attr = "vf4_classification_pr";
						priority_attr = "vf4_priority_pr";
						combine_release_status_attr = "vf4_combine_release_status";
						model_attr = "vf4_model_pr";
						exchangeability_newpart_attr = "vf4_exchangeability_new_pr";
						exchangeability_oldpart_attr = "vf4_exchangeability_old_pr";
					}
					/*get description, synopsis, module group*/
					CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iInd], &tChangeItem));
					CHECK_ITK(iRetCode, ITEM_ask_id2(tChangeItem, &pcItemID));
					CHECK_ITK(iRetCode, ITEM_ask_rev_id2(ptTgt[iInd], &pcRevID));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_name", &pcSynopsis));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_desc", &pcDesc));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_string", &pcObjString));

					tag_t owner = NULLTAG;
					CHECK_ITK(iRetCode, AOM_ask_value_tag(ptTgt[iInd], "owning_user", &owner));
					CHECK_ITK(iRetCode, AOM_ask_value_string(owner, "object_string", &initior));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], change_reason_attr, &changeReason));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], classification_attr, &ecnClassification));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], priority_attr, &ecnPriority));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], combine_release_status_attr, &combine));

					char** models = NULL;
					int		iSolItems = 0;
					CHECK_ITK(iRetCode, AOM_ask_value_strings(ptTgt[iInd], model_attr, &iSolItems, &models));
					for (int iCnt = 0; iCnt < iSolItems; iCnt++) {
						model.append(models[iCnt]);
						model.append(", ");
					}

					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], exchangeability_newpart_attr, &exchangeabilityNewPart));
					std::string exchangeabilityNewPartStr = exchangeabilityNewPart;

					if (strcmp(exchangeabilityNewPart, "X") == 0)
						exchangeabilityNewPartStr.append(" - New parts may be used on old models as they are");
					else if (strcmp(exchangeabilityNewPart, "Y") == 0)
						exchangeabilityNewPartStr.append(" - New parts may be used on old models in combination with other parts");
					else if (strcmp(exchangeabilityNewPart, "Z") == 0)
						exchangeabilityNewPartStr.append(" - New parts must not be used on old models");

					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], exchangeability_oldpart_attr, &exchangeabilityOldPart));
					std::string exchangeabilityOldPartStr = exchangeabilityOldPart;

					if (strcmp(exchangeabilityOldPart, "1") == 0)
						exchangeabilityOldPartStr.append(" - Old parts may be used on new models as they are");
					else if (strcmp(exchangeabilityOldPart, "2") == 0)
						exchangeabilityOldPartStr.append(" - Old parts may be used on new models in combination with other parts");
					else if (strcmp(exchangeabilityOldPart, "3") == 0)
						exchangeabilityOldPartStr.append(" - Old parts must not be used on new models");
					else if (strcmp(exchangeabilityOldPart, "4") == 0)
						exchangeabilityOldPartStr.append(" - Old parts must not be used either on old models or new models");

					CHECK_ITK(iRetCode, GRM_find_relation_type("CMHasSolutionItem", &tSolRelation));

					mailBody << "<html><meta charset='UTF-8'>";
					mailBody << "<p><strong>FOLLOWING ARE THE DETAILS OF OBJECT RELEASED:</strong></p>";
					mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>OBJECT ID</strong></p></td><td width='283'><p>" << pcItemID << "/" << pcRevID << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>OBJECT NAME</strong></p></td><td width='283'><p>" << pcSynopsis << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>INITIOR</strong></p></td><td width='283'><p>" << initior << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>CHANGE REASON</strong></p></td><td width='283'><p>" << changeReason << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>MODEL</strong></p></td><td width='283'><p>" << model.c_str() << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>ECN CLASSIFICATION</strong></p></td><td width='283'><p>" << ecnClassification << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>ECN PRIORITY</strong></p></td><td width='283'><p>" << ecnPriority << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>EXCHANGEABILITY OF NEW PART</strong></p></td><td width='283'><p>" << exchangeabilityNewPartStr.c_str() << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>EXCHANGEABILITY OF OLD PART</strong></p></td><td width='283'><p>" << exchangeabilityOldPartStr.c_str() << "</p></td></tr>";
					mailBody << "<tr><td width='283'><p><strong>OBJECT DESCRIPTION</strong></p></td><td width='283'><p>" << pcDesc << "</p></td></tr>";
					if ((combine != NULL) && (combine[0] == '\0'))
						mailBody << "<tr><td width='283'><p><strong>PROCESS DESCRIPTION</strong></p></td><td width='283'><p></p></td></tr></tbody></table>";
					else
						mailBody << "<tr><td width='283'><p><strong>PROCESS DESCRIPTION</strong></p></td><td width='283'><p>Combined with " << combine << "</p></td></tr></tbody></table>";

					/*get solution items*/
					int	iSol = 0;
					tag_t* ptSol = NULL;

					CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[iInd], tSolRelation, &iSol, &ptSol));
					for (int iCnt = 0; iCnt < iSol; iCnt++) {/*getting items from solutions folder*/
						char* pcSolutionRev = NULL;
						CHECK_ITK(iRetCode, WSOM_ask_id_string(ptSol[iCnt], &pcSolutionRev));
						szSolType.push_back(pcSolutionRev);

						//thanh 31-May add classification info
						char* isClassified = NULL;
						CHECK_ITK(iRetCode, AOM_ask_value_string(ptSol[iCnt], "ics_classified", &isClassified));
						if (iRetCode == ITK_ok && tc_strcasecmp(isClassified, "YES") == 0)
						{
							tag_t classRelation = NULL_TAG;
							int count = 0;
							tag_t* classiObj = NULL;
							CHECK_ITK(iRetCode, GRM_find_relation_type("IMAN_classification", &classRelation));
							CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptSol[iCnt], classRelation, &count, &classiObj));
							if (count >= 1)
							{
								std::string allClassification = "";
								for (int classCount = 0; classCount < count; classCount++)
								{
									/*int nAttr;
									char** attrName;
									char** attrValue;
									CHECK_ITK(iRetCode, ICS_ask_attributes_of_classification_obj(classiObj[classCount], &nAttr, &attrName, &attrValue));
									for (int ii = 0; ii < nAttr; ii++)
									{
										TC_write_syslog("[VF_handlers]\t%s - %s\n", attrName[ii], attrValue[ii]);
										SAFE_MEM_free(attrName[ii]);
										SAFE_MEM_free(attrValue[ii]);
									}
									SAFE_MEM_free(attrName);
									SAFE_MEM_free(attrValue);*/

									char* cid = NULL;
									CHECK_ITK(iRetCode, AOM_ask_value_string(classiObj[classCount], "cid", &cid));
									if (!allClassification.empty())
									{
										allClassification.append("\n");
										allClassification.append(cid);
									}
									allClassification.append(cid);
									SAFE_MEM_free(cid);
								}
								classifiInfo.push_back(allClassification);
							}
							SAFE_MEM_free(classiObj);
						}
						else
						{
							classifiInfo.push_back("");
						}
						SAFE_MEM_free(isClassified);
					}

					string szSolParts = "";
					/*for (int iSol = 0; iSol < szSolType.size(); iSol++) {
						szSolParts.append(szSolType[iSol]);
						if (szSolType.size() > iSol + 1) {
							szSolParts.append(", ");
						}
						else {
							mailBody << "<p><strong><u>FOLLOWING ARE SOLUTION ITEMS FOR ECN RELEASED:</u></strong></p>";
							mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>EBOM Parts</strong></p></td><td width='283'><p>" << szSolParts.c_str() << "</p></td></tr></tbody></table>";
						}
					}*/
					mailBody << "<p><strong><u>FOLLOWING ARE SOLUTION ITEMS FOR ECN RELEASED:</u></strong></p>";
					mailBody << "<table border='1'><tbody><tr><td width='283'><p><strong>Part Number</strong></p></td><td width='283'><p><strong>Classification</strong></p></td></tr>";
					for (int iSol = 0; iSol < szSolType.size(); iSol++)
					{
						mailBody << "<tr><td width='283'><p>" << szSolType[iSol] << "</p></td><td width='283'><p>" << classifiInfo[iSol] << "</p></td></tr>";
					}
					mailBody << "</tbody></table>";

					mailBody << "<p>Note: This is Teamcenter generated mail. Please do not reply.</p>";
					mailBody << "</html>";
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
			SAFE_MEM_free(pcObjString);
			SAFE_MEM_free(pcRevID);
		}
	}
	sendEmail(pcMailSubject, mailBody.str(), szMailList, isDebug);
	return iRetCode;
}