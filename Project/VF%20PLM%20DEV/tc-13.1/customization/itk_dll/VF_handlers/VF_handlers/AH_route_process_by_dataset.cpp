/***************************************************************************************
Function     : AH_route_process_by_dataset()
Description  : Based rule descriped in -routing argument, check the real value in process-input-dataset and find an outway for next task.
               This handler should be placed in a condition task.
Input        : -routing=attr1=value1^outway1|attr2=value2^outway1|attr1=value2^outway2
Output       : None
Author       : nguyenttk2
******************************************************************************************/
#include "Vinfast_Custom.h"

extern int AH_route_process_by_dataset(EPM_action_message_t msg)
{
	int   ret_code            = ITK_ok;
	tag_t root_task           = NULLTAG;
	char *arg_key			  = NULL;	
	char *arg_value	          = NULL;
	char *dsUid = NULL;
	char *conditionArg = NULL;
	char *moveToAttachmentArg = NULL;
	int moveToAttachment = -1;
	tag_t inpuDs = NULLTAG;
	char ErrorMessage[EMH_MAXEMSG];

	char *isDebug = NULL;
	CHECK_ITK(ret_code, PREF_ask_char_value("VF_PLM_DEBUG", 0, &isDebug));
	
	LOG(TC_write_syslog("@@@Enter AH_route_process_by_dataset\n"));

	int num_args = TC_number_of_arguments(msg.arguments);
	if (num_args > 0)
	{
		for(int i = 0; i < num_args; ++i)
		{
			CHECK_ITK(ret_code, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &arg_key, &arg_value));
			if (ret_code == ITK_ok )
			{
				if (tc_strcasecmp(arg_key, "condition") == 0 && arg_value != NULL)
				{
					conditionArg = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(conditionArg, arg_value);
				}
				else if (tc_strcasecmp(arg_key, "move_to") == 0 && arg_value != NULL)
				{
					moveToAttachmentArg = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(moveToAttachmentArg, arg_value);
					if (tc_strcasecmp(moveToAttachmentArg, "target") == 0)
					{
						moveToAttachment = EPM_target_attachment;
					} 
					else
					{
						moveToAttachment = EPM_reference_attachment;
					}
				}
			}
		}
	}

	if (conditionArg == NULL || moveToAttachmentArg == NULL)
	{
		sprintf( ErrorMessage, "Missing required argument for the handler.");
		CHECK_ITK(ret_code, EMH_store_error_s1(EMH_severity_user_error, -1, ErrorMessage));
		return -1;
	}

	bool setTaskResultSuccessfully = false;
	LOG(TC_write_syslog("got -condition %s\n", conditionArg));
	LOG(TC_write_syslog("got -moveToAttachmentArg %s\n", moveToAttachmentArg));

	CHECK_ITK(ret_code, EPM_ask_root_task(msg.task, &root_task));
	LOG(TC_write_syslog("got root task %d\n", root_task));
	//CHECK_ITK(ret_code, POM_tag_to_string(root_task, &dsUid));
	ITK__convert_tag_to_uid(root_task, &dsUid);
	LOG(TC_write_syslog("got dsUid %s\n", dsUid));
	CHECK_ITK(ret_code, AE_find_dataset2(dsUid, &inpuDs));
	if (inpuDs != NULLTAG)
	{
		LOG(TC_write_syslog("start read dsUid %s\n", dsUid));
		CHECK_ITK(ret_code,AOM_refresh(inpuDs, FALSE));
		AE_reference_type_t ref_type;
		tag_t text_file;
		std::string dsContent("");

        //CHECK_ITK(ret_code,AE_ask_dataset_named_ref(inpuDs, "Text",
        //    &ref_type, &text_file)); //@SKIP_DEPRECATED ==> THIS HANDLER IS NOT USED ANY MORE
        if (text_file != NULLTAG)
		{
			CHECK_ITK(ret_code,AOM_refresh(text_file, FALSE));

			IMF_file_t file_descriptor;
			CHECK_ITK(ret_code,IMF_ask_file_descriptor(text_file, &file_descriptor));
			CHECK_ITK(ret_code,IMF_open_file(file_descriptor, SS_RDONLY));
			
			char text_line[SS_MAXLLEN+1];
			//while (IMF_read_file_line(file_descriptor, text_line) == ITK_ok)//@SKIP_DEPRECATED
			{
				dsContent.append(text_line).append("\n");
			}
			CHECK_ITK(ret_code,IMF_close_file(file_descriptor));
			LOG(TC_write_syslog("IMF_close_file ret_code=%d\n",ret_code));
		}
		CHECK_ITK(ret_code,AOM_unload(text_file));
        CHECK_ITK(ret_code,AOM_unload(inpuDs));

		LOG(TC_write_syslog("got dsContent %s\n", dsContent.c_str()));

		if (!dsContent.empty())
		{
			std::vector<string> rules = split_string(conditionArg, '|');
			for(const auto rule : rules)
			{
				//std::vector<string> conditionAndOutway = split_string(rule, '^');
				//if (conditionAndOutway.size() == 2)
				{
					//std::string condition = conditionAndOutway.at(0);
					//std::string outWay = conditionAndOutway.at(1);
					std::string condition = rule;
					//LOG(TC_write_syslog("checking condition-outway %s - %s\n",condition.c_str(), outWay.c_str()));
					LOG(TC_write_syslog("checking condition %s\n",condition.c_str()));
					std::vector<string> lines = split_string(dsContent.c_str(), '\n');
					
					for(const auto line : lines)
					{
						LOG(TC_write_syslog("line %s\n", line.c_str()));
						if (line.find(condition) != line.npos)
						{
							LOG(TC_write_syslog("found condition\n"));
							//pinggo
							std::string revUid = line.substr(line.find("UID=") + 4, 14);
							tag_t revTag = NULLTAG;
							LOG(TC_write_syslog("revUid %s\n", revUid.c_str()));
							ITK__convert_uid_to_tag(revUid.c_str(), &revTag);
							CHECK_ITK(ret_code, EPM_remove_attachments(root_task, 1, &revTag));
							if (ret_code != ITK_ok) goto CLEAN;
							CHECK_ITK(ret_code, EPM_add_attachments(root_task, 1, &revTag, &moveToAttachment));
							if (ret_code != ITK_ok) goto CLEAN;

							if (ret_code == ITK_ok)
							{
								//setTaskResultSuccessfully = true;
								EMH_clear_errors();
							}

							/*CHECK_ITK(ret_code, EPM_set_task_result(msg.task, outWay.c_str()));
							if (ret_code == ITK_ok)
							{
								setTaskResultSuccessfully = true;
								EMH_clear_errors();
								ret_code = ITK_ok;
							}*/

							break;
						}
					}
				}
			}

		}
	}

CLEAN:
	SAFE_SM_FREE(conditionArg);
	SAFE_SM_FREE(moveToAttachmentArg);
	SAFE_SM_FREE(isDebug);
	
	LOG(TC_write_syslog("@@@LEAVE AH_route_process_by_dataset\n"));/////////

	return ret_code;
}