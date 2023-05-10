#include "Vinfast_Custom.h"

using namespace std;

/***************************************************************************************
Description  : To check whether any Item Revision in BOM which is not part of Solution 
			   Items is not having valid release status. 
  
Input        :
				inObjectTypes: list of object type separated by comma.						

Output       : 
				EPM_go: when all objects in targets which belongs to inObjetTypes has secondary object of baseline relation
				EPM_nogo: vice versa

Author       : Vinfast
*****************************************************************************************/
EPM_decision_t RH_is_include_baseline_revisions(EPM_rule_message_t msg)
{
	int			iRetCode			= ITK_ok;
	int			iNumArg				= 0;
	tag_t		tRootTask			= NULLTAG;
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
			int baselineTargetRevsCount = 0;
			tag_t *baselineTargetRevs = NULL;
			
			tag_t baselineType = NULLTAG;
			CHECK_ITK(iRetCode,TCTYPE_find_type("IMAN_baseline", "IMAN_baseline", &baselineType));
			CHECK_ITK(iRetCode,GRM_list_secondary_objects_only(target, baselineType, &baselineTargetRevsCount, &baselineTargetRevs));
			bool isBaselineRevs = baselineTargetRevsCount > 0;

			TC_write_syslog("\n[VF] baselinesCount=%d", baselineTargetRevsCount);
			
			
			decision = isBaselineRevs ? EPM_go : EPM_nogo;
			if (decision == EPM_nogo)
			{
				sprintf( errorMessage, "%s is not a valid baseline!", targetObjectString);
				EMH_store_error_s1(EMH_severity_user_error, CHECK_STATUS, errorMessage);
			}

			SAFE_SM_FREE(targetObjectString);
			SAFE_SM_FREE(baselineTargetRevs);
		}

		SAFE_SM_FREE(targetType);
	}
	SAFE_SM_FREE(targets);


	TC_write_syslog("\n[VF] Leave %s", __FUNCTION__);
	return decision;
}