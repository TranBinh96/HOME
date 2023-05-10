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


extern int AH_send_email_to_all(EPM_action_message_t msg)
{
	int			iRetCode = ITK_ok;
	int			iNumArgs = 0;
	int			iPrefCount = 0;
	int			portPref_count = 0;
	tag_t		tRootTask = NULLTAG;
	char* pcMailSubject = NULL;
	char* pcRecipients = NULL;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* portNo = NULL;
	char* pcServerName = NULL;
	char* pcMailBodyFileName = NULL;
	char* pcRecipient = NULL;
	char* current_task_name = NULL;
	char* object_type = NULL;
	char* sub_object_type = NULL;
	char* process_desc = NULL;
	char* process_job_name = NULL;
	char* date_string = NULL;
	int			count_task = NULL;
	int			signoff_count = NULL;
	tag_t		current_job = NULLTAG;
	tag_t* subtasks = NULLTAG;
	tag_t* attachment = NULLTAG;
	date_t		task_due_date;
	tag_t			_performer = NULLTAG;
	char		caMailDetails[BUFSIZ + 1];
	char		pcRecipientsList[BUFSIZ + 1];

	iNumArgs = TC_number_of_arguments(msg.arguments);

	std::stringstream mailBody;
	std::vector<string> szMailList;

	if (iNumArgs >= 2)
	{
		for (int i = 0; i < iNumArgs; i++)
		{
			/*getting arguments*/
			//TC_write_syslog("shit1\n");
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue);
			if (iRetCode == ITK_ok)
			{
				//TC_write_syslog("shit2\n");
				if (tc_strcasecmp(pcFlag, "subject") == 0 && pcValue != NULL)
				{
					//	TC_write_syslog("shit3\n");
					pcMailSubject = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcMailSubject, pcValue);
				}

				else if (tc_strcasecmp(pcFlag, "to") == 0 && pcValue != NULL)
				{
					//	TC_write_syslog("shit5\n");
					pcRecipients = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcRecipients, pcValue);
				}

				else if (tc_strcasecmp(pcFlag, "recipient") == 0 && pcValue != NULL)
				{
					//	TC_write_syslog("shit7\n");
					pcRecipient = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
					tc_strcpy(pcRecipient, pcValue);
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

	/*set tc_mail_smtp path*/


	/*getting root task*/
	if (pcRecipients != NULL)
	{
		caMailDetails[0] = '\0';
#if defined(WNT)
		tc_strcpy(caMailDetails, "%TC_BIN%\\tc_mail_smtp ");
#else
		tc_strcpy(caMailDetails, "$TC_BIN/tc_mail_smtp ");
#endif

		if (tc_strcmp(pcRecipients, "ECNList") == 0)
		{
			/*set recipients list path path*/
			pcRecipientsList[0] = '\0';
#if defined(WNT)
			tc_strcpy(pcRecipientsList, "%TC_BIN%\\car_ecn_list.txt");
#else
			tc_strcpy(pcRecipientsList, "$TC_BIN/car_ecn_list.txt");
#endif
		}

		if (tc_strcmp(pcRecipients, "MCNList") == 0)
		{
			/*set recipients list path path*/
			pcRecipientsList[0] = '\0';
#if defined(WNT)
			tc_strcpy(pcRecipientsList, "%TC_BIN%\\car_mcn_list.txt");
#else
			tc_strcpy(pcRecipientsList, "$TC_BIN/car_mcn_list.txt");
#endif
		}

		// Acar MCN Recipients List
		if (tc_strcmp(pcRecipients, "AcarMCNList") == 0)
		{
			/*set recipients list path path*/
			pcRecipientsList[0] = '\0';
#if defined(WNT)
			tc_strcpy(pcRecipientsList, "%TC_BIN%\\AcarMCNList.txt");
#else
			tc_strcpy(pcRecipientsList, "$TC_BIN/AcarMCNList.txt");
#endif
		}

		if (tc_strcmp(pcRecipients, "ScooterECNList") == 0)
		{
			/*set recipients list path path*/
			pcRecipientsList[0] = '\0';
#if defined(WNT)
			tc_strcpy(pcRecipientsList, "%TC_BIN%\\scooter_ecn_list.txt");
#else
			tc_strcpy(pcRecipientsList, "$TC_BIN/scooter_ecn_list.txt");
#endif
		}

		if (tc_strcmp(pcRecipients, "ScooterMCNList") == 0)
		{
			/*set recipients list path path*/
			pcRecipientsList[0] = '\0';
#if defined(WNT)
			tc_strcpy(pcRecipientsList, "%TC_BIN%\\scooter_mcn_list.txt");
#else
			tc_strcpy(pcRecipientsList, "$TC_BIN/scooter_mcn_list.txt");
#endif
		}

		/*get to from argument*/
		/*tc_strcat( caMailDetails, "-to=\"" );
		tc_strcat( caMailDetails, pcRecipients );
		tc_strcat( caMailDetails, "\" " );*/

		tc_strcat(caMailDetails, "-to_list_file=\"");
		tc_strcat(caMailDetails, pcRecipientsList);
		tc_strcat(caMailDetails, "\" ");



		/*getting server name*/
		if (iRetCode == ITK_ok)
		{
			// CHECK_ITK(iRetCode,PREF_set_search_scope( TC_preference_site ));//@SKIP_DEPRECATED - WILL REMOVED
		}
		if (iRetCode == ITK_ok)
		{
			CHECK_ITK(iRetCode, PREF_ask_value_count("Mail_server_name", &iPrefCount));
		}

		if ((iRetCode == ITK_ok) && iPrefCount != 0)
		{
			CHECK_ITK(iRetCode, PREF_ask_char_value("Mail_server_name", 0, &pcServerName));
			if (iRetCode == ITK_ok)
			{
				tc_strcat(caMailDetails, "-server=\"");
				tc_strcat(caMailDetails, pcServerName);
				tc_strcat(caMailDetails, "\" ");
			}
			SAFE_MEM_free(pcServerName);
		}

		/*getting port number*/
		if (iRetCode == ITK_ok)
		{
			CHECK_ITK(iRetCode, PREF_ask_value_count("Mail_server_port", &portPref_count));
		}

		if ((iRetCode == ITK_ok) && portPref_count != 0)
		{
			CHECK_ITK(iRetCode, PREF_ask_char_value("Mail_server_port", 0, &portNo));
			if (iRetCode == ITK_ok)
			{
				tc_strcat(caMailDetails, "-port=\"");
				tc_strcat(caMailDetails, portNo);
				tc_strcat(caMailDetails, "\" ");
			}
			SAFE_MEM_free(portNo);
		}
		CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
		if (tRootTask != 0)
		{
			int			iTgt = 0;
			tag_t* ptTgt = NULL;

			vector<string> szCadType;
			vector<string> szCarType;
			vector<string> szMEType;

			/*getting target objects*/
			CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
			for (int iInd = 0; iInd < iTgt; iInd++)
			{
				char* pcObjType = NULL;

				CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptTgt[iInd], &pcObjType));
				/*for object of ECN/MCN Revision*/
				if (tc_strcmp(pcObjType, "Vf6_ECNRevision") == 0 || tc_strcmp(pcObjType, "Vf6_MCNRevision") == 0)
				{
					char* pcDesc = NULL;
					char* pcSynopsis = NULL;
					char* pcItemID = NULL;
					char* pcModGrp = NULL;
					tag_t	tChangeItem = NULLTAG;
					tag_t	tSolRelation = NULLTAG;
					tag_t	tMCNRelation = NULLTAG;
					FILE* fMailBodyFile = NULL;

					/*get description, synopsis, module group*/
					CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iInd], &tChangeItem));
					CHECK_ITK(iRetCode, ITEM_ask_id2(tChangeItem, &pcItemID));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_name", &pcSynopsis));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iInd], "object_desc", &pcDesc));
					CHECK_ITK(iRetCode, AOM_ask_value_string(tChangeItem, "vf6_module_group", &pcModGrp));

					CHECK_ITK(iRetCode, GRM_find_relation_type("CMHasSolutionItem", &tSolRelation));
					CHECK_ITK(iRetCode, GRM_find_relation_type("EC_solution_item_rel", &tMCNRelation));

					/*get slution items*/
					if (tc_strcmp(pcObjType, "Vf6_ECNRevision") == 0 || tc_strcmp(pcObjType, "Vf6_MCNRevision") == 0)
					{
						int			iSolItems = 0;
						int			iMCNItems = 0;
						tag_t* ptSol = NULL;
						tag_t* ptMCNSol = NULL;

						CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[iInd], tSolRelation, &iSolItems, &ptSol));
						CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[iInd], tMCNRelation, &iMCNItems, &ptMCNSol));
						for (int iCnt = 0; iCnt < iSolItems; iCnt++)
						{
							char* pcItemRev = NULL;
							char* pcSolType = NULL;

							CHECK_ITK(iRetCode, WSOM_ask_object_type2(ptSol[iCnt], &pcSolType));
							CHECK_ITK(iRetCode, WSOM_ask_id_string(ptSol[iCnt], &pcItemRev));
							if (tc_strcmp(pcSolType, "PR4D_MS_IT_VINFRevision") == 0)
							{
								szCadType.push_back(pcItemRev);
							}
							else
							{
								szCarType.push_back(pcItemRev);
							}

						}

						for (int iMECnt = 0; iMECnt < iMCNItems; iMECnt++)
						{
							char* pcMEItemRev = NULL;
							char* pcSolType = NULL;

							//CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptMCNSol[iMECnt],&pcSolType));
							CHECK_ITK(iRetCode, WSOM_ask_id_string(ptMCNSol[iMECnt], &pcMEItemRev));


							szMEType.push_back(pcMEItemRev);

						}
					}

					pcMailBodyFileName = USER_new_file_name("cr_notify_dset", "TEXT", "html", 1);
					fMailBodyFile = fopen(pcMailBodyFileName, "w");

					if (fMailBodyFile == NULL)
					{
						iRetCode = CR_cannot_open_file;
					}
					else
					{
						fprintf(fMailBodyFile, "<p><strong>FOLLOWING ARE THE DETAILS OF CHANGE RELEASED:</strong></p>\n\n");
						fprintf(fMailBodyFile, "<table border=\"1\"><tbody><tr><td width=\"283\"><p><strong>CHANGE NUMBER</strong></p></td><td width=\"283\"><p>%s</p></td></tr>\n", pcItemID);
						fprintf(fMailBodyFile, "<tr><td width=\"283\"><p><strong>SYNOPSIS</strong></p></td><td width=\"283\"><p>%s</p></td></tr>\n", pcSynopsis);
						fprintf(fMailBodyFile, "<tr><td width=\"283\"><p><strong>DESCRIPTION</strong></p></td><td width=\"283\"><p>%s</p></td></tr>\n", pcDesc);
						fprintf(fMailBodyFile, "<tr><td width=\"283\"><p><strong>MODULE GROUP</strong></p></td><td width=\"283\"><p>%s</p></td></tr></tbody></table>\n\n\n", pcModGrp);

						if (tc_strcmp(pcObjType, "Vf6_ECNRevision") == 0)
						{
							string		szCarParts = "";
							string		szCadParts = "";

							for (int iCadSol = 0; iCadSol < szCadType.size(); iCadSol++)
							{
								szCadParts.append(szCadType[iCadSol]);
								if (szCadType.size() > iCadSol + 1)
								{
									szCadParts.append(", ");
								}
								else
								{
									fprintf(fMailBodyFile, "<p><strong><u>FOLLOWING CAD PARTS ARE RELEASED:</u></strong></p>\n");
									fprintf(fMailBodyFile, "<table border=\"1\"><tbody><tr><td width=\"283\"><p><strong>CAD Parts</strong></p></td><td width=\"283\"><p>%s</p></td></tr></tbody></table>\n\n\n", szCadParts.c_str());
								}
							}

							for (int iCarSol = 0; iCarSol < szCarType.size(); iCarSol++)
							{
								szCarParts.append(szCarType[iCarSol]);
								if (szCarType.size() > iCarSol + 1)
								{
									szCarParts.append(", ");
								}
								else
								{
									fprintf(fMailBodyFile, "<p><strong><u>FOLLOWING EBOM PARTS ARE RELEASED:</u></strong></p>\n");
									fprintf(fMailBodyFile, "<table border=\"1\"><tbody><tr><td width=\"283\"><p><strong>EBOM Parts</strong></p></td><td width=\"283\"><p>%s</p></td></tr></tbody></table>\n\n\n", szCarParts.c_str());
								}
							}
						}

						if (tc_strcmp(pcObjType, "Vf6_MCNRevision") == 0)
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
									fprintf(fMailBodyFile, "<p><strong><u>FOLLOWING MBOM PARTS ARE RELEASED:</u></strong></p>\n");
									fprintf(fMailBodyFile, "<table border=\"1\"><tbody><tr><td width=\"283\"><p><strong>MBOM Parts</strong></p></td><td width=\"283\"><p>%s</p></td></tr></tbody></table>\n\n\n", szMEParts.c_str());
								}
							}
						}
						fprintf(fMailBodyFile, "<p>Note: This is Teamcenter generated mail. Please do not reply.</p>\n\n");

						/*get subject*/
						tc_strcat(caMailDetails, "-subject=\"");
						tc_strcat(caMailDetails, pcMailSubject);
						tc_strcat(caMailDetails, " - ");
						tc_strcat(caMailDetails, pcItemID);
						tc_strcat(caMailDetails, "\" ");

						tc_strcat(caMailDetails, "-body=\"");
						tc_strcat(caMailDetails, pcMailBodyFileName);
						tc_strcat(caMailDetails, "\" ");
					}

					if (fMailBodyFile != NULL)
					{
						fclose(fMailBodyFile);
					}
					SAFE_MEM_free(pcDesc);
					SAFE_MEM_free(pcSynopsis);
					SAFE_MEM_free(pcItemID);
					SAFE_MEM_free(pcModGrp);
				}
				SAFE_MEM_free(pcObjType);
			}
		}
		system(caMailDetails);

		remove(pcMailBodyFileName);
	}
	//Send notify review for review task, do task, condition task, acknowledge task only
	else if (pcRecipient != NULL)
	{
		FILE* fMailBodyFile = NULL;
		int obj_count = 0;
		tag_t* root_attachments = NULLTAG;
		char* obj_type1 = NULL;
		char* effective_proposal = NULL;
		char* instruction = NULL;
		char* uid = NULL;
		char* object_name = NULL;
		char* usr_email = NULL;
		vector<string> list_user(20);
		vector<pair<char*, char*>> target_object_list(200);

		CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));

		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &obj_count, &root_attachments));
		for (int k = 0;k < obj_count; k++)
		{

			CHECK_ITK(iRetCode, AOM_ask_value_string(root_attachments[k], "object_name", &object_name));

			CHECK_ITK(iRetCode, WSOM_ask_object_type2(root_attachments[k], &obj_type1));
			target_object_list[k].first = object_name;
			target_object_list[k].second = obj_type1;
			if (tc_strcmp(obj_type1, "Vf6_ECRRevision") == 0 || tc_strcmp(obj_type1, "Vf6_ECNRevision") == 0)
			{


				CHECK_ITK(iRetCode, AOM_ask_value_string(root_attachments[k], "vf6_effective_proposal", &effective_proposal));
			}
		}

		CHECK_ITK(iRetCode, WSOM_ask_object_type2(msg.task, &object_type));
		TC_write_syslog("\n Object_type : %s", object_type);
		CHECK_ITK(iRetCode, EPM_ask_job(msg.task, &current_job));
		CHECK_ITK(iRetCode, AOM_ask_value_string(msg.task, "current_desc", &instruction));
		CHECK_ITK(iRetCode, AOM_ask_value_string(current_job, "current_desc", &process_desc));
		CHECK_ITK(iRetCode, AOM_ask_value_string(current_job, "current_name", &process_job_name));
		if (tc_strcmp(object_type, "EPMDoTask") == 0 || tc_strcmp(object_type, "EPMConditionTask") == 0)
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
		else if (tc_strcmp(object_type, "EPMPerformSignoffTask") == 0)
		{
			tag_t parent_task = NULLTAG;
			CHECK_ITK(iRetCode, EPM_ask_parent_task(msg.task, &parent_task));
			CHECK_ITK(iRetCode, EPM_ask_sub_tasks(parent_task, &count_task, &subtasks));
			for (int i = 0; i < count_task; i++)
			{
				CHECK_ITK(iRetCode, WSOM_ask_object_type2(subtasks[i], &sub_object_type));
				TC_write_syslog("\n sub_object_type: %s", sub_object_type);
				if (tc_strcmp(sub_object_type, "EPMPerformSignoffTask") == 0)
				{
					CHECK_ITK(iRetCode, AOM_ask_value_string(subtasks[i], "object_name", &current_task_name));
					CHECK_ITK(iRetCode, AOM_ask_value_date(subtasks[i], "due_date", &task_due_date));
					CHECK_ITK(iRetCode, DATE_date_to_string(task_due_date, "%Y-%m-%d %H:%M:%S", &date_string));
					ITK__convert_tag_to_uid(subtasks[i], &uid);
				}
				if (tc_strcmp(sub_object_type, "EPMSelectSignoffTask") == 0)
				{
					CHECK_ITK(iRetCode, EPM_ask_attachments(subtasks[i], EPM_signoff_attachment, &signoff_count, &attachment));
					TC_write_syslog("\n signoff_count: %d", signoff_count);
					for (int j = 0;j < signoff_count; j++)
					{
						tag_t tUser = NULLTAG;
						char* user_email = NULL;
						CHECK_ITK(iRetCode, AOM_ask_value_tag(attachment[j], "fnd0Performer", &tUser));
						CHECK_ITK(iRetCode, EPM_get_user_email_addr(tUser, &user_email));
						TC_write_syslog("\n user_email: %s", user_email);
						list_user.push_back(user_email);



					}
				}
			}

		}
		SAFE_MEM_free(obj_type1);
		//prepare email to sending
		//pcMailBodyFileName = USER_new_file_name("cr_notify_dset","TEXT","html",1);
		//fMailBodyFile = fopen(pcMailBodyFileName, "w");
		char* fullpath = NULL;
		std::string awc_link = "https://tcweb.vinfast.vn/#/com.siemens.splm.clientfx.tcui.xrt.showObject?uid=";
		TC_write_syslog("before awc_link: %s", awc_link.c_str());
		std::string rich_client_link = "http://tcweb.vinfast.vn/tc/launchapp?-attach=true&-s=226TCSession&-o=";
		TC_write_syslog("before rich_client_link: %s", rich_client_link.c_str());
		awc_link.append(uid);
		rich_client_link.append(uid);
		TC_write_syslog("\ncurrent task name: %s", current_task_name);

		TC_write_syslog("\nprocess job name: %s", process_job_name);

		TC_write_syslog("\n date string: %s", date_string);

		TC_write_syslog("\n process desc: %s", process_desc);

		TC_write_syslog("\n instruction: %s", instruction);

		TC_write_syslog("\n effective proposal: %s", effective_proposal);

		TC_write_syslog("\n awc link: %s", awc_link.c_str());

		TC_write_syslog("\n rich client link: %s", rich_client_link.c_str());
		std::string fullpathfile = "";
		fullpathfile = "C:\\Temp\\" + getCurrentTimeStampStr() + "_" + std::to_string(rand()) + ".json";
		ofstream o(fullpathfile);
		o << "{to:\"";
		for (int k = 0; k < list_user.size();k++)
		{
			TC_write_syslog("\nuser mail: %d - %s", k, list_user[k].c_str());
		}
		TC_write_syslog("\nuser mail Do Task: %s", usr_email);

		for (int l = 0;l < list_user.size();l++)
		{
			if (list_user.at(l) != "")
			{

				o << list_user.at(l).c_str();
				o << ";";

			}
		}

		if (usr_email != NULL)
		{

			o << usr_email;

		}
		o << "\",";
		o << "subject:\"";
		o << pcMailSubject;
		o << "\",";
		o << "message:\"";
		o << "<!DOCTYPE html><html><title>Change Process Email</title><body>";
		o << "<p><strong>Overview:</strong></p>\n";
		o << "<table border='1'><tbody><tr><td width='283'><p><strong>Current Task</strong></p></td><td width='500'>";
		o << current_task_name;
		o << "</td></tr>";
		o << "<tr><td width='283'><p><strong>Process Name</strong></p></td><td width='500'>";
		o << process_job_name;
		o << "</td></tr>";
		o << "<tr><td width='283'><p><strong>Due Date</strong></p></td><td width='500'>";
		o << date_string;
		o << "</td></tr>";
		o << "<tr><td width='283'><p><strong>Process Description</strong></p></td><td width='500'>";
		o << process_desc;
		o << "</td></tr>";
		o << "<tr><td width='283'><p><strong>Instruction</strong></p></td><td width='500'>";
		o << instruction;
		o << "</td></tr>";
		o << "<tr><td width='283'><p><strong>Effective Proposal</strong></p></td><td width='500'>";
		o << effective_proposal;
		o << "</td></tr>";
		o << "</tbody></table>\n";
		o << "<p><strong>select the preffered client to view the task</strong></p>\n";
		o << "<a href='";
		o << awc_link.c_str();
		o << "'>Active Workspace</a><br>";
		o << "<a href='";
		o << rich_client_link.c_str();
		o << "'>Rich Client</a><br>";
		o << "<p><strong>Target</strong></p><br>";
		o << "<table border='1'><tr><td>Object Name</td></tr>";
		for (int m = 0;m < target_object_list.size();m++)
		{
			if (target_object_list[m].first != NULL)
			{
				TC_write_syslog("\nobject name : %s - object type: %s\n", target_object_list[m].first, target_object_list[m].second);
				o << "<tr>";
				o << "<td>";
				o << target_object_list[m].first;
				//tc_strcat(message, "</td><td>");
				//tc_strcat(message, target_object_list[m].second);
				o << "</td>";
				o << "</tr>";
			}

		}
		o << "</table><br>";
		o << "<p><strong>This is auto generated email. Please don't reply</strong></p>";
		o << "</body></html>";
		o << "\"}";
		o.flush();
		o.close();

		SAFE_MEM_free(current_task_name);
		SAFE_MEM_free(process_job_name);
		SAFE_MEM_free(date_string);
		SAFE_MEM_free(process_desc);
		SAFE_MEM_free(instruction);
		SAFE_MEM_free(effective_proposal);
		SAFE_MEM_free(object_name);
		SAFE_MEM_free(uid);




		CHECK_ITK(iRetCode, PREF_ask_char_value("VF_PLM_MAIL_LOCATION", 0, &fullpath));
		TC_write_syslog("fullpath: %s\n", fullpath);
		//exportJson(fullpath, to, subject, message);
		//open file

		//write file
		//move file
		std::string newpathfile = "";
		newpathfile.append(fullpath);
		newpathfile.append("\\");
		newpathfile.append(getCurrentTimeStampStr());
		newpathfile.append("_");
		newpathfile.append(std::to_string(rand()));
		newpathfile.append(".json");
		if (rename(fullpathfile.c_str(), newpathfile.c_str()) != 0)
		{
			perror("move file error");
		}
		else
		{
			TC_write_syslog("moved file");
		}
		SAFE_MEM_free(usr_email);
		SAFE_MEM_free(object_type);
		SAFE_MEM_free(sub_object_type);
		SAFE_MEM_free(fullpath);

	}
	return iRetCode;
}


