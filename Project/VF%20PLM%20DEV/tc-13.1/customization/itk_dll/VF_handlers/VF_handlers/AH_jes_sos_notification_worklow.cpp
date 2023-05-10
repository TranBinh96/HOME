#include "Vinfast_Custom.h"

extern int AH_jes_sos_notification_worklow(EPM_action_message_t msg) {

	int	iRetCode = ITK_ok;
	tag_t tRootTask = NULLTAG;
	int	  iNumArgs = 0;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* pcTemplate = NULL;
	char* mcnObjType = ("Vf6_MCNRevision");

	iNumArgs = TC_number_of_arguments(msg.arguments);
	TC_write_syslog("TC_number_of_arguments %d\n", iNumArgs);
	if (iNumArgs > 0)
	{
		for (int i = 0; i < iNumArgs; i++)
		{
			/*getting arguments*/
			CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue));
			/*for argument template name*/
			if (tc_strcmp(pcFlag, "template") == 0)
			{
				pcTemplate = (char*)MEM_alloc(((int)tc_strlen(pcValue) + 1) * sizeof(char));
				tc_strcpy(pcTemplate, pcValue);
			}
		}
		MEM_free(pcValue);
		MEM_free(pcFlag);
	}
	else
	{
		iRetCode = EPM_invalid_argument;
	}
	/*getting root task*/
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int  iTgt = 0;
		tag_t* ptTgt = NULLTAG;
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		//Looping the targets
		for (int iLoopTarget = 0; iLoopTarget < iTgt; iLoopTarget++)
		{
			int* attach_types = NULL;
			tag_t rev_id = NULLTAG;
			tag_t process = NULLTAG;
			string processName = "";
			char* current = NULL;
			char* shop = NULL;
			char* revision_list = NULL;
			tag_t rootTask = NULLTAG;
			char* objName = NULL;
			tag_t temp_tag = NULLTAG;
			int alCount = 0;
			logical processFlag = false;
			std::vector<string> szMailList;
			attach_types = (int*)MEM_alloc(1 * sizeof(int));
			attach_types[0] = EPM_target_attachment;
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iLoopTarget], "current_id", &current));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iLoopTarget], "object_string", &objName));
			CHECK_ITK(iRetCode, ITEM_find_item(current, &rev_id));
			CHECK_ITK(iRetCode, AOM_ask_value_string(rev_id, "vf6_shop", &shop));
			processName.assign(objName);
			processName.append(":");
			processName.append(pcTemplate);			
			CHECK_ITK(iRetCode, EPM_find_process_template(pcTemplate, &temp_tag));
			tag_t* tAssignmentList = NULLTAG;
			CHECK_ITK(iRetCode, AOM_ask_value_tags(temp_tag, "assignment_lists", &alCount, &tAssignmentList));
			if (alCount > 0) {
				for (int iALIny = 0; iALIny < alCount; iALIny++)
				{
					tag_t assignment = tAssignmentList[iALIny];
					int descCount = 0;
					char* listDesc = NULL;
					CHECK_ITK(iRetCode, AOM_ask_value_string(assignment, "list_name", &listDesc));
					int status = string(listDesc).find(shop, 0);
					if (status != -1) {
						iRetCode = EPM_create_process_deferred_start(processName.c_str(), "", temp_tag, 1, &ptTgt[iLoopTarget], attach_types, &process);
						if (iRetCode == ITK_ok) {
							CHECK_ITK(iRetCode, EPM_ask_root_task(process, &rootTask));
							CHECK_ITK(iRetCode, AOM_refresh(process, TRUE));
							CHECK_ITK(iRetCode, EPM_assign_assignment_list(process, assignment));
							CHECK_ITK(iRetCode, EPM_trigger_action(rootTask, EPM_complete_action, ""));
							CHECK_ITK(iRetCode, AOM_save_with_extensions(process));
							CHECK_ITK(iRetCode, AOM_refresh(process, FALSE));
							processFlag = true;
							break;
						}
					}
				}if (processFlag) break;				
			}
			else
			{
				CHECK_ITK(iRetCode, EPM_trigger_action(rootTask, EPM_complete_action, ""));
			}
			MEM_free(attach_types);
		}
	}
	return iRetCode;
}