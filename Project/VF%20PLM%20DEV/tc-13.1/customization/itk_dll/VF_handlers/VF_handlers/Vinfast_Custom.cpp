#include "Vinfast_Custom.h"

extern int VF_handlers_register_handlers(int* decision, va_list args)
{
	int iRetCode = ITK_ok;
	*decision = ALL_CUSTOMIZATIONS;

	/*Rule handlers*/
	iRetCode = EPM_register_rule_handler("RH-check-unreleased-ItemRevision", "This rule handler will check for unreleased IR", (EPM_rule_handler_t)RH_Check_unreleaseed_ItemRevision);

	iRetCode = EPM_register_rule_handler("RH-check-SAP-transfer-prop", "This rule handler will check for object propeties", (EPM_rule_handler_t)RH_check_SAP_transfer_prop);

	iRetCode = EPM_register_rule_handler("RH-check-checkedout-objects", "This rule handler will check for object propeties", (EPM_rule_handler_t)RH_check_checkedout_objects);

	iRetCode = EPM_register_rule_handler("RH-check-bom-bop-status", "This rule handler will check release status of MBOM and BOP", (EPM_rule_handler_t)RH_check_bom_bop_status);

	iRetCode = EPM_register_rule_handler("RH-check-workflow-process-status", "This rule handler will check if workflow is applied already", (EPM_rule_handler_t)RH_check_workflow_process_status);

	iRetCode = EPM_register_rule_handler("RH-check-task-state", "This action handler will create sourcing and costing object", (EPM_rule_handler_t)RH_check_task_state);

	iRetCode = EPM_register_rule_handler("RH-check-revision-release-status", "This rule handler will check if 01 Revision present in solutions", (EPM_rule_handler_t)RH_check_revision_release_status);

	iRetCode = EPM_register_rule_handler("RH-check-release-assembly-escooter", "This rule handler will check if all child parts of assembly has been released or in process", (EPM_rule_handler_t)RH_check_release_assembly_escooter);

	iRetCode = EPM_register_rule_handler("RH-part-not-include-release-statuses", "this rule handler will check if all Item Revision dont have release status. Otherwise, false", (EPM_rule_handler_t)RH_Part_Not_Include_Release_Statuses);

	iRetCode = EPM_register_rule_handler("RH-check-revision-baseline", "Checking basline flag of a specific revision", (EPM_rule_handler_t)RH_check_revision_baseline);

	iRetCode = EPM_register_rule_handler("RH-is-include-baseline-revs", "Checking basline flag of a specific revision", (EPM_rule_handler_t)RH_is_include_baseline_revisions);

	iRetCode = EPM_register_rule_handler("RH-is-ECN-valid-to-trigger", "Checking ECRN valid to trigger", (EPM_rule_handler_t)RH_is_ECN_valid_to_trigger);

	iRetCode = EPM_register_rule_handler("RH_check_object_properties", "Checking properties of objects", (EPM_rule_handler_t)RH_check_object_properties);

	iRetCode = EPM_register_rule_handler("RH_check_dde_status", "Checking DDE Status", (EPM_rule_handler_t)RH_check_dde_status);

	iRetCode = EPM_register_rule_handler("RH_check_dde_storage_exception", "Checking DDE Exception", (EPM_rule_handler_t)RH_check_dde_storage_exception);

	/*Action handlers*/
	iRetCode = EPM_register_action_handler("AH-attach-cost-form", "This action handler will attach cost forms under cost impact pseudo folder", (EPM_action_handler_t)AH_attach_cost_form);

	iRetCode = EPM_register_action_handler("AH-send-email-to-all", "This action handler will email notification", (EPM_action_handler_t)AH_send_email_to_all);

	iRetCode = EPM_register_action_handler("AH-send-email-to-all-participants", "This action handler will send email notification", (EPM_action_handler_t)AH_send_email_to_all_participants);

	iRetCode = EPM_register_action_handler("AH-send-email-to-all-participants-scooter", "This action handler will send email notification", (EPM_action_handler_t)AH_send_email_to_all_participants_scooter);

	iRetCode = EPM_register_action_handler("AH-send-mail-notification-workflow-participants", "This action handler will send email notification", (EPM_action_handler_t)AH_send_mail_notification_workflow_participants);

	iRetCode = EPM_register_action_handler("AH-change-ownership-cl-to-cl", "This action handler will change ownership", (EPM_action_handler_t)AH_change_ownership_cl_to_cl);

	iRetCode = EPM_register_action_handler("AH-access-cl-to-buyer", "This action handler will provide access to buyer", (EPM_action_handler_t)AH_assign_access_cl_to_buyer);

	iRetCode = EPM_register_action_handler("AH-create-purchasing-object", "This action handler will create sourcing and costing object", (EPM_action_handler_t)AH_create_purchasing_object);

	iRetCode = EPM_register_action_handler("AH-attach-manufac-form", "this action handler will create forms", (EPM_action_handler_t)AH_attach_manufac_form);

	iRetCode = EPM_register_action_handler("AH-add-release-status-sol-items", "This action handler will add release status to items", (EPM_action_handler_t)AH_add_release_sts_sol_items);

	iRetCode = EPM_register_action_handler("AH-add-release-status-sol-items-scooter", "This action handler will add release status to items", (EPM_action_handler_t)AH_add_release_sts_sol_items_scooter);

	//iRetCode = EPM_register_action_handler("AH-route-process-by-dataset","This action handler route process based on input dataset",(EPM_action_handler_t)AH_route_process_by_dataset);

	iRetCode = EPM_register_action_handler("AH-MCR-create-purchasing-object", "This action handler will create sourcing object", (EPM_action_handler_t)AH_MCR_create_purchasing_object);

	iRetCode = EPM_register_action_handler("AH-create-plant-form-afs", "This action handler will update plant code escooter part revision based on some logic", (EPM_action_handler_t)AH_create_plant_form_afs);

	iRetCode = EPM_register_action_handler("AH-create-plant-forms", "This action handler will update plant codes based on part make buy and inputs", (EPM_action_handler_t)AH_create_or_fill_plant_form);

	iRetCode = EPM_register_action_handler("AH-update-plant-code-escooter", "This action handler will update plant code escooter part revision based on some logic", (EPM_action_handler_t)AH_create_escooter_plant_form);

	iRetCode = EPM_register_action_handler("AH-send-parts-transfer-request", "This action handler will update plant code escooter part revision based on some logic", (EPM_action_handler_t)AH_send_parts_transfer_request);

	iRetCode = EPM_register_action_handler("AH-notify", "This action handler sends email notification to individual task", (EPM_action_handler_t)AH_notify);

	iRetCode = EPM_register_action_handler("AH-notify-scooter-release-all", "This action handler send email notification for Scooter Release process, ONLY.", (EPM_action_handler_t)AH_notify_scooter_relase_all);

	iRetCode = EPM_register_action_handler("AH-assign-workflow-tasks", "Assign task based on preference.", (EPM_action_handler_t)AH_assign_workflow_tasks);

	iRetCode = EPM_register_action_handler("AH-send-mail-EPM_noti-participants", "This action handler will send email notification", (EPM_action_handler_t)AH_send_mail_EPM_noti_workflow_participants);

	iRetCode = EPM_register_action_handler("AH-create_ecn", "This action handler creates ecn object based on ecr", (EPM_action_handler_t)AH_create_ecn);

	iRetCode = EPM_register_action_handler("AH-set-number-proprety", "This action handler set property for object", (EPM_action_handler_t)AH_set_number_property);

	iRetCode = EPM_register_action_handler("AH-cpy-piece-cost-form", "This action handler set property for object", (EPM_action_handler_t)AH_cpy_piece_cost_form);

	iRetCode = EPM_register_action_handler("AH-create_mcn", "This action handler creates mcn object based on change request", (EPM_action_handler_t)AH_create_mcn);

	iRetCode = EPM_register_action_handler("AH-copy-part-to-ecn-storage", "This action handler add list part from solution to DDE storage", (EPM_action_handler_t)AH_copy_part_to_ecn_storage);

	iRetCode = EPM_register_action_handler("AH-create-dde-from-ecn-storage", "This action handler creates dde object based on DDE storage", (EPM_action_handler_t)AH_create_dde_from_ecn_storage);

	iRetCode = EPM_register_action_handler("AH-jes-sos-notification-worklow", "This action handler creates dde object based on DDE storage", (EPM_action_handler_t)AH_jes_sos_notification_worklow);
	
	iRetCode = EPM_register_action_handler("AH-send-mail-update-status-jes", "This action handler creates dde object based on DDE storage", (EPM_action_handler_t)AH_send_mail_update_status_jes);
	
	iRetCode = EPM_register_action_handler("AH-set-reviewer-to-jes-form", "This action handler creates dde object based on DDE storage", (EPM_action_handler_t)AH_set_reviewer_to_jes_form);

	return iRetCode;
}

extern "C" DLLAPI int VF_handlers_register_callbacks()
{
	int iRetCode = ITK_ok;
	iRetCode = CUSTOM_register_exit("VF_handlers", "USER_gs_shell_init_module", (CUSTOM_EXIT_ftn_t)VF_handlers_register_handlers);
	printf("\n Registered Successfully   --  VF_handlers Project\n");
	return iRetCode;
}

std::vector<std::string> split_string(std::string str, char delimiter)
{
	std::vector<std::string> o;
	std::string s;
	istringstream f(str);
	while (getline(f, s, delimiter)) {
		o.push_back(s);
	}
	return o;
}

std::vector<string> split_string2(std::string s, std::string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

logical has_valid_status(char* part_status_list, char* target_status_list)
{
	int		ret_code = ITK_ok;
	char* a_target_status = nullptr;
	char* a_part_status = nullptr;
	char* cpy_target_status_list = nullptr;
	logical is_valid = false;

	vector<string> vec_target_status;
	vector<string> vec_part_status;

	cpy_target_status_list = static_cast<char*>(MEM_alloc((static_cast<int>(tc_strlen(target_status_list)) + 1) * sizeof(char)));
	tc_strcpy(cpy_target_status_list, target_status_list);

	a_target_status = tc_strtok(cpy_target_status_list, ",");
	while (a_target_status != nullptr)
	{
		vec_target_status.push_back(a_target_status);
		a_target_status = tc_strtok(nullptr, ",");
	}

	a_part_status = tc_strtok(part_status_list, ",");
	while (a_part_status != nullptr)
	{
		vec_part_status.push_back(a_part_status);
		a_part_status = tc_strtok(nullptr, ",");
	}

	for (int iCnt = 0; iCnt < vec_target_status.size(); iCnt++)
	{
		ltrim(vec_target_status[iCnt]);
		rtrim(vec_target_status[iCnt]);
		for (int iSts = 0; iSts < vec_part_status.size(); iSts++)
		{
			ltrim(vec_part_status[iSts]);
			rtrim(vec_part_status[iSts]);
			if (tc_strcmp(vec_target_status[iCnt].c_str(), vec_part_status[iSts].c_str()) == 0)
			{
				is_valid = true;
				break;
			}
			else
			{
				is_valid = false;
			}
		}

		if (is_valid == true)
		{
			break;
		}
	}

	SAFE_MEM_free(cpy_target_status_list)
		return is_valid;
}

std::string getCurrentTimeStampStr()
{
	std::string str;
	std::tm* tm;
	std::time_t ctm = 0;
	char buffer[80];
	time(&ctm);
	tm = localtime(&ctm);
	strftime(buffer, sizeof(buffer), "%d%m%Y%H%M%S", tm);
	str = std::string(buffer);
	return str;
}

bool containElement(std::vector<string> v, std::string key) {
	if (std::count(v.begin(), v.end(), key)) {
		return true;
	}
	else {
		return false;
	}
	return false;
}

char* getPreferenceValue(char* prefName) {
	int	prefCount = 0;
	char* prefValue = NULL;
	int	iRetCode = ITK_ok;
	if (iRetCode == ITK_ok)
	{
		CHECK_ITK(iRetCode, PREF_ask_value_count(prefName, &prefCount));
	}
	if (iRetCode == ITK_ok && prefCount != 0)
	{
		CHECK_ITK(iRetCode, PREF_ask_char_value(prefName, 0, &prefValue));
	}
	return prefValue;
}

std::string removeSpaces(std::string str1) {
	str1.erase(remove_if(str1.begin(), str1.end(), isspace), str1.end());
	return str1;
}

void sendEmail(char* mailSubject, std::string mailBody, std::vector<std::string> mailToVector, logical isDebug) {
	int iRetCode = 0;
	std::string mailTo;
	Teamcenter::scoped_smptr<char> newMailBody;
	Teamcenter::scoped_smptr<char> newMailSubject;
	char* mailSendType = getPreferenceValue("VF_PLM_MAIL_TYPE");
	
	if (tc_strcmp(mailSendType, "Karaf") == 0) {
		for (std::string email : mailToVector)
		{
			mailTo.append(removeSpaces(email));
			mailTo.append(";");
		}
		CHECK_ITK(iRetCode, STRNG_replace_str(mailSubject, "\"" , "\\\"" , &newMailSubject));
		CHECK_ITK(iRetCode, STRNG_replace_str(mailBody.c_str(), "\"" , "\\\"" , &newMailBody));
		char* fileLocation = getPreferenceValue("VF_PLM_MAIL_LOCATION");
		std::string mailBodyPath;
		mailBodyPath.append(fileLocation);
		mailBodyPath.append("\\");
		mailBodyPath.append(getCurrentTimeStampStr());
		mailBodyPath.append("_");
		mailBodyPath.append(std::to_string(rand()));
		mailBodyPath.append(".json");

		ofstream mailBodyFile(mailBodyPath);
		mailBodyFile << "{to:\"";
		mailBodyFile << mailTo;
		mailBodyFile << "\",";
		mailBodyFile << "subject:\"";
		mailBodyFile << newMailSubject.getString();
		mailBodyFile << "\",";
		mailBodyFile << "message:\"";
		mailBodyFile << newMailBody.getString();
		mailBodyFile << "\"}";
		mailBodyFile.flush();
		mailBodyFile.close();
	}
	else {
		char* mailServerName = getPreferenceValue("Main_server_name");
		char* mailServerPort = getPreferenceValue("Main_server_port");
		char* mailFrom = getPreferenceValue("Main_OS_from_address");
		char* authActive = getPreferenceValue("Mail_server_authentication_activated");
		char* authID = getPreferenceValue("Mail_server_authentication_id");
		char* authPassword = getPreferenceValue("Mail_server_authentication_passwd_location");
		char* charSet = getPreferenceValue("Mail_server_charset");
		char* mailListPath = NULL;
		char* mailBodyPath = NULL;

		for (std::string email : mailToVector)
		{
			mailTo.append(email);
			mailTo.append("\n");
		}

		mailBodyPath = USER_new_file_name("cr_notify_dset", "TEXT", "html", 1);
		ofstream mailBodyFile(mailBodyPath);
		mailBodyFile << newMailBody.getString();
		mailBodyFile.flush();
		mailBodyFile.close();

		mailListPath = USER_new_file_name("cr_notify_dset", "TEXT", "txt", 1);
		ofstream mailListFile(mailListPath);
		mailListFile << mailTo;
		mailListFile.flush();
		mailListFile.close();

		char caMailDetails[BUFSIZ + 1];
		caMailDetails[0] = '\0';
		tc_strcat(caMailDetails, "%TC_BIN%\\tc_mail_smtp ");
		//Mail To
		tc_strcat(caMailDetails, "-to_list_file=\"");
		tc_strcat(caMailDetails, mailListPath);
		tc_strcat(caMailDetails, "\" ");
		//Mail server name
		tc_strcat(caMailDetails, "-server=\"");
		tc_strcat(caMailDetails, mailServerName);
		tc_strcat(caMailDetails, "\" ");
		//Mail server port
		tc_strcat(caMailDetails, "-port=\"");
		tc_strcat(caMailDetails, mailServerPort);
		tc_strcat(caMailDetails, "\" ");
		//Mail from
		tc_strcat(caMailDetails, "-user=\"");
		tc_strcat(caMailDetails, mailFrom);
		tc_strcat(caMailDetails, "\" ");
		//Mail subject
		tc_strcat(caMailDetails, "-subject=\"");
		tc_strcat(caMailDetails, newMailSubject.getString());
		tc_strcat(caMailDetails, "\" ");
		//Mail body
		tc_strcat(caMailDetails, "-body=\"");
		tc_strcat(caMailDetails, mailBodyPath);
		tc_strcat(caMailDetails, "\" ");
		//Auth
		tc_strcat(caMailDetails, "-auth=\"");
		tc_strcat(caMailDetails, authActive);
		tc_strcat(caMailDetails, "\" ");
		//Auth ID
		tc_strcat(caMailDetails, "-authId=\"");
		tc_strcat(caMailDetails, authID);
		tc_strcat(caMailDetails, "\" ");
		//Auth Password
		tc_strcat(caMailDetails, "-pf=\"");
		tc_strcat(caMailDetails, authPassword);
		tc_strcat(caMailDetails, "\" ");
		//Charset
		tc_strcat(caMailDetails, "-charset=\"");
		tc_strcat(caMailDetails, charSet);
		tc_strcat(caMailDetails, "\" ");

		TC_write_syslog("\n[VF] cmd=%s\n retcode=%d \n", caMailDetails);

		if (!isDebug) {
			remove(mailListPath);
			remove(mailBodyPath);
		}
	}
}

std::string genHtmlTitle(char* value) {
	std::stringstream returnValue;
	returnValue << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><b><span style='font-family:Arial;color:#448DA6'>" << value << "<o:p></o:p></span></b></p></div>";
	return returnValue.str();
}

std::string genHtmlPropertyName(char* value) {
	std::stringstream returnValue;
	returnValue << "<td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:Arial;color:gray'>" << value << "<o:p></o:p></span></p></td>";
	return returnValue.str();
}

std::string genHtmlPropertyValue(std::string value) {
	std::stringstream returnValue;
	returnValue << "<td valign=top style='padding:.75pt .75pt .75pt .75pt'><p class=MsoNormal><span style='font-family:Arial'>" << value << "<o:p></o:p></span></p></td>";
	return returnValue.str();
}

std::string genHtmlTableHeader(char* value) {
	std::stringstream returnValue;
	returnValue << "<td style='border:solid gray 1.0pt;background:#F0F0F0;padding:1.5pt 1.5pt 1.5pt 1.5pt'>" << value << "</td>";
	return returnValue.str();
}

std::string genHtmlTableBody(char* value) {
	std::stringstream returnValue;
	returnValue << "<td style='border:solid gray 1.0pt;border-top:none;padding:1.5pt 1.5pt 1.5pt 1.5pt'>" << value << "</td>";
	return returnValue.str();
}

std::string genHtmlLink(std::string link, char* name) {
	std::stringstream returnValue;
	returnValue << "<div style='margin-bottom:2.25pt'><p class=MsoNormal><span style='font-family:Arial'><a href='" << link << "'>" << name << "</a><o:p></o:p></span></p></div>";
	return returnValue.str();
}

std::string genHtmlNote(char* value) {
	std::stringstream returnValue;
	returnValue << "<div><p class=MsoNormal><b><span style='font-family:Arial;color:gray'>"<<value<<"<o:p></o:p></span></b></p></div>";
	return returnValue.str();
}

char* getUserMailOfTask(tag_t task) {
	int	iRetCode = ITK_ok;
	tag_t tagUser = NULLTAG;
	char* userMail = NULL;

	CHECK_ITK(iRetCode, AOM_ask_value_tag(task, "fnd0Performer", &tagUser));
	CHECK_ITK(iRetCode, EPM_get_user_email_addr(tagUser, &userMail));
	if (tc_strcmp(userMail, "") != NULL)
	{
		return userMail;
	}
	return NULL;
}

std::string getNextStatus(const std::string &currentStatus)
{
	std::map<std::string, std::string> currentAndNextStatus;
	currentAndNextStatus["VF4_Sourcing"] = "VF4_SCR";
	currentAndNextStatus["VF4_SCR"] = "VF4_SCR";
	currentAndNextStatus["VL5_P"] = "VF4_PECR";
	currentAndNextStatus["VF4_PECR"] = "VF4_PECR";
	currentAndNextStatus["Vl5_I"] = "VF4_IECR";
	currentAndNextStatus["VF4_IECR"] = "VF4_IECR";
	currentAndNextStatus["PR4D_PR"] = "Vf6_PPR";
	currentAndNextStatus["Vf6_PPR"] = "Vf6_PPR";

	return currentAndNextStatus[currentStatus];
}

std::string getDefaultStatusProgression()
{
	std:string statusProgression;
	statusProgression.assign("VF4_Sourcing,VF4_SCR,VL5_P,VF4_PECR, Vl5_I,VF4_IECR,PR4D_PR,Vf6_PPR");
	return statusProgression;
}

std::string getLatestValidStatus(const std::string &statusProgressionInput, tag_t tRev, const char* isDebug)
{

	int			iTemp = 0;
	char* pcItemRev = NULL;
	tag_t		tItem = NULLTAG;
	tag_t		tLatestValidReleasedRev = NULLTAG;
	char* pcItem = NULL;
	string		szItemRev = "";
	string		szPrevRev = "";
	int iBvr = 0;
	int iRetCode = ITK_ok;
	std::map<std::string, int> allowedStatusesAndAge;

	std::string statusProgression(statusProgressionInput);
	/*prepare status list*/
	if (statusProgression.empty())
	{
		statusProgression = getDefaultStatusProgression();
	}

	std::vector<std::string> statuses = split_string(statusProgression, ',');
	for (int i = 0; i < statuses.size(); i++)
	{
		std::string statusTmp = statuses[i];
		LOG(TC_write_syslog("\n[VF] before trim: %s", statusTmp.c_str()));
		ltrim(statusTmp);
		rtrim(statusTmp);
		allowedStatusesAndAge[statusTmp] = i;
		LOG(TC_write_syslog("\n[VF] %s", statusTmp.c_str()));
	}

	LOG(TC_write_syslog("\n[VF] Prepare status list done."));

	/*get previous revision*/
	CHECK_ITK(iRetCode, ITEM_ask_rev_id2(tRev, &pcItemRev));
	CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(tRev, &tItem));
	CHECK_ITK(iRetCode, ITEM_ask_id2(tItem, &pcItem));

	szItemRev.assign(pcItemRev);
	iTemp = stoi(szItemRev);
	iTemp--;

	if (to_string(iTemp).length() == 1)
	{
		szPrevRev.assign("0");
		szPrevRev.append(to_string(iTemp));
	}
	else
	{
		szPrevRev.assign(to_string(iTemp));
	}

	LOG(TC_write_syslog("\n[VF] previous rev: %s.", szPrevRev.c_str()));

	int			iSts = 0;
	tag_t* ptRelSts = NULLTAG;
	tag_t* ptBvr = NULLTAG;
	char* pcStsName = NULL;

	/*getting latest release status of prev rev*/
	tag_t tRevRule = NULLTAG;
	char* szHowConfigured = NULL;
	CHECK_ITK(iRetCode, CFM_find(__ADMIN_LATEST_VALID_STATUS, &tRevRule));
	if (iRetCode == ITK_ok && tRevRule != NULLTAG)
	{
		CHECK_ITK(iRetCode, CFM_item_ask_configured(tRevRule, tItem, &tLatestValidReleasedRev, &szHowConfigured));
		LOG(TC_write_syslog("\n[VF] %s", szHowConfigured));
		SAFE_MEM_free(szHowConfigured);
	}
	else
	{
		TC_write_syslog("\n[VF] WARNING: CANNOT FIND REV RULE %s", __ADMIN_LATEST_VALID_STATUS);
		iRetCode = ITK_ok;
	}

	if (tLatestValidReleasedRev != NULLTAG)
	{
		CHECK_ITK(iRetCode, AOM_refresh(tLatestValidReleasedRev, FALSE));
		CHECK_ITK(iRetCode, WSOM_ask_release_status_list(tLatestValidReleasedRev, &iSts, &ptRelSts));
		SAFE_SM_FREE(pcItemRev);
		CHECK_ITK(iRetCode, ITEM_ask_rev_id2(tLatestValidReleasedRev, &pcItemRev));
		LOG(TC_write_syslog("\n[VF] No. status of part %s/%s=%d.", pcItem, pcItemRev, iSts));
	}
	else
	{
		CHECK_ITK(iRetCode, AOM_refresh(tRev, FALSE));
		CHECK_ITK(iRetCode, WSOM_ask_release_status_list(tRev, &iSts, &ptRelSts));
		LOG(TC_write_syslog("\n[VF] No. status of part %s/%s=%d.", pcItem, pcItemRev, iSts));
		if ((iSts == 0 || ptRelSts == NULLTAG) && szPrevRev.size() > 0)
		{
			iSts = 0;
			ptRelSts = NULLTAG;
			CHECK_ITK(iRetCode, ITEM_find_revision(tItem, szPrevRev.c_str(), &tLatestValidReleasedRev));
			if (tLatestValidReleasedRev == NULLTAG)
			{
				std::string szRevIdTemp("0");
				szRevIdTemp.append(szPrevRev);
				CHECK_ITK(iRetCode, ITEM_find_revision(tItem, szRevIdTemp.c_str(), &tLatestValidReleasedRev));
			}

			if (tLatestValidReleasedRev != NULLTAG)
			{
				CHECK_ITK(iRetCode, WSOM_ask_release_status_list(tLatestValidReleasedRev, &iSts, &ptRelSts));
				LOG(TC_write_syslog("\n[VF] No. status of part %s/%s=%d (current rev does not have status).", pcItem, szPrevRev.c_str(), iSts));
			}
		}
	}

	std::string latestValidStatus("");
	if (ptRelSts != NULLTAG && iSts > 0)
	{
		// loop status list to find the latest allowed status

		int oldestAge = -999999;
		for (int i = 0; i < iSts; i++)
		{
			CHECK_ITK(iRetCode, RELSTAT_ask_release_status_type(ptRelSts[i], &pcStsName));
			if (pcStsName != NULL && allowedStatusesAndAge.find(pcStsName) != allowedStatusesAndAge.end())
			{
				int age = allowedStatusesAndAge[pcStsName];
				if (age > oldestAge)
				{
					oldestAge = age;
					latestValidStatus = pcStsName;
				}
			}
			SAFE_SM_FREE(pcStsName);
		}
	}

	return latestValidStatus;
}

std::string getValidChildrenStatusesStr(const std::string& currentStatus)
{
	std::map<std::string, std::string> parentStatusAndValidChildrenStatuses;
	parentStatusAndValidChildrenStatuses["VF4_Sourcing"]   = "";
	parentStatusAndValidChildrenStatuses["VF4_SCR"]        = "";
	parentStatusAndValidChildrenStatuses["VL5_P"]          = "VL5_P,VF4_PECR,Vl5_I,VF4_IECR,PR4D_PR,Vf6_PPR";
	parentStatusAndValidChildrenStatuses["VF4_PECR"]       = "VL5_P,VF4_PECR,Vl5_I,VF4_IECR,PR4D_PR,Vf6_PPR";
	parentStatusAndValidChildrenStatuses["Vl5_I"]          = "Vl5_I,VF4_IECR,PR4D_PR,Vf6_PPR";
	parentStatusAndValidChildrenStatuses["VF4_IECR"]       = "Vl5_I,VF4_IECR,PR4D_PR,Vf6_PPR";
	parentStatusAndValidChildrenStatuses["PR4D_PR"]        = "PR4D_PR,Vf6_PPR";
	parentStatusAndValidChildrenStatuses["Vf6_PPR"]        = "PR4D_PR,Vf6_PPR";

	return parentStatusAndValidChildrenStatuses[currentStatus];
}