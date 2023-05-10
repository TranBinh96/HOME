/***************************************************************************************
File         : RH_check_checkout_objects.cpp

Description  : To check whether if given object type is checked out. 
  
Input        : None
						
Output       : None

Author       : Vinfast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Feb,18 2019     1.0         Kantesh			Initial Creation

*****************************************************************************************/
#include "Vinfast_Custom.h"

EPM_decision_t RH_check_checkedout_objects(EPM_rule_message_t msg)
{
	int			iRetCode			= ITK_ok;
	int			iNumArg				= 0;
	tag_t		tRootTask			= NULLTAG;
	char		*pcFlag				= NULL;	
	char		*pcValue			= NULL;
	char		*pcFormObjType		= NULL;
	char		ErrorMessage [2048] = "";
	
	EPM_decision_t decision			= EPM_go;

	/*getting number of arguments*/
	iNumArg = TC_number_of_arguments(msg.arguments);

	if(iNumArg > 0)
	{
		for(int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue );
			/*for argument status*/
			if( tc_strcmp(pcFlag,"type") == 0 && pcValue != NULL)
			{
				pcFormObjType = (char*) MEM_alloc(( (int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy( pcFormObjType, pcValue);
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

		/*getting target objects*/
		CHECK_ITK(iRetCode,EPM_ask_attachments(tRootTask,EPM_target_attachment,&iTgt,&ptTgt));
		for(int iInd = 0; iInd < iTgt; iInd++)
		{
			char	*pcObjType	= NULL;

			CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptTgt[iInd],&pcObjType));
			/*if given object type is matched*/
			if(tc_strcmp(pcObjType,pcFormObjType) == NULL)
			{
				logical lIsCheckedOut = false;

				// check if object is checked out
				CHECK_ITK(iRetCode,RES_is_checked_out(ptTgt[iInd],&lIsCheckedOut ));
				/*if given object is checked out*/
				if( iRetCode == ITK_ok && lIsCheckedOut == true)
				{
					char	*pcName	= NULL;

					CHECK_ITK(iRetCode,AOM_ask_name(ptTgt[iInd],&pcName));
					decision = EPM_nogo;
					iRetCode = CHECKOUT_STATUS;
					sprintf( ErrorMessage, "\"%s\" is checked out. Please check in before completing the task.",pcName);	
					EMH_store_error_s1(EMH_severity_error, CHECKOUT_STATUS, ErrorMessage );
					break;
				}
			}
			MEM_free(pcObjType);
		}
	}

	return decision;
}