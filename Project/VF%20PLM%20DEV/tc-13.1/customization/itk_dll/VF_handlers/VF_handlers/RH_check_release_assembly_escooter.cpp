/***************************************************************************************
File         : RH_check_release_assembly.cpp

Description  : To check if all child parts in assembly has been released or in process?. 

Input        : object_type, release_status, released_by_wf

Output       : GO/NO_GO

Author       : Vinfast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Feb,26 2020     1.0         thanhpn6		Initial Creation

******************************************************************************************/
#include "Vinfast_Custom.h"

auto check_for_sub_assembly(tag_t tag_top_line, char* request_status, char* released_by_wf) -> logical
{
	auto ret_code = ITK_ok;
	auto total_lines = 0;
	auto bl_item_attr_id = 0;
	auto bl_revision_attr_id = 0;
	auto bl_status_attr_id = 0;
	tag_t* children_lines = nullptr;
	tag_t item_revision = NULLTAG;
	char error_message[2048] = "";
	auto is_released = true;

	CHECK_ITK(ret_code, BOM_line_ask_all_child_lines(tag_top_line, &total_lines, &children_lines))

	for (auto i = 0; i < total_lines; i++)
	{
		char* bl_item_item_id = nullptr;
		char* bl_rev_item_revision_id = nullptr;
		char* object_type = nullptr;
		std::string release_status;

		const tag_t child_line = children_lines[i];

		CHECK_ITK(ret_code, BOM_line_look_up_attribute("bl_item_item_id", &bl_item_attr_id))
		CHECK_ITK(ret_code, BOM_line_look_up_attribute("bl_rev_item_revision_id", &bl_revision_attr_id))
		CHECK_ITK(ret_code, BOM_line_look_up_attribute("bl_rev_release_status_list", &bl_status_attr_id))

		/*getting item id, rev id and release status of each bom line*/
		CHECK_ITK(ret_code, BOM_line_ask_attribute_string(child_line, bl_item_attr_id, &bl_item_item_id))
		CHECK_ITK(ret_code, BOM_line_ask_attribute_string(child_line, bl_revision_attr_id, &bl_rev_item_revision_id))

		tag_t* t_release_status_list = nullptr;
		int total_release_status_list = -1;

		CHECK_ITK(ret_code, AOM_ask_value_tag(child_line,"bl_line_object", &item_revision))
		CHECK_ITK(ret_code, AOM_ask_value_tags(item_revision,"release_status_list", &total_release_status_list, &t_release_status_list))

		for (int inx = 0; inx < total_release_status_list; inx++)
		{
			char* sz_status_tmp = nullptr;
			CHECK_ITK(ret_code, RELSTAT_ask_release_status_type(t_release_status_list[inx], &sz_status_tmp))
			if (release_status.empty() == false)
			{
				release_status.append(",");
			}
			release_status.append(sz_status_tmp);

			SAFE_SM_FREE(sz_status_tmp);
		}

		SAFE_SM_FREE(t_release_status_list);

		CHECK_ITK(ret_code, AOM_ask_value_tag(child_line,"bl_line_object",&item_revision))
		CHECK_ITK(ret_code, WSOM_ask_object_type2(item_revision,&object_type))

		/*ignore revision > 01 */
		if (strcmp(bl_rev_item_revision_id, "01") != 0)
		{
			continue;
		}

		/*check if child part already applied in other workflow*/
		auto total_triggered_process = 0;
		tag_t* processes = nullptr;
		auto in_release_processing = false;
		/*getting number of workflow process*/
		CHECK_ITK(ret_code, AOM_ask_value_tags(item_revision, "fnd0AllWorkflows", &total_triggered_process, &processes))
		if (total_triggered_process >= 1)
		{
			for (auto iterator = 0; iterator < total_triggered_process; iterator++)
			{
				char* process_name = nullptr;

				/*check for process name matching with argument*/
				CHECK_ITK(ret_code, AOM_ask_value_string(processes[iterator],"current_name", &process_name))
				if (tc_strcmp(process_name, released_by_wf) == 0)
				{
					in_release_processing = true;
				}
				SAFE_MEM_free(process_name)
			}
		}

		if (ret_code != ITK_ok)
		{
			TC_write_syslog("[VF_handlers_RH_check_release_assembly_escooter]__%d__%s__Failed to get Item Type for %s. RetCode: %d", __LINE__, __FUNCTION__, bl_item_item_id, ret_code);
		}


		/*if release status of bom line is equal to status specified in handler argument*/
		if (tc_strcmp(object_type, "VF4_scooter_vehRevision") == 0 || tc_strcmp(object_type, "VF4_scooter_modRevision") == 0 ||
			tc_strcmp(object_type, "VF3_Scooter_partRevision") == 0)
		{
			if (!has_valid_status(const_cast<char*>(release_status.c_str()), request_status) && !in_release_processing)
			{
				is_released = false;
				sprintf_s(error_message, sizeof(error_message), "%s/%s is not having valid Release status. All child part must to have valid Release Status to continue with the workflow.", bl_item_item_id, bl_rev_item_revision_id);
				TC_write_syslog(error_message);
				EMH_store_error_s1(EMH_severity_user_error, CHECK_STATUS, error_message);
				break;
			}
		}

		//if(lReleased == true)
		//{
		//	/*check part is assembly or not*/
		//	int		total_bomview_rev		= 0;
		//	tag_t	*bom_view_rev			= NULLTAG;
		//	tag_t	*ptLines	= NULLTAG;
		//	tag_t	tItem		= NULLTAG;
		//	tag_t	tWindow		= NULLTAG;
		//	tag_t	tag_top_line	= NULLTAG;
		//	CHECK_ITK(iRetCode, ITEM_rev_list_bom_view_revs(tItemRevision, &total_bomview_rev, &bom_view_rev));
		//	if(total_bomview_rev > 0){
		//		/*getting all children of the assembly and bom line property of each*/
		//		CHECK_ITK(iRetCode,ITEM_ask_item_of_rev(tItemRevision, &tItem));
		//		CHECK_ITK(iRetCode,BOM_create_window(&tWindow));
		//		CHECK_ITK(iRetCode,BOM_set_window_top_line(tWindow, tItem, tItemRevision, NULLTAG, &tag_top_line));
		//		lReleased = check_for_sub_assembly(tag_top_line, request_status, released_by_wf);
		//	}
		//	SAFE_MEM_free(bom_view_rev);
		//}

		SAFE_MEM_free(bl_item_item_id)
		SAFE_MEM_free(object_type)
		SAFE_MEM_free(bl_rev_item_revision_id)
		SAFE_MEM_free(processes)
	}
	SAFE_MEM_free(children_lines)
	return is_released;
}


/**
 * \brief check whether all child line has status PR4D_PR or in a release process
 * \param msg
 * \return decision
 */
auto RH_check_release_assembly_escooter(const EPM_rule_message_t msg) -> EPM_decision_t
{
	auto ret_code = ITK_ok;
	auto root_task = NULLTAG;

	char* release_status_required = nullptr;
	char* released_by_wf = nullptr;
	char* arg_key = nullptr;
	char* arg_value = nullptr;
	auto decision = EPM_go;
	vector<string> solution_items;
	
	/*getting argument*/
	const auto num_args = TC_number_of_arguments(msg.arguments);
	if (num_args > 0)
	{
		for (auto i = 0; i < num_args; ++i)
		{
			CHECK_ITK(ret_code, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &arg_key, &arg_value))
			if (ret_code == ITK_ok)
			{
				if (tc_strcasecmp(arg_key, "release_status") == 0 && arg_value != nullptr)
				{
					release_status_required = static_cast<char*>(MEM_alloc((static_cast<int>(tc_strlen(arg_value)) + 1) * sizeof(char)));
					tc_strcpy(release_status_required, arg_value);
				}
				if (tc_strcasecmp(arg_key, "released_by_wf") == 0 && arg_value != nullptr)
				{
					released_by_wf = static_cast<char*>(MEM_alloc((static_cast<int>(tc_strlen(arg_value)) + 1) * sizeof(char)));
					tc_strcpy(released_by_wf, arg_value);
				}
			}
		}
		SAFE_MEM_free(arg_key)
		SAFE_MEM_free(arg_value)
	}

	CHECK_ITK(ret_code, EPM_ask_root_task(msg.task, &root_task))

	if (root_task != NULLTAG)
	{
		auto total_target_obj = 0;
		tag_t* target_obj = nullptr;

		/*getting target objects*/
		CHECK_ITK(ret_code, EPM_ask_attachments(root_task, EPM_target_attachment, &total_target_obj, &target_obj))
		for (auto i = 0; i < total_target_obj; ++i)
		{
			char* item_id = nullptr;
			char* object_name = nullptr;
			char* object_type = nullptr;
			CHECK_ITK(ret_code, WSOM_ask_id_string(target_obj[i], &item_id))
			CHECK_ITK(ret_code, WSOM_ask_name2(target_obj[i], &object_name))
			CHECK_ITK(ret_code, WSOM_ask_object_type2(target_obj[i], &object_type))
			if (tc_strcasecmp(object_type, "VF4_scooter_vehRevision") != 0 &&
				tc_strcasecmp(object_type, "VF4_scooter_modRevision") != 0 && 
				tc_strcasecmp(object_type, "VF3_Scooter_partRevision") != 0)
			{
				TC_write_syslog("[VF_handlers_RH_check_release_assembly_escooter]__%d__%s__Ignore part: %s-%s", __LINE__, __FUNCTION__, item_id, object_name);
				continue;
			}

			/*check part is assembly or not*/
			auto total_bomview_rev = 0;
			tag_t* bom_view_rev = nullptr;
			CHECK_ITK(ret_code, ITEM_rev_list_bom_view_revs(target_obj[i], &total_bomview_rev, &bom_view_rev))
			if (total_bomview_rev > 0)
			{
				auto item = NULLTAG;
				auto window = NULLTAG;
				auto top_line = NULLTAG;
				/*getting all children of the assembly and bom line property of each*/
				CHECK_ITK(ret_code, ITEM_ask_item_of_rev(target_obj[i], &item))
				CHECK_ITK(ret_code, BOM_create_window(&window))
				CHECK_ITK(ret_code, BOM_set_window_top_line(window, item, target_obj[i], NULLTAG, &top_line))  // NOLINT(clang-diagnostic-old-style-cast)
				const auto is_released = check_for_sub_assembly(top_line, release_status_required, released_by_wf);
				if (is_released == false)
				{
					decision = EPM_nogo;
					return decision;
				}
			}
			SAFE_MEM_free(bom_view_rev)
		}
	}

	SAFE_MEM_free(released_by_wf)
	SAFE_MEM_free(release_status_required)

	return decision;
}



