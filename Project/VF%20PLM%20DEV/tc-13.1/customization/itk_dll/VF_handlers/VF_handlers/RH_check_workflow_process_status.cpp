/***************************************************************************************
File         : RH_check_workflow_process_status.cpp

Description  : To check if workflow is already applied on mentioned object. 
  
Input        : None
						
Output       : None

Author       : Vinfast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Jun,13 2019     1.0         Kantesh			Initial Creation
Jun,17 2020		1.1			Duc				Added use case "Aborted" process	
Jun,25 2020     1.2         Nguyen          Added freeing memory
******************************************************************************************/
#include "Vinfast_Custom.h"

EPM_decision_t RH_check_workflow_process_status(EPM_rule_message_t msg)
{
	int			iRetCode				= ITK_ok;
	int			iNumArg					= 0;
	tag_t		tRootTask				= NULLTAG;
	char		*pcFlag					= NULL;	
	char		*pcValue				= NULL;
	char		*pcTemplate				= NULL;
	char		*pcObjType				= NULL;
	char		ErrorMessage [2048]		= "";
	
	EPM_decision_t decision			= EPM_go;

	/*getting number of arguments*/
	iNumArg = TC_number_of_arguments(msg.arguments);

	if(iNumArg > 0)
	{
		for(int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue );
			/*for argument template name*/
			if( tc_strcmp(pcFlag,"template") == 0)
			{
				pcTemplate = (char*) MEM_alloc(( (int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy( pcTemplate, pcValue);
			}
			if( tc_strcmp(pcFlag,"type") == 0)
			{
				pcObjType = (char*) MEM_alloc(( (int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy( pcObjType, pcValue);
			}
		}
		MEM_free(pcValue);
		MEM_free(pcFlag);
	}
	else
	{
		iRetCode = EPM_wrong_number_of_arguments;  
	}

	/*getting root task*/
	CHECK_ITK(iRetCode,EPM_ask_root_task(msg.task,&tRootTask));
	if(tRootTask != 0)
	{
		int			iTgt				= 0;
		tag_t		*ptTgt				= NULL;
		char        *pcObjTypeList      = NULL;
		char		*pcReq				= NULL;
		vector<string> ReqObjType;

		pcObjTypeList = (char*) MEM_alloc(( (int)tc_strlen(pcObjType) + 1) * sizeof(char));
		tc_strcpy(pcObjTypeList,pcObjType);

		pcReq = tc_strtok (pcObjTypeList,",");
		while (pcReq != NULL)
		{
			ReqObjType.push_back(pcReq);
			pcReq = tc_strtok(NULL,",");
		}

		/*getting target objects*/
		CHECK_ITK(iRetCode,EPM_ask_attachments(tRootTask,EPM_target_attachment,&iTgt,&ptTgt));
		for(int iInd = 0; iInd < iTgt; iInd++)
		{
			char	*pcTgtType	= NULL;

			CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptTgt[iInd],&pcTgtType));
			/*if given object type is matched*/
			for(int iCnt = 0; iCnt < ReqObjType.size(); iCnt++)
			{
				if(tc_strcmp(ReqObjType[iCnt].c_str(),pcTgtType) == NULL)
				{
					int			iProcess		= 0;
					tag_t		*ptWF			= NULL;

					/*getting number of workflow process*/
					CHECK_ITK(iRetCode,AOM_ask_value_tags(ptTgt[iInd],"fnd0AllWorkflows", &iProcess, &ptWF));
					
					if(iProcess > 1)
					{
						for(int iIter1 = 0; iIter1 < iProcess; iIter1++)
						{
							char		*pcProcessName			= NULL;
							char		*pcStatus				= NULL;
							/*check for process name matching with argument*/
							CHECK_ITK(iRetCode, AOM_ask_value_string(ptWF[iIter1],"current_name",&pcProcessName));
							CHECK_ITK(iRetCode, AOM_ask_value_string(ptWF[iIter1], "fnd0Status", &pcStatus));
							if(tc_strcmp(pcProcessName, pcTemplate) == 0 && ptWF[iIter1] != tRootTask && tc_strcmp(pcStatus, "Started") == 0)
							{
								decision = EPM_nogo;
								sprintf( ErrorMessage, "You are not allowed to apply \"%s\" on the selected object as it is already in process.",pcTemplate);
								EMH_store_error_s1(EMH_severity_user_error, CHECK_WORKFLOW, ErrorMessage);
								SAFE_SM_FREE(pcStatus);
								break;
							}

							SAFE_SM_FREE(pcStatus);
						}
					}
				}
			}
		}
	}

	SAFE_SM_FREE(pcTemplate);
	SAFE_SM_FREE(pcObjType);
	return decision;
}