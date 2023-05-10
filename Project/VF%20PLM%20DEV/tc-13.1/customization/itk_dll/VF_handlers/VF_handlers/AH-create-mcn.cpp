/*******************************************************************************
File         : AH_create_mcn.cpp

Description  : 	Auto Create MCN after Change Request approved

Input        : None

Output       : None

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
June,22 2022     1.0         Pallavi		 Initial Creation

/*******************************************************************************/
#include "Vinfast_Custom.h"
#include <map>
#include <iterator>
#include <base_utils/ScopedSmPtr.hxx>
#include <base_utils/ScopedPtr.hxx>
#include "Fnd0Participant/participant.h"
#include "property/idgeneration.h"

#define Error (EMH_USER_error_base +285)

int createMCN(tag_t mcnCreateInput, tag_t changeItem, tag_t changeItemRev, const Teamcenter::scoped_smptr<char>& relationsToCopy, const Teamcenter::scoped_smptr<char>& crImplementsBy, tag_t tRootTask)
{
	TC_write_syslog("[VF] ENTER %s \n", __FUNCTION__);
	int			iRetCode = ITK_ok;
	tag_t mcnItem = NULLTAG;
	tag_t mcnItemRev = NULLTAG;

	CHECK_ITK(iRetCode, TCTYPE_create_object(mcnCreateInput, &mcnItem));
	CHECK_ITK(iRetCode, AOM_save_with_extensions(mcnItem));
	CHECK_ITK(iRetCode, ITEM_ask_latest_rev(mcnItem, &mcnItemRev));

	//attaching new change object with CMImplements
	tag_t cmImplements = NULLTAG;
	tag_t tCMImplements = NULLTAG;

	if (tc_strcmp(crImplementsBy.getString(), "") != 0)
	{
		const char* rel = { crImplementsBy.getString() };
		CHECK_ITK(iRetCode, GRM_find_relation_type(rel, &tCMImplements));
		CHECK_ITK(iRetCode, GRM_create_relation(mcnItemRev, changeItemRev, tCMImplements, NULLTAG, &cmImplements));
		if (cmImplements != NULLTAG)
		{
			CHECK_ITK(iRetCode, GRM_save_relation(cmImplements));
			CHECK_ITK(iRetCode, AOM_refresh(cmImplements, FALSE));
			//if (iRetCode == ITK_ok) EMH_clear_errors();
		}
	}
	// copying relations from change object to mcn
	if (tc_strcmp(relationsToCopy.getString(), "") != 0)
	{
		int  relCount = 0;
		Teamcenter::scoped_smptr<char*> relValues;
		CHECK_ITK(iRetCode, EPM__parse_string(relationsToCopy.getString(), ",", &relCount, &relValues));
		for (int iRelCount = 0; iRelCount < relCount; iRelCount++)
		{
			int iCountSecObjs = 0;
			tag_t tRepresentedBy = NULLTAG;
			Teamcenter::scoped_smptr<tag_t> ptSecondaryObjs;
			if (tc_strcmp(relValues.getString()[iRelCount], "CMHasSolutionItem") == 0)
			{
				TC_write_syslog("[VF] Solutions is not a valid relation for Manufacturing Change Notice\n");
			}
			else
			{
				CHECK_ITK(iRetCode, GRM_find_relation_type(relValues[iRelCount], &tRepresentedBy));
				CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(changeItemRev, tRepresentedBy, &iCountSecObjs, &ptSecondaryObjs));
				for (int isecIny = 0; isecIny < iCountSecObjs; isecIny++)
				{
					tag_t tcreatedRelation = NULLTAG;
					CHECK_ITK(iRetCode, GRM_create_relation(mcnItemRev, ptSecondaryObjs[isecIny], tRepresentedBy, NULLTAG, &tcreatedRelation));
					CHECK_ITK(iRetCode, GRM_save_relation(tcreatedRelation));
				}
			}
		}
	}

	//transfer ownership of ecn
	tag_t tOwningUser = NULLTAG;
	tag_t tOwningGroup = NULLTAG;
	CHECK_ITK(iRetCode, AOM_ask_value_tag(changeItemRev, "owning_user", &tOwningUser));
	CHECK_ITK(iRetCode, AOM_ask_value_tag(changeItemRev, "owning_group", &tOwningGroup));

	CHECK_ITK(iRetCode, AOM_refresh(mcnItem, TRUE));
	CHECK_ITK(iRetCode, AOM_refresh(mcnItemRev, TRUE));
	CHECK_ITK(iRetCode, AOM_set_ownership(mcnItem, tOwningUser, tOwningGroup));
	CHECK_ITK(iRetCode, AOM_set_ownership(mcnItemRev, tOwningUser, tOwningGroup));
	CHECK_ITK(iRetCode, AOM_save_without_extensions(mcnItem));
	CHECK_ITK(iRetCode, AOM_save_without_extensions(mcnItemRev));
	CHECK_ITK(iRetCode, AOM_refresh(mcnItem, FALSE));
	CHECK_ITK(iRetCode, AOM_refresh(mcnItemRev, FALSE));

	int attachmentTypes[1] = { EPM_target_attachment };
	CHECK_ITK(iRetCode, EPM_add_attachments(tRootTask, 1, &mcnItemRev, attachmentTypes));

	TC_write_syslog("[VF] LEAVE %s \n", __FUNCTION__);
	return iRetCode;
}

tag_t createMCNCreateInput(tag_t changeItem, tag_t changeItemRev, const char* mcnObjType, char* shop, char* program, const Teamcenter::scoped_smptr<char*>& attrValues, const Teamcenter::scoped_smptr<char*>& attrPasteValues, int attrCount)
{
	TC_write_syslog("[VF] ENTER %s \n", __FUNCTION__);

	tag_t createMCNInput = NULLTAG;
	int iRetCode = ITK_ok;
	tag_t mcnType = NULLTAG;
	tag_t mcnRevType_tag = NULLTAG;
	char ErrorMessage[2048];

	CHECK_ITK(iRetCode, TCTYPE_find_type(mcnObjType, NULLTAG, &mcnType));
	string mcnRevType(mcnObjType);
	mcnRevType.append("Revision");


	if (tc_strcmp(mcnObjType, "Vf6_MCN") == 0)
	{
		tag_t idGenTypeTag = NULLTAG;
		CHECK_ITK(iRetCode, TCTYPE_ask_type("Vf6_ecr_id", &idGenTypeTag));

		int pCount = 0;
		//Teamcenter::scoped_smptr<char*> vehicleTypes;
		//CHECK_ITK(iRetCode, AOM_ask_value_strings(changeItem, "vf4_vehicle_type_arr", &pCount, &vehicleTypes));
		for (int iPropCount = 0; iPropCount < pCount; iPropCount++)
		{
			tag_t item_create_input_tag = NULLTAG;
			tag_t rev_create_input_tag = NULLTAG;
			tag_t idGenCreateInputTag = NULLTAG;

			CHECK_ITK(iRetCode, TCTYPE_construct_create_input(mcnType, &createMCNInput));
			CHECK_ITK(iRetCode, TCTYPE_construct_create_input(idGenTypeTag, &idGenCreateInputTag));
			CHECK_ITK(iRetCode, TCTYPE_find_type(mcnRevType.c_str(), NULLTAG, &mcnRevType_tag));
			CHECK_ITK(iRetCode, TCTYPE_construct_create_input(mcnRevType_tag, &rev_create_input_tag));

			// set the values on CreI of idgen BO
			CHECK_ITK(iRetCode, AOM_set_value_string(idGenCreateInputTag, "vf6_vehicle_type", program));

			//Set the value for fnd0IdGenerator prop value 
			CHECK_ITK(iRetCode, AOM_set_value_tag(createMCNInput, "fnd0IdGenerator", idGenCreateInputTag));

			// copying attributes from change request to mcn
			for (int iAtrCount = 0; iAtrCount < attrCount; iAtrCount++)
			{
				Teamcenter::scoped_smptr<char> propValue;
				if (tc_strstr(attrValues[iAtrCount], ":") != NULL)
				{
					int  tokenCount = 0;
					tag_t tItem = NULLTAG;
					tag_t classId = NULLTAG;
					logical itemRevisionExists = false;

					Teamcenter::scoped_smptr<char*> tokenValue;
					Teamcenter::scoped_smptr<char> className;

					CHECK_ITK(iRetCode, POM_class_of_instance(changeItemRev, &classId));
					CHECK_ITK(iRetCode, POM_name_of_class(classId, &className));
					CHECK_ITK(iRetCode, EPM__parse_string(attrValues[iAtrCount], ":", &tokenCount, &tokenValue));
					CHECK_ITK(iRetCode, POM_attr_exists(tokenValue[1], className.getString(), &itemRevisionExists));
					if (itemRevisionExists)
					{
						CHECK_ITK(iRetCode, AOM_ask_value_string(changeItemRev, tokenValue[1], &propValue));
						if (propValue != NULL)
						{
							if (tc_strstr(attrPasteValues[iAtrCount], ":") != NULL)
							{
								int  tokenCount1 = 0;
								Teamcenter::scoped_smptr<char*> tokenValue1;
								CHECK_ITK(iRetCode, EPM__parse_string(attrPasteValues[iAtrCount], ":", &tokenCount1, &tokenValue1));

								const char* tmp = (propValue.getString());

								CHECK_ITK(iRetCode, TCTYPE_set_create_display_value(rev_create_input_tag, tokenValue1[1], 1, &tmp));
							}
							else {
								const char* tmp = (propValue.getString());
								CHECK_ITK(iRetCode, TCTYPE_set_create_display_value(createMCNInput, attrPasteValues[iAtrCount], 1, &tmp));
							}
						}
					}
					else
					{
						sprintf_s(ErrorMessage, 2048, "Cannot find attribute: %s in the target object", attrValues[iAtrCount]);
						CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
						return Error;
					}
				}
				else
				{
					if (tc_strcmp(attrValues.getString()[iAtrCount], "vf6_action") == 0)
					{
						const char* action = { "A" };
						CHECK_ITK(iRetCode, TCTYPE_set_create_display_value(createMCNInput, "vf6_action", 1, &action));
					}
					else
					{
						tag_t classId = NULLTAG;
						Teamcenter::scoped_smptr<char> className;
						logical itemExists = false;

						CHECK_ITK(iRetCode, POM_class_of_instance(changeItem, &classId));
						CHECK_ITK(iRetCode, POM_name_of_class(classId, &className));
						CHECK_ITK(iRetCode, POM_attr_exists(attrValues[iAtrCount], className.getString(), &itemExists));
						if (itemExists)
						{
							CHECK_ITK(iRetCode, AOM_ask_value_string(changeItem, attrValues[iAtrCount], &propValue));
							if (propValue != NULL)
							{
								const char* tmp = (propValue.getString());
								CHECK_ITK(iRetCode, TCTYPE_set_create_display_value(createMCNInput, attrPasteValues[iAtrCount], 1, &tmp));
							}
						}
						else
						{
							sprintf_s(ErrorMessage, 2048, "Cannot find attribute: %s in the class %s", attrValues[iAtrCount], className.getString());
							CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
							return Error;
						}
					}
				}


				//set shop to MCN vf6_sap_plant
				CHECK_ITK(iRetCode, TCTYPE_set_create_display_value(createMCNInput, "vf6_sap_plant", 1, (const char**)&shop));



			}

			if (rev_create_input_tag != NULLTAG) {
				CHECK_ITK(iRetCode, AOM_set_value_tag(createMCNInput, "revision", rev_create_input_tag));
			}



		}

		TC_write_syslog("[VF] LEAVE %s \n", __FUNCTION__);
		return createMCNInput;
	}
}

extern int AH_create_mcn(EPM_action_message_t msg)
{
	TC_write_syslog("[VF] ENTER %s \n", __FUNCTION__);

	int			iRetCode = ITK_ok;
	int			iNumArgs = 0;
	int			iPrefCount = 0;
	int         valCount = 0;

	char		ErrorMessage[2048] = "";
	const char* mcnObjType = NULLTAG;

	tag_t		tRootTask = NULLTAG;
	Teamcenter::scoped_smptr<char> crImplementsBy(NULL);
	Teamcenter::scoped_smptr<char> mcnItemType(NULL);
	Teamcenter::scoped_smptr<char> changeItemType(NULL);
	Teamcenter::scoped_smptr<char> attrsToCopy(NULL);
	Teamcenter::scoped_smptr<char> attrsToPaste(NULL);
	Teamcenter::scoped_smptr<char> relationsToCopy(NULL);
	Teamcenter::scoped_smptr<char> shopsAttrToCopy(NULL);
	Teamcenter::scoped_smptr<char> programsAttrToCopy(NULL);



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
				if (tc_strcmp(pcFlag.getString(), "ChangeRequest-MCN-relation") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &crImplementsBy));
				}
				else if (tc_strcmp(pcFlag.getString(), "Change-item-type") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &changeItemType));
				}
				else if (tc_strcmp(pcFlag.getString(), "MCN-item-type") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &mcnItemType));
				}
				else if (tc_strcmp(pcFlag.getString(), "Shops-to-copy") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &shopsAttrToCopy));
				}
				else if (tc_strcmp(pcFlag.getString(), "Programs-to-copy") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &programsAttrToCopy));
				}
				else if (tc_strcmp(pcFlag.getString(), "Attributes-to-copy") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &attrsToCopy));
				}
				else if (tc_strcmp(pcFlag.getString(), "Attributes-to-paste") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &attrsToPaste));
				}
				else if (tc_strcmp(pcFlag.getString(), "Relations-to-copy") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &relationsToCopy));
				}
			}
		}
	}
	else
	{
		iRetCode = EPM_invalid_argument;
	}

	if (tc_strcmp(crImplementsBy.getString(), "") == 0)
	{
		sprintf_s(ErrorMessage, 2048, "Mandatory Input ChangeRequest-MCN-relation not provided");
		CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
		return Error;
	}
	else if (tc_strcmp(changeItemType.getString(), "") == 0)
	{
		sprintf_s(ErrorMessage, 2048, "Mandatory Input Change Item Type  not provided");
		CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
		return Error;
	}

	if (tc_strcmp(mcnItemType.getString(), "") == 0)
	{
		mcnObjType = ("Vf6_MCN");
	}
	else {
		mcnObjType = (mcnItemType.getString());
	}

	int  attrCount = 0;
	int  attrPasteCount = 0;
	Teamcenter::scoped_smptr<char*> attrValues;
	Teamcenter::scoped_smptr<char*> attrPasteValues;

	CHECK_ITK(iRetCode, EPM__parse_string(attrsToCopy.getString(), ",", &attrCount, &attrValues));
	CHECK_ITK(iRetCode, EPM__parse_string(attrsToPaste.getString(), ",", &attrPasteCount, &attrPasteValues));
	if (attrCount != attrPasteCount)
	{
		sprintf_s(ErrorMessage, 2048, "Size of attributes to copy and attributes to paste does not match");
		CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
		return Error;
	}
	/*getting root task*/
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int	iTgt = 0;
		tag_t mcnItem = NULLTAG;
		tag_t mcnItemRev = NULLTAG;
		tag_t pChangeRevObj = NULLTAG;
		Teamcenter::scoped_smptr<tag_t> ptTgt;

		tag_t tChangeRevType = NULLTAG;
		CHECK_ITK(iRetCode, TCTYPE_ask_type("ChangeItemRevision", &tChangeRevType));

		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		TC_write_syslog("EPM_target_attachment count %d\n", iTgt);
		//Looping the targets
		for (int iLoopTarget = 0; iLoopTarget < iTgt; iLoopTarget++)
		{
			tag_t mcnType = NULLTAG;
			tag_t changeItem = NULLTAG;
			tag_t mcnRevType_tag = NULLTAG;

			bool isChangeRevType = false;

			Teamcenter::scoped_smptr<char> objType;

			pChangeRevObj = ptTgt[iLoopTarget];
			CHECK_ITK(iRetCode, AOM_is_type_of(pChangeRevObj, tChangeRevType, &isChangeRevType));

			if (isChangeRevType) {
				CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(pChangeRevObj, &changeItem));
			}
			else {
				continue;
			}

			CHECK_ITK(iRetCode, AOM_ask_value_string(changeItem, "object_type", &objType));
			if (tc_strcmp(objType.getString(), changeItemType.getString()) == 0)
			{
				Teamcenter::scoped_smptr<char> changeRevType;

				CHECK_ITK(iRetCode, AOM_ask_value_string(pChangeRevObj, "object_type", &changeRevType));
				CHECK_ITK(iRetCode, TCTYPE_find_type(mcnObjType, NULLTAG, &mcnType));

				// assume shopsAttrToCopy & programsAttrToCopy is are valid attributes of the change object
				int shopsCount = 0;
				Teamcenter::scoped_smptr<char*> shopsToCopy;
				int programsCount = 0;
				Teamcenter::scoped_smptr<char*> programsToCopy;
				vector<tag_t>  mcnCreateInputs;

				tag_t sourceObjToGetShops = NULLTAG;
				tag_t sourceObjToGetPrograms = NULLTAG;
				string shopsAttrStr("");
				string programsAttrStr("");

				Teamcenter::scoped_smptr<char*> tokenValues;
				int tokenCount = 0;
				if (tc_strstr(shopsAttrToCopy.getString(), ":") != NULL) CHECK_ITK(iRetCode, EPM__parse_string(shopsAttrToCopy.getString(), ":", &tokenCount, &tokenValues));
				if (tokenCount == 2)
				{
					sourceObjToGetShops = pChangeRevObj;
					shopsAttrStr.append(tokenValues[1]);
				}
				else
				{
					sourceObjToGetShops = changeItem;
					shopsAttrStr.append(shopsAttrToCopy.getString());
				}

				Teamcenter::scoped_smptr<char*> tokenValues2;
				tokenCount = 0;
				if (tc_strstr(programsAttrToCopy.getString(), ":") != NULL) CHECK_ITK(iRetCode, EPM__parse_string(programsAttrToCopy.getString(), ":", &tokenCount, &tokenValues2));
				if (tokenCount == 2)
				{
					sourceObjToGetPrograms = pChangeRevObj;
					programsAttrStr.append(tokenValues2[1]);
				}
				else
				{
					sourceObjToGetPrograms = changeItem;
					programsAttrStr.append(programsAttrToCopy.getString());
				}



				CHECK_ITK(iRetCode, AOM_ask_value_strings(sourceObjToGetShops, shopsAttrStr.c_str(), &shopsCount, &shopsToCopy));
				CHECK_ITK(iRetCode, AOM_ask_value_strings(sourceObjToGetPrograms, programsAttrStr.c_str(), &programsCount, &programsToCopy));

				int numMCNToCreate = 0;

				for (int shopInx = 0; shopInx < shopsCount; shopInx++)
				{
					for (int programInx = 0; programInx < programsCount; programInx++)
					{
						char* shop = shopsToCopy[shopInx];
						char* program = programsToCopy[programInx];
						tag_t mcnCreateInput = createMCNCreateInput(changeItem, pChangeRevObj, mcnObjType, shop, program, attrValues, attrPasteValues, attrCount);
						if (mcnCreateInput != NULLTAG) mcnCreateInputs.push_back(mcnCreateInput);
						numMCNToCreate++;

					}
				}

				int numMCNCreated = 0;
				for (int mcnInx = 0; mcnInx < mcnCreateInputs.size(); mcnInx++)
				{
					int retcode = createMCN(mcnCreateInputs[mcnInx], changeItem, pChangeRevObj, relationsToCopy, crImplementsBy, tRootTask);
					if (retcode == ITK_ok) numMCNCreated++;
				}



				if (numMCNCreated < numMCNToCreate)
				{
					// save error => IT need to ensure there are enough MCN created
				}



			}
		}
	}
	TC_write_syslog("[VF] EXIT %s \n", __FUNCTION__);
	return iRetCode;
}