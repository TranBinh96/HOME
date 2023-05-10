/***************************************************************************************
File         : RH_check_revision_release_status.cpp

Description  : To check if 01 revision contains in Solution folder. 

Input        : None

Output       : None

Author       : Vinfast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Nov,12 2019     1.0         Kantesh			Initial Creation

******************************************************************************************/
#include "Vinfast_Custom.h"

EPM_decision_t RH_is_ECN_valid_to_trigger(EPM_rule_message_t msg)
{
	int			iRetCode				= ITK_ok;
	int			iNumArg					= 0;
	tag_t		tRootTask				= NULLTAG;
	char		*pcFlag					= NULL;	
	char		*pcValue				= NULL;
	char		*pcSolRel				= NULL;
	char		*revisionRule			= NULL;
	char		*type                   = NULL;
	char		ErrorMessage [2048]		= "";

	EPM_decision_t decision			= EPM_go;

	TC_write_syslog("\n[VF] Enter %s", __FUNCTION__);
	/*getting number of arguments*/
	iNumArg = TC_number_of_arguments(msg.arguments);

	if(iNumArg > 0)
	{
		for(int iArgs = 0; iArgs < iNumArg; iArgs++)
		{
			iRetCode = ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue );
			/*for argument template name*/
			if( tc_strcmp(pcFlag,"solution") == 0)
			{
				pcSolRel = (char*) MEM_alloc(( (int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy( pcSolRel, pcValue);
			}

			if( tc_strcmp(pcFlag,"revision_rule") == 0)
			{
				revisionRule = (char*) MEM_alloc(( (int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy( revisionRule, pcValue);
			}

			if (tc_strcmp(pcFlag, "type") == 0)
			{
				type = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(type, pcValue);
			}
		}

		if (pcSolRel == revisionRule || revisionRule == NULLTAG)
		{
			iRetCode = EPM_wrong_number_of_arguments;
			sprintf(ErrorMessage, "Handler argument solution and revision_rule are required!");
			EMH_store_error_s1(EMH_severity_user_error, -1, ErrorMessage);
		}

		if (!type) type = "Vf6_ECNRevision";

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
			char	*pcTgtType	= NULL;

			CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptTgt[iInd],&pcTgtType));
			/*if given object type is matched*/

			if(tc_strcmp(type,pcTgtType) == 0)
			{
				int		iSolItems	= 0;
				tag_t	*ptSol		= NULL;
				tag_t	tRelation	= NULLTAG;
				tag_t   tEcnRev         = ptTgt[iInd];
				char    *newPartStatus  = NULL;

				CHECK_ITK(iRetCode,AOM_ask_value_string(tEcnRev, "vf6_new_parts_status", &newPartStatus));
				if (tc_strlen(newPartStatus) == 0)
				{
					tag_t tRevRule = NULLTAG;
					char *szHowConfigured = NULL;
					CHECK_ITK(iRetCode,CFM_find(revisionRule, &tRevRule));
					if (tRevRule == NULLTAG)
					{
						TC_write_syslog("\n[VF] WARNING: CANNOT FIND REV RULE %s", __ADMIN_LATEST_VALID_STATUS);
						iRetCode = ITK_ok;
						SAFE_MEM_free(pcTgtType);
						SAFE_MEM_free(newPartStatus);
						SAFE_MEM_free(revisionRule);
						break;
					}

					CHECK_ITK(iRetCode,GRM_find_relation_type(pcSolRel,&tRelation));
					if(tRelation != NULLTAG)
					{
						CHECK_ITK(iRetCode,GRM_list_secondary_objects_only(ptTgt[iInd],tRelation,&iSolItems,&ptSol));
						for(int iCnt = 0; iCnt < iSolItems; iCnt++)
						{
							char	*pcItemRev	= NULL;
							char	*pcItem  	= NULL;
							tag_t tRev = ptSol[iCnt];
							tag_t tLatestValidReleasedRev = NULLTAG;
							tag_t tItem = NULLTAG;
							CHECK_ITK(iRetCode,ITEM_ask_item_of_rev(tRev,&tItem));
							CHECK_ITK(iRetCode,ITEM_ask_rev_id2(tRev,&pcItemRev));
							CHECK_ITK(iRetCode,ITEM_ask_id2(tItem,&pcItem));

							char *szHowConfigured = NULL;
							CHECK_ITK(iRetCode,CFM_item_ask_configured(tRevRule, tItem, &tLatestValidReleasedRev, &szHowConfigured));
							TC_write_syslog("\n[VF] %s", szHowConfigured);
							SAFE_MEM_free(szHowConfigured);

							if (tLatestValidReleasedRev == NULLTAG)
							{
								decision = EPM_nogo;
								sprintf( ErrorMessage, "Please fill \"New Part Status\" field before triggering the current process!");
								EMH_store_error_s1(EMH_severity_user_error, -1, ErrorMessage);
								SAFE_MEM_free(pcItemRev);
								SAFE_MEM_free(pcItem);
								break;
							}

							/*if(tc_strcmp("01",pcItemRev) == NULL || tc_strstr(pcItemRev,".") != NULL)
							{
							tag_t	tItem	= NULLTAG;
							char	*pcItem	= NULL;

							CHECK_ITK(iRetCode,ITEM_ask_item_of_rev(ptSol[iCnt],&tItem));
							CHECK_ITK(iRetCode,ITEM_ask_id2(tItem,&pcItem));

							decision = EPM_nogo;
							sprintf( ErrorMessage, "Item %s with Revision 01/ Baseline Revision is not allowed in Solution folder. Please remove %s/%s from Solution folder to proceed.",pcItem,pcItem,pcItemRev);
							EMH_store_error_s1(EMH_severity_user_error, CHECK_REV_STS, ErrorMessage);
							break;
							}*/
							SAFE_MEM_free(pcItemRev);
							SAFE_MEM_free(pcItem);
						}
					}
				}

				SAFE_MEM_free(newPartStatus);
			}
			SAFE_MEM_free(pcTgtType);
			SAFE_MEM_free(revisionRule);
			SAFE_SM_FREE(type);
		}
	}

	TC_write_syslog("\n[VF] Leave %s", __FUNCTION__);
	return decision;
}