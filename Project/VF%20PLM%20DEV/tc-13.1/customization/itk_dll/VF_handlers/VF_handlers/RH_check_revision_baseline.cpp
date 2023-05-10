#include "Vinfast_Custom.h"

using namespace std;

/***************************************************************************************
Description  : To check whether any Item Revision in BOM which is not part of Solution 
			   Items is not having valid release status. 
  
Input        :
				inBaselineFlags: list of flags separated by comma.
				inObjectTypes: list of object type separated by comma.						

Output       : 
				EPM_go: when baslines of all target parts (belongs to one of type of inObjectTypes) has at least one flag in inBaselineFlags
				EPM_nogo: vice versa

Author       : Vinfast
*****************************************************************************************/
EPM_decision_t RH_check_revision_baseline(EPM_rule_message_t msg)
{
	int			iRetCode			= ITK_ok;
	int			iNumArg				= 0;
	tag_t		tRootTask			= NULLTAG;
	string		inBaselineFlags		;	
	string		inObjectTypes		;
	char		*pcFlag				= NULL;	
	char		*pcValue			= NULL;	
	char		errorMessage [2048] = "";

	TC_write_syslog("\n[VF] Enter %s", __FUNCTION__);
	EPM_decision_t decision = EPM_go;
	iNumArg = TC_number_of_arguments(msg.arguments);

	if(iNumArg > 0)
	{
		for(int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue );
			
			if( tc_strcmp(pcFlag,"flags") == 0)
			{
				inBaselineFlags.assign(pcValue);
			}
			if( tc_strcmp(pcFlag,"check_object_types") == 0)
			{
				inObjectTypes.assign(pcValue);
			}
		}
		MEM_free(pcValue);
		MEM_free(pcFlag);
	}
	else
	{
		iRetCode = EPM_wrong_number_of_arguments;  
	}

	tag_t rootTask = NULLTAG;
	tag_t *targets = NULL;
	int targetsCount = 0;

	TC_write_syslog("\n[VF] inBaselineFlags=%s", inBaselineFlags.c_str());
	TC_write_syslog("\n[VF] inObjectTypes=%s", inObjectTypes.c_str());

	CHECK_ITK(iRetCode,EPM_ask_root_task(msg.task, &rootTask));
	CHECK_ITK(iRetCode,EPM_ask_attachments(rootTask, EPM_target_attachment, &targetsCount, &targets));
	for (int i = 0; i < targetsCount && decision == EPM_go; i++)
	{
		tag_t target = targets[i];
		char *targetObjectString = NULL;
		char *targetType = NULL;
		CHECK_ITK(iRetCode,WSOM_ask_object_type2(target, &targetType));
		CHECK_ITK(iRetCode,AOM_ask_value_string(target, "object_string", &targetObjectString));

		if (inObjectTypes.find(targetType) != string::npos)
		{
			int targetAllRevsCount = 0;
			tag_t *targetAllRevs = NULL;
			tag_t targetItem = NULLTAG;

			CHECK_ITK(iRetCode,AOM_ask_value_tag(target, "items_tag", &targetItem));
			CHECK_ITK(iRetCode,AOM_ask_value_tags(targetItem, "revision_list", &targetAllRevsCount, &targetAllRevs));

			bool baselineFlagsExisted = false;

			TC_write_syslog("\n[VF] targetAllRevsCount=%d", targetAllRevsCount);
			for (int j = 0; j < targetAllRevsCount && !baselineFlagsExisted; j++)
			{
				tag_t baseline = targetAllRevs[j];
				int statusesCount = 0;
				tag_t *statuses = NULL;
				TC_write_syslog("\n[VF] targetAllRevsCount[%d]", j);

				CHECK_ITK(iRetCode,WSOM_ask_release_status_list(baseline, &statusesCount, &statuses));
			
				for (int k = 0; k < statusesCount; k++)
				{
					char *statusType = NULL;
					CHECK_ITK(iRetCode,RELSTAT_ask_release_status_type(statuses[k], &statusType));
					TC_write_syslog("\n[VF] statusType=%ds", statusType);
					if (inBaselineFlags.find(statusType) != string::npos)
					{
						baselineFlagsExisted = true;
						break;
					}
					SAFE_SM_FREE(statusType);
				}

				SAFE_SM_FREE(statuses);
				TC_write_syslog("\n");
			}
			
			decision = baselineFlagsExisted ? EPM_go : EPM_nogo;
			if (decision == EPM_nogo)
			{
				sprintf( errorMessage, "%s does not have valid baselines flag \"%s\"!", targetObjectString, inBaselineFlags.c_str());
				EMH_store_error_s1(EMH_severity_user_error, CHECK_STATUS, errorMessage);
			}

			SAFE_SM_FREE(targetAllRevs);
			SAFE_SM_FREE(targetObjectString);
		}

		SAFE_SM_FREE(targetType);
	}

	SAFE_SM_FREE(targets);

	TC_write_syslog("\n[VF] Leave %s", __FUNCTION__);
	return decision;
}