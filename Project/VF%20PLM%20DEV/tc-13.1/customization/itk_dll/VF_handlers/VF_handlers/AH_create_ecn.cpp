/*******************************************************************************
File         : AH_create_ecn.cpp

Description  : 	Auto Create ECN after ECR approved

Input        : None

Output       : None

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
April,20 2022     1.0         Pallavi		 Initial Creation

/*******************************************************************************/
#include "Vinfast_Custom.h"
#include <map>
#include <iterator>
#include <base_utils/ScopedSmPtr.hxx>
#include <base_utils/ScopedPtr.hxx>
#include "Fnd0Participant/participant.h"

#define Error (EMH_USER_error_base +284)
int getPart(std::string partNumber, std::string partType, tag_t* tPart);
int assignEcnProcess(tag_t tNewChange, tag_t newChangeRev, tag_t ptTgt, std::string ecnId, std::string ecnWorkflow, std::string failToTriggerSubject, std::string failToTriggerRecipients, std::string pcUserMail);
extern int AH_create_ecn(EPM_action_message_t msg)
{
	TC_write_syslog("[VF] ENTER %s \n", __FUNCTION__);

	int			iRetCode = ITK_ok;
	int			iNumArgs = 0;
	int			iPrefCount = 0;
	int         valCount = 0;

	char		ErrorMessage[2048] = "";
	const char* ecrObjType = NULLTAG;
	const char* ecnObjType = NULLTAG;

	tag_t		tRootTask = NULLTAG;
	Teamcenter::scoped_smptr<char*> prefValues;
	Teamcenter::scoped_smptr<char> ecrImplemetsBy;
	Teamcenter::scoped_smptr<char> ecnItemType;
	Teamcenter::scoped_smptr<char> ecrItemType;
	Teamcenter::scoped_smptr<char> attrsToCopy;
	Teamcenter::scoped_smptr<char> relationsToCopy;
	Teamcenter::scoped_smptr<char> participantsToCopy;
	Teamcenter::scoped_smptr<char> mailRecipientsList;
	Teamcenter::scoped_smptr<char> mailSubject;
	Teamcenter::scoped_smptr<char> ecnWorkflow;
	Teamcenter::scoped_smptr<char> failToCreateRecipients;
	Teamcenter::scoped_smptr<char> failToCreateSubject;
	Teamcenter::scoped_smptr<char> failToTriggerRecipients;
	Teamcenter::scoped_smptr<char> failToTriggerSubject;
	Teamcenter::scoped_smptr<char> failToCopyRecipients;
	Teamcenter::scoped_smptr<char> failToCopySubject;

	char pcMailSubject[2048] = "";
	std::stringstream mailBody;
	std::vector<string> szMailList;

	iNumArgs = TC_number_of_arguments(msg.arguments);
	TC_write_syslog("TC_number_of_arguments %d\n", iNumArgs);
	if (iNumArgs > 0)
	{
		for (int i = 0; i < iNumArgs; i++)
		{
			Teamcenter::scoped_smptr<char> pcFlag;
			Teamcenter::scoped_smptr<char> pcValue;

			/*getting arguments*/
			CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue));
			if (iRetCode == ITK_ok)
			{
				if (tc_strcmp(pcFlag.getString(), "ECN-ECR-relation") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &ecrImplemetsBy));
				}
				if (tc_strcmp(pcFlag.getString(), "ECR-item-type") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &ecrItemType));
				}
				if (tc_strcmp(pcFlag.getString(), "ECN-item-type") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &ecnItemType));
				}
				if (tc_strcmp(pcFlag.getString(), "Attributes-to-copy") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &attrsToCopy));
				}
				if (tc_strcmp(pcFlag.getString(), "Relations-to-copy") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &relationsToCopy));
				}
				if (tc_strcmp(pcFlag.getString(), "Participants-to-copy") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &participantsToCopy));
				}
				if (tc_strcmp(pcFlag.getString(), "Error-mail-recipients-list") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &mailRecipientsList));
				}
				if (tc_strcmp(pcFlag.getString(), "Error-mail-subject") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &mailSubject));
				}
				if (tc_strcmp(pcFlag.getString(), "ECN-workflow") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &ecnWorkflow));
				}
				if (tc_strcmp(pcFlag.getString(), "Fail-to-create-recipients") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &failToCreateRecipients));
				}
				if (tc_strcmp(pcFlag.getString(), "Fail-to-create-subject") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &failToCreateSubject));
				}
				if (tc_strcmp(pcFlag.getString(), "Fail-to-trigger-recipients") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &failToTriggerRecipients));
				}
				if (tc_strcmp(pcFlag.getString(), "Fail-to-trigger-subject") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &failToTriggerSubject));
				}
				if (tc_strcmp(pcFlag.getString(), "Fail-to-copy-solutions-recipients") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &failToCopyRecipients));
				}
				if (tc_strcmp(pcFlag.getString(), "Fail-to-copy-solutions-subject") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &failToCopySubject));
				}
			}
		}
	}
	else
	{
		iRetCode = EPM_invalid_argument;
	}
	if (tc_strcmp(attrsToCopy.getString(), "") == 0)
	{
		sprintf(ErrorMessage, "Mandatory Input Attributes-to-copy  not provided");
		CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
		return Error;
	}
	if (tc_strcmp(relationsToCopy.getString(), "") == 0)
	{
		sprintf(ErrorMessage, "Mandatory Input Relations-to-copy not provided");
		CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
		return Error;
	}
	int prefCount = 0;
	logical prefValue = false;
	CHECK_ITK(iRetCode, PREF_ask_value_count("VF_SKIP_AUTO_CREATE_ECN", &prefCount));
	if (prefCount > 0)
	{
		CHECK_ITK(iRetCode, PREF_ask_logical_value("VF_SKIP_AUTO_CREATE_ECN", 0, &prefValue));
	}

	TC_write_syslog("Preference Value[%d]\n", prefValue);
	if (prefValue)
	{
		TC_write_syslog("Preference value is true, skipping the ECN creation.\n");
		return ITK_ok;
	}

	if (tc_strcmp(ecrItemType.getString(), "") == 0)
	{
		ecrObjType = ("Vf6_ECR");
	}
	else {
		ecrObjType = (ecrItemType.getString());
	}
	if (tc_strcmp(ecnItemType.getString(), "") == 0)
	{
		ecnObjType = ("Vf6_ECN");
	}
	else {
		ecnObjType = (ecnItemType.getString());
	}

	/*getting root task*/
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int	iTgt = 0;
		tag_t tNewChange = NULLTAG;
		tag_t newChangeRev = NULLTAG;
		Teamcenter::scoped_smptr<char> ecrId;
		Teamcenter::scoped_smptr<char> ecnId;
		Teamcenter::scoped_smptr<char> pcUserMail;
		Teamcenter::scoped_smptr<tag_t> ptTgt;

		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		//Looping the targets
		for (int iLoopTarget = 0; iLoopTarget < iTgt; iLoopTarget++)
		{
			tag_t ecnType = NULLTAG;
			tag_t ecrItem = NULLTAG;
			tag_t tOwningUser = NULLTAG;
			tag_t item_create_input_tag = NULLTAG;
			tag_t rev_create_input_tag = NULLTAG;

			Teamcenter::scoped_smptr<char> objType;
			Teamcenter::scoped_smptr<char> ecrRevType;

			CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iLoopTarget], &ecrItem));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iLoopTarget], "object_type", &ecrRevType));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ecrItem, "object_type", &objType));
			CHECK_ITK(iRetCode, AOM_ask_value_tag(ecrItem, "owning_user", &tOwningUser));
			CHECK_ITK(iRetCode, EPM_get_user_email_addr(tOwningUser, &pcUserMail));
			if (tc_strcmp(objType.getString(), ecrObjType) == 0)
			{
				CHECK_ITK(iRetCode, TCTYPE_find_type(ecnObjType, NULLTAG, &ecnType));
				CHECK_ITK(iRetCode, TCTYPE_construct_create_input(ecnType, &item_create_input_tag));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iLoopTarget], "item_id", &ecrId));
				CHECK_ITK(iRetCode, STRNG_replace_str(ecrId.getString(), "ECR", "ECN", &ecnId));

				const char* value[] = { ecnId.getString() };
				CHECK_ITK(iRetCode, TCTYPE_set_create_display_value(item_create_input_tag, "item_id", 1, value));


				// copying attributes from ecr to ecn
				int  attrCount = 0;
				logical revAttrFlag = false;
				Teamcenter::scoped_smptr<char*> attrValues;
				CHECK_ITK(iRetCode, EPM__parse_string(attrsToCopy.getString(), ",", &attrCount, &attrValues));
				for (int iAtrCount = 0; iAtrCount < attrCount; iAtrCount++)
				{
					Teamcenter::scoped_smptr<char> propValue;
					Teamcenter::scoped_smptr<char> propCheck;
					propCheck = strstr(attrValues[iAtrCount], ":");
					if (propCheck != NULL)
					{
						int  tokenCount = 0;
						tag_t tItem = NULLTAG;
						tag_t classId = NULLTAG;
						logical itemRevisionExists = false;

						Teamcenter::scoped_smptr<char> propValue;
						Teamcenter::scoped_smptr<char*> tokenValue;
						Teamcenter::scoped_smptr<char> className;

						CHECK_ITK(iRetCode, POM_class_of_instance(ptTgt[0], &classId));
						CHECK_ITK(iRetCode, POM_name_of_class(classId, &className));
						CHECK_ITK(iRetCode, EPM__parse_string(attrValues[iAtrCount], ":", &tokenCount, &tokenValue));
						CHECK_ITK(iRetCode, POM_attr_exists(tokenValue[1], className.getString(), &itemRevisionExists));
						if (itemRevisionExists)
						{
							CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[0], tokenValue[1], &propValue));
							if (propValue != NULL)
							{
								tag_t ecnRevType_tag = NULLTAG;
								const char* tmp = (propValue.getString());
								CHECK_ITK(iRetCode, TCTYPE_find_type(ecrRevType.getString(), NULLTAG, &ecnRevType_tag));
								CHECK_ITK(iRetCode, TCTYPE_construct_create_input(ecnRevType_tag, &rev_create_input_tag));
								CHECK_ITK(iRetCode, TCTYPE_set_create_display_value(rev_create_input_tag, tokenValue[1], 1, &tmp));
								revAttrFlag = true;
							}
						}
						else
						{
							sprintf(ErrorMessage, "Cannot find attribute: %s in the target object", attrValues[iAtrCount]);
							CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
							return Error;
						}

					}
					else
					{
						tag_t classId = NULLTAG;
						Teamcenter::scoped_smptr<char> className;
						logical itemExists = false;

						CHECK_ITK(iRetCode, POM_class_of_instance(ecrItem, &classId));
						CHECK_ITK(iRetCode, POM_name_of_class(classId, &className));
						CHECK_ITK(iRetCode, POM_attr_exists(attrValues[iAtrCount], className.getString(), &itemExists));
						if (itemExists)
						{
							CHECK_ITK(iRetCode, AOM_ask_value_string(ecrItem, attrValues[iAtrCount], &propValue));
							if (propValue != NULL)
							{
								const char* tmp = (propValue.getString());
								CHECK_ITK(iRetCode, TCTYPE_set_create_display_value(item_create_input_tag, attrValues[iAtrCount], 1, &tmp));
							}
						}
						else
						{
							sprintf(ErrorMessage, "Cannot find attribute: %s in the target object", attrValues[iAtrCount]);
							CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
							return Error;
						}
					}
				}
				if (revAttrFlag)
				{
					CHECK_ITK(iRetCode, AOM_set_value_tag(item_create_input_tag, "revision", rev_create_input_tag));
				}
				tag_t newChange = NULLTAG;
				tag_t tChangeRev = NULLTAG;
				getPart(ecnId.getString(), ecnObjType, &newChange);
				CHECK_ITK(iRetCode, ITEM_ask_latest_rev(newChange, &tChangeRev));
				if (newChange == NULLTAG)
				{
					iRetCode = TCTYPE_create_object(item_create_input_tag, &tNewChange);
					if (iRetCode != ITK_ok)
					{
						Teamcenter::scoped_smptr<char> error_str;
						EMH_ask_error_text(iRetCode, &error_str);
						sprintf(ErrorMessage, "ERROR MSG:%s.Error in line %d function %s", error_str.getString(), __LINE__, __FUNCTION__);
						if (tc_strcmp(failToCreateSubject.getString(), "") == 0)
						{
							sprintf(pcMailSubject, "[ECN-auto-create] %s is fail to create after %s approved", ecnId.getString(), ecrId.getString());
						}
						else {
							tc_strcpy(pcMailSubject, failToCreateSubject.getString());
						}
						mailBody << "<!DOCTYPE html><html><body>";
						mailBody << ecnId.getString();
						mailBody << " is fail to create.";
						mailBody << ErrorMessage;
						mailBody << "<p><strong>This is auto generated email. Please don't reply</strong></p>";
						mailBody << "</body></html>";
						//preparing mail list
						if (tc_strcmp(failToCreateRecipients.getString(), "") == 0)
						{
							szMailList.push_back(pcUserMail.getString());
						}
						else
						{
							int  recipientsCount = 0;
							Teamcenter::scoped_smptr<char*> recipientsValues;
							CHECK_ITK(iRetCode, EPM__parse_string(failToCreateRecipients.getString(), ",", &recipientsCount, &recipientsValues));
							for (int iRecCount = 0; iRecCount < recipientsCount; iRecCount++)
							{
								szMailList.push_back(recipientsValues.getString()[iRecCount]);
							}
						}
						sendEmail(pcMailSubject, mailBody.str(), szMailList, false);
						return iRetCode;
					}
					CHECK_ITK(iRetCode, AOM_save_with_extensions(tNewChange));
					CHECK_ITK(iRetCode, ITEM_ask_latest_rev(tNewChange, &newChangeRev));
					CHECK_ITK(iRetCode, AOM_save_with_extensions(newChangeRev));
				}
				else
				{
					tag_t user_tag = NULLTAG;
					tag_t group_tag = NULLTAG;
					Teamcenter::scoped_smptr<char> user_name_string;
					Teamcenter::scoped_smptr<char> groupname;

					CHECK_ITK(iRetCode, POM_get_user(&user_name_string, &user_tag));
					CHECK_ITK(iRetCode, SA_ask_user_login_group(user_tag, &group_tag));
					CHECK_ITK(iRetCode, SA_ask_group_name2(group_tag, &groupname));
					if (tc_strcmp(groupname.getString(), "dba") == 0)
					{
						iRetCode = assignEcnProcess(newChange, tChangeRev, ptTgt[0], ecnId.getString(), ecnWorkflow.getString(), failToTriggerSubject.getString(), failToTriggerRecipients.getString(), pcUserMail.getString());
						return iRetCode;
					}
					else
					{
						if (tc_strcmp(failToTriggerSubject.getString(), "") == 0)
						{
							sprintf(pcMailSubject, "[ECN-auto-create] %s is fail to trigger ECN process", ecnId.getString());
						}
						else {
							tc_strcpy(pcMailSubject, failToTriggerSubject.getString());
						}
						mailBody << "<!DOCTYPE html><html><body>";
						mailBody << ecnId.getString();
						mailBody << " created but fail to trigger ECN process.";
						mailBody << "<p><strong>This is auto generated email. Please don't reply</strong></p>";
						mailBody << "</body></html>";
						//preparing mail list
						if (tc_strcmp(failToTriggerRecipients.getString(), "") == 0)
						{
							szMailList.push_back(pcUserMail.getString());
						}
						else
						{
							int  recipientsCount = 0;
							Teamcenter::scoped_smptr<char*> recipientsValues;
							CHECK_ITK(iRetCode, EPM__parse_string(failToTriggerRecipients.getString(), ",", &recipientsCount, &recipientsValues));
							for (int iRecCount = 0; iRecCount < recipientsCount; iRecCount++)
							{
								szMailList.push_back(recipientsValues.getString()[iRecCount]);
							}
						}
						sendEmail(pcMailSubject, mailBody.str(), szMailList, false);
						return iRetCode;
					}
				}
			}
			else
			{
				sprintf(ErrorMessage, "Target object type does not match with input object type: %s ", objType.getString());
				CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
				return Error;
			}
		}
		//attaching new change object with CMImplements
		tag_t cmImplements = NULLTAG;
		tag_t tCMImplements = NULLTAG;

		CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, TRUE));
		if (tc_strcmp(ecrImplemetsBy.getString(), "") != 0)
		{
			const char* rel = { ecrImplemetsBy.getString() };
			CHECK_ITK(iRetCode, GRM_find_relation_type(rel, &tCMImplements));
			CHECK_ITK(iRetCode, GRM_create_relation(newChangeRev, ptTgt[0], tCMImplements, NULLTAG, &cmImplements));
			CHECK_ITK(iRetCode, GRM_save_relation(cmImplements));
		}
		else {
			CHECK_ITK(iRetCode, GRM_find_relation_type("CMImplements", &tCMImplements));
			CHECK_ITK(iRetCode, GRM_create_relation(newChangeRev, ptTgt[0], tCMImplements, NULLTAG, &cmImplements));
			CHECK_ITK(iRetCode, GRM_save_relation(cmImplements));
		}
		CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, FALSE));
		// copying relations from ecr to ecn
		int  relCount = 0;
		Teamcenter::scoped_smptr<char*> relValues;
		CHECK_ITK(iRetCode, EPM__parse_string(relationsToCopy.getString(), ",", &relCount, &relValues));
		for (int iRelCount = 0; iRelCount < relCount; iRelCount++)
		{
			int iCountSecObjs = 0;
			tag_t tRepresentedBy = NULLTAG;
			tag_t tPraposal = NULLTAG;
			tag_t tAnalyst = NULLTAG;
			tag_t tcUserTag = NULLTAG;
			tag_t tParticipant = NULLTAG;
			Teamcenter::scoped_smptr<tag_t> ptSecondaryObjs;
			Teamcenter::scoped_smptr<char> currentUser;
			if (tc_strcmp(relValues.getString()[iRelCount], "CMHasSolutionItem") == 0)
			{
				//add analyst
				CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, TRUE));
				CHECK_ITK(iRetCode, EPM_get_participanttype("Analyst", &tAnalyst));
				CHECK_ITK(iRetCode, SA_ask_current_groupmember(&tcUserTag));
				CHECK_ITK(iRetCode, EPM_create_participant(tcUserTag, tAnalyst, &tParticipant));
				CHECK_ITK(iRetCode, PARTICIPANT_set_participants(newChangeRev, true, 1, &tParticipant));
				CHECK_ITK(iRetCode, AOM_save_with_extensions(newChangeRev));
				CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, FALSE));

				CHECK_ITK(iRetCode, GRM_find_relation_type(relValues[iRelCount], &tRepresentedBy));
				CHECK_ITK(iRetCode, GRM_find_relation_type("Cm0HasProposal", &tPraposal));
				CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[0], tPraposal, &iCountSecObjs, &ptSecondaryObjs));
				for (int isecIny = 0; isecIny < iCountSecObjs; isecIny++)
				{
					tag_t tcreatedRelation = NULLTAG;
					iRetCode = GRM_create_relation(newChangeRev, ptSecondaryObjs[isecIny], tRepresentedBy, NULLTAG, &tcreatedRelation);
					if (iRetCode != ITK_ok)
					{
						if (tc_strcmp(failToCopySubject.getString(), "") == 0)
						{
							sprintf(pcMailSubject, "[ECN-auto-create]Cannot copy all part from  %s's Proposal to Solutions folder", ecnId.getString());
						}
						else {
							tc_strcpy(pcMailSubject, failToCopySubject.getString());
						}
						mailBody << "<!DOCTYPE html><html><body>";
						mailBody << ecnId.getString();
						mailBody << " created but fail to copy parts from Proposal to Solutions folder";
						mailBody << "<p><strong>This is auto generated email. Please don't reply</strong></p>";
						mailBody << "</body></html>";
						//preparing mail list
						if (tc_strcmp(failToCopyRecipients.getString(), "") == 0)
						{
							szMailList.push_back(pcUserMail.getString());
						}
						else
						{
							int  recipientsCount = 0;
							Teamcenter::scoped_smptr<char*> recipientsValues;
							CHECK_ITK(iRetCode, EPM__parse_string(failToCopyRecipients.getString(), ",", &recipientsCount, &recipientsValues));
							for (int iRecCount = 0; iRecCount < recipientsCount; iRecCount++)
							{
								szMailList.push_back(recipientsValues.getString()[iRecCount]);
							}
						}
						sendEmail(pcMailSubject, mailBody.str(), szMailList, false);
					}
					CHECK_ITK(iRetCode, GRM_save_relation(tcreatedRelation));
				}

				//remove analyst
				CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, TRUE));
				CHECK_ITK(iRetCode, PARTICIPANT_remove_participant(newChangeRev, tParticipant));
				CHECK_ITK(iRetCode, AOM_save_with_extensions(newChangeRev));
				CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, FALSE));
			}
			else {
				CHECK_ITK(iRetCode, GRM_find_relation_type(relValues[iRelCount], &tRepresentedBy));
				CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[0], tRepresentedBy, &iCountSecObjs, &ptSecondaryObjs));
				for (int isecIny = 0; isecIny < iCountSecObjs; isecIny++)
				{
					tag_t tcreatedRelation = NULLTAG;
					CHECK_ITK(iRetCode, GRM_create_relation(newChangeRev, ptSecondaryObjs[isecIny], tRepresentedBy, NULLTAG, &tcreatedRelation));
					CHECK_ITK(iRetCode, GRM_save_relation(tcreatedRelation));
				}
			}
		}
		// copying participant from ecr to ecn
		int  userCount = 0;
		Teamcenter::scoped_smptr<char*> participantValues;
		if (tc_strcmp(participantsToCopy.getString(), "") != 0)
		{
			CHECK_ITK(iRetCode, EPM__parse_string(participantsToCopy.getString(), ",", &userCount, &participantValues));
			CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, TRUE));
			for (int pCount = 0; pCount < userCount; pCount++)
			{
				int iCountSecObjs = 0;
				//tag_t *tParticipants = NULL;
				int tParticipantCount = 0;
				tag_t tParticipantType = NULLTAG;
				tag_t tParticipantGM = NULLTAG;
				tag_t tParticipant = NULLTAG;

				CHECK_ITK(iRetCode, AOM_ask_value_tag(ptTgt[0], participantValues[pCount], &tParticipantGM));
				CHECK_ITK(iRetCode, TCTYPE_ask_type(participantValues[pCount], &tParticipantType));
				if (tParticipantGM != NULLTAG && tParticipantType != NULLTAG)
				{
					//CHECK_ITK(iRetCode, ITEM_rev_ask_participants(ptTgt[0], tParticipantType, &tParticipantCount, &tParticipants));
					CHECK_ITK(iRetCode, EPM_create_participant(tParticipantGM, tParticipantType, &tParticipant));

					if (pCount == 0)
					{
						CHECK_ITK(iRetCode, PARTICIPANT_set_participants(newChangeRev, true, 1, &tParticipant));
					}
					else
					{
						CHECK_ITK(iRetCode, PARTICIPANT_add_participant(newChangeRev, true, tParticipant));
					}
				}
				else
				{
					sprintf(ErrorMessage, "There are no participant \"%s\" assigned in ECR.", participantValues[pCount]);
					CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
				}

			}
			CHECK_ITK(iRetCode, AOM_save_with_extensions(newChangeRev));
			CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, FALSE));
		}
		//transfer ownership of ecn
		tag_t tOwningUser = NULLTAG;
		tag_t tOwningGroup = NULLTAG;
		CHECK_ITK(iRetCode, AOM_ask_value_tag(ptTgt[0], "owning_user", &tOwningUser));
		CHECK_ITK(iRetCode, AOM_ask_value_tag(ptTgt[0], "owning_group", &tOwningGroup));

		CHECK_ITK(iRetCode, AOM_refresh(tNewChange, TRUE));
		CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, TRUE));
		CHECK_ITK(iRetCode, AOM_set_ownership(tNewChange, tOwningUser, tOwningGroup));
		CHECK_ITK(iRetCode, AOM_set_ownership(newChangeRev, tOwningUser, tOwningGroup));
		CHECK_ITK(iRetCode, AOM_save_with_extensions(tNewChange));
		CHECK_ITK(iRetCode, AOM_save_with_extensions(newChangeRev));
		CHECK_ITK(iRetCode, AOM_refresh(tNewChange, FALSE));
		CHECK_ITK(iRetCode, AOM_refresh(newChangeRev, FALSE));

		//attaching process to ecn 
		iRetCode = assignEcnProcess(tNewChange, newChangeRev, ptTgt[0], ecnId.getString(), ecnWorkflow.getString(), failToTriggerSubject.getString(), failToTriggerRecipients.getString(), pcUserMail.getString());
	}
	TC_write_syslog("[VF] EXIT %s \n", __FUNCTION__);
	return iRetCode;
}
int assignEcnProcess(tag_t tNewChange, tag_t newChangeRev, tag_t ptTgt, std::string ecnId, std::string ecnWorkflow, std::string failToTriggerSubject, std::string failToTriggerRecipients, std::string pcUserMail)
{
	int iRetCode = ITK_ok;
	int* attach_types = NULL;
	tag_t  tJob = NULLTAG;
	tag_t  rootTask = NULLTAG;
	tag_t  process = NULLTAG;
	tag_t  ecnTemplate = NULLTAG;
	tag_t  group_tag = NULLTAG;

	std::stringstream mailBody;
	std::vector<string> szMailList;
	char pcMailSubject[2048] = "";
	char ErrorMessage[2048] = "";
	Teamcenter::scoped_smptr<char> objName;
	attach_types = (int*)MEM_alloc(1 * sizeof(int));
	attach_types[0] = EPM_target_attachment;
	std::string processName = "";
	if (tc_strcmp(ecnWorkflow.c_str(), "") != 0)
	{
		CHECK_ITK(iRetCode, AOM_ask_value_string(tNewChange, "object_string", &objName));
		processName.assign(objName.getString());
		processName.append(":");
		processName.append(ecnWorkflow);
		CHECK_ITK(iRetCode, EPM_find_process_template(ecnWorkflow.c_str(), &ecnTemplate));
		if (ecnTemplate != NULLTAG)
		{
			int alCount = 0;
			string listcomb = "";
			logical processFlag = false;

			Teamcenter::scoped_smptr<char*> moduleGroup;
			Teamcenter::scoped_smptr<char*> vehicleGroup;
			Teamcenter::scoped_smptr<tag_t> tAssignmentList;
			tag_t group_tag_id = NULL_TAG;
			char* nameGroup = NULL;

			iRetCode = EPM_create_process_deferred_start(processName.c_str(), "", ecnTemplate, 1, &newChangeRev, attach_types, &process);
			if (iRetCode != ITK_ok)
			{
				Teamcenter::scoped_smptr<char> error_str;
				EMH_ask_error_text(iRetCode, &error_str);
				sprintf(ErrorMessage, "ERROR MSG:%s.Error in line %d function %s", error_str.getString(), __LINE__, __FUNCTION__);
				if (tc_strcmp(failToTriggerSubject.c_str(), "") == 0)
				{
					sprintf(pcMailSubject, "[ECN-auto-create] %s is fail to trigger ECN process", ecnId.c_str());
				}
				else {
					tc_strcpy(pcMailSubject, failToTriggerSubject.c_str());
				}
				mailBody << "<!DOCTYPE html><html><body>";
				mailBody << ecnId.c_str();
				mailBody << " created but fail to trigger ECN process.";
				mailBody << ErrorMessage;
				mailBody << "<p><strong>This is auto generated email. Please don't reply</strong></p>";
				mailBody << "</body></html>";
				//preparing mail list
				if (tc_strcmp(failToTriggerRecipients.c_str(), "") == 0)
				{
					szMailList.push_back(pcUserMail.c_str());
				}
				else
				{
					int  recipientsCount = 0;
					Teamcenter::scoped_smptr<char*> recipientsValues;
					CHECK_ITK(iRetCode, EPM__parse_string(failToTriggerRecipients.c_str(), ",", &recipientsCount, &recipientsValues));
					for (int iRecCount = 0; iRecCount < recipientsCount; iRecCount++)
					{
						szMailList.push_back(recipientsValues.getString()[iRecCount]);
					}
				}
				sendEmail(pcMailSubject, mailBody.str(), szMailList, false);
				return iRetCode;
			}
			CHECK_ITK(iRetCode, AOM_ask_value_tags(ecnTemplate, "assignment_lists", &alCount, &tAssignmentList));
			CHECK_ITK(iRetCode, EPM_ask_root_task(process, &rootTask));

			int tmpCount = 0;
			CHECK_ITK(iRetCode, AOM_ask_displayable_values(ptTgt, "vf6_module_group_comp", &tmpCount, &moduleGroup));
			CHECK_ITK(iRetCode, AOM_ask_value_tag(ptTgt, "owning_group", &group_tag_id));
			CHECK_ITK(iRetCode, SA_ask_group_full_name(group_tag_id, &nameGroup));
			const std::string owning_group_name(nameGroup);
			if (alCount > 0)
			{
				Teamcenter::scoped_smptr<char> selectedALFromECR;
				bool isGetSelectedALFromECRTrigger = (tc_strcasecmp("E-MOTOR", (const char*)moduleGroup.getString()) == 0);
				bool isGetSelectedmodelGroupECR = owning_group_name.find("VINES", 0) != -1;
				if (isGetSelectedALFromECRTrigger || isGetSelectedmodelGroupECR) {
					CHECK_ITK(iRetCode, AOM_ask_value_string(tNewChange, "vf4_module_group_vi", &selectedALFromECR));
				}
				tmpCount = 0;
				CHECK_ITK(iRetCode, AOM_ask_displayable_values(ptTgt, "vf6_vehicle_group", &tmpCount, &vehicleGroup));
				if (tmpCount > 0)
				{
					if (isGetSelectedALFromECRTrigger)
					{
						listcomb.assign(selectedALFromECR.get());
					}
					else
					{
						listcomb.assign(vehicleGroup.getString()[0]);
						listcomb.append(";;");
						listcomb.append(moduleGroup.getString()[0]);
					}

					for (int iALIny = 0; iALIny < alCount; iALIny++)
					{
						int descCount = 0;
						Teamcenter::scoped_smptr<char*> listDesc;

						CHECK_ITK(iRetCode, AOM_ask_value_strings(tAssignmentList[iALIny], "list_desc", &descCount, &listDesc));
						for (int iALDesIny = 0; iALDesIny < descCount; iALDesIny++)
						{
							if (tc_strstr(listDesc.getString()[iALDesIny], listcomb.c_str()))
							{
								CHECK_ITK(iRetCode, AOM_refresh(process, TRUE));
								CHECK_ITK(iRetCode, EPM_assign_assignment_list(process, tAssignmentList[iALIny]));
								CHECK_ITK(iRetCode, EPM_trigger_action(rootTask, EPM_complete_action, ""));
								CHECK_ITK(iRetCode, AOM_save_with_extensions(process));
								CHECK_ITK(iRetCode, AOM_refresh(process, FALSE));
								processFlag = true;
								break;
							}
						}

						if (processFlag) break;
					}
				}
				if (!processFlag)
				{
					if (tc_strcmp(failToTriggerSubject.c_str(), "") == 0)
					{
						sprintf(pcMailSubject, "[ECN-auto-create] %s is fail to trigger ECN process", ecnId.c_str());
					}
					else {
						tc_strcpy(pcMailSubject, failToTriggerSubject.c_str());
					}
					mailBody << "<!DOCTYPE html><html><body>";
					mailBody << ecnId.c_str();
					mailBody << " created but fail to trigger ECN process as no matching AL found";
					mailBody << "<p><strong>This is auto generated email. Please don't reply</strong></p>";
					mailBody << "</body></html>";
					//preparing mail list
					if (tc_strcmp(failToTriggerRecipients.c_str(), "") == 0)
					{
						szMailList.push_back(pcUserMail.c_str());
					}
					else
					{
						int  recipientsCount = 0;
						Teamcenter::scoped_smptr<char*> recipientsValues;
						CHECK_ITK(iRetCode, EPM__parse_string(failToTriggerRecipients.c_str(), ",", &recipientsCount, &recipientsValues));
						for (int iRecCount = 0; iRecCount < recipientsCount; iRecCount++)
						{
							szMailList.push_back(recipientsValues.getString()[iRecCount]);
						}
					}
					sendEmail(pcMailSubject, mailBody.str(), szMailList, false);

					//removing attached workflow

					int n_references = 0;
					int* levels = NULL;

					Teamcenter::scoped_smptr<tag_t> reference_tags;
					Teamcenter::scoped_smptr<char*> relation_type_name;

					CHECK_ITK(iRetCode, WSOM_where_referenced2(newChangeRev, 1, &n_references, &levels,
						&reference_tags, &relation_type_name));
					for (int ii = 0; ii < n_references; ii++)
					{
						Teamcenter::scoped_smptr<char> type_name;
						CHECK_ITK(iRetCode, WSOM_ask_object_type2(reference_tags[ii], &type_name));
						if (strcmp(type_name.getString(), "EPMTask") == 0)
						{
							tag_t task = reference_tags[ii];
							ITK_set_bypass(TRUE);
							CHECK_ITK(iRetCode, EPM_remove_attachments(task, 1, &newChangeRev));
						}
					}
				}
			}
			else
			{
				CHECK_ITK(iRetCode, EPM_trigger_action(rootTask, EPM_complete_action, ""));
			}
		}
		else {
			sprintf(ErrorMessage, "Given ECN workflow template does not present in DB: %s ", ecnWorkflow.c_str());
			CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
			return Error;
		}
		MEM_free(attach_types);
	}
	return iRetCode;
}
int getPart(std::string partNumber, std::string partType, tag_t* tPart)
{
	int iRetCode = ITK_ok;
	int iCount = 0;
	tag_t tQuery = NULLTAG;
	const char* queryName = "Item...";
	Teamcenter::scoped_smptr<tag_t> tQryResults;

	CHECK_ITK(iRetCode, QRY_find2(queryName, &tQuery));
	if (tQuery == NULLTAG)
	{
		TC_write_syslog("ERROR: Cannot find query %s:\n", queryName);
		return ITK_ok;
	}

	char* queryEntries[] = { "Item ID", "Type" };
	char** queryValues = (char**)MEM_alloc(sizeof(char*) * 2);

	queryValues[0] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(partNumber.c_str()) + 1));
	queryValues[1] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(partType.c_str()) + 1));

	queryValues[0] = tc_strcpy(queryValues[0], partNumber.c_str());
	queryValues[1] = tc_strcpy(queryValues[1], partType.c_str());

	CHECK_ITK(iRetCode, QRY_execute(tQuery, 2, queryEntries, queryValues, &iCount, &tQryResults));
	if (iCount == 0)
	{
		//TC_write_syslog("[VFCost] No Item found for given Part Number [%s] and Part Type [%s]\n", partNumber.c_str(), partType.c_str());
	}
	else if (iCount == 1)
	{
		*tPart = tQryResults[0];
	}
	else
	{
		TC_write_syslog("[VFCost] More than 1 Items found for given Part Number [%s] and Part Type [%s]\n", partNumber.c_str(), partType.c_str());
	}

	SAFE_SM_FREE(queryValues[0]);
	SAFE_SM_FREE(queryValues[1]);
	SAFE_SM_FREE(queryValues);

	return iRetCode;
}