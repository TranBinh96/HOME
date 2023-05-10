#include "Vinfast_Custom.h"

EPM_decision_t RH_check_dde_status(EPM_rule_message_t msg)
{
	int	iRetCode = ITK_ok;
	int	iNumArg = 0;
	tag_t tRootTask = NULLTAG;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* statusList = NULL;
	char* relationName = NULL;

	EPM_decision_t decision = EPM_go;

	iNumArg = TC_number_of_arguments(msg.arguments);
	if (iNumArg > 0)
	{
		for (int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue);
			if (tc_strcmp(pcFlag, "status") == 0)
			{
				statusList = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(statusList, pcValue);
			}
			if (tc_strcmp(pcFlag, "relation") == 0)
			{
				relationName = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(relationName, pcValue);
			}
		}
		MEM_free(pcValue);
		MEM_free(pcFlag);
	}
	else
	{
		iRetCode = EPM_wrong_number_of_arguments;
	}

	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		const char* ecnObjType = ("Vf6_ECNRevision");
		int	objectNumber = 0;
		tag_t* objectItem = NULL;

		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &objectNumber, &objectItem));
		for (int i = 0; i < objectNumber; i++)
		{
			Teamcenter::scoped_smptr<char> ecnRevType;
			CHECK_ITK(iRetCode, AOM_ask_value_string(objectItem[i], "object_type", &ecnRevType));
			if (tc_strcmp(ecnRevType.getString(), ecnObjType) == 0) {
				tag_t relationType = NULLTAG;
				CHECK_ITK(iRetCode, GRM_find_relation_type(relationName, &relationType));
				if (relationType != NULL)
				{
					const char* ddeObjType = ("VF4_SCDDE");
					int	relationNumber = 0;
					tag_t* relationItem = NULL;
					CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(objectItem[i], relationType, &relationNumber, &relationItem));
					for (int j = 0; j < relationNumber; j++)
					{
						Teamcenter::scoped_smptr<char> ddeRevType;
						CHECK_ITK(iRetCode, AOM_ask_value_string(relationItem[i], "object_type", &ddeRevType));
						if (tc_strcmp(ddeRevType.getString(), ddeObjType) == 0) {
							Teamcenter::scoped_smptr<char> ddeStatus;
							CHECK_ITK(iRetCode, AOM_ask_value_string(relationItem[j], "vf4_status", &ddeStatus));

							bool check = false;
							std::vector<std::string> statusObjects = split_string(statusList, ',');
							for (int k = 0; k < statusObjects.size(); k++)
							{
								std::string objectType = statusObjects[k];
								ltrim(objectType);
								rtrim(objectType);
								if (tc_strcmp(objectType.c_str(), ddeStatus.getString()) == 0)
									check = true;
							}
							if (!check) {
								decision = EPM_nogo;
								char ErrorMessage[2048] = "";
								sprintf_s(ErrorMessage, "DDE not yet finished");
								EMH_store_error_s1(EMH_severity_user_error, CHECK_REV_STS, ErrorMessage);
								break;
							}
						}
					}
					SAFE_MEM_free(relationItem);
				}
			}
		}
		SAFE_MEM_free(objectItem);
	}
	return decision;
}