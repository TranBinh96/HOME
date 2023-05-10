/***************************************************************************************
Function     : AH_create_purchasing_object()
Description  : call create_sourcing_object() and
create_cost_object() for each target object in root task
Input        : None
Output       : None
Author       : thanhpn6
******************************************************************************************/
#include "Vinfast_Custom.h"

struct ST_Create_Input {
	std::string item_id;
	std::string object_name;
	std::string sourcing_obj_type;
	std::string car_program;
	std::string cl_user_name;
	std::string part_obj_type;
	std::string purchase_lvl;
	std::string change_index;
	std::string module_grp;
	std::string module_name;
	std::string main_module;
	std::string uom;
	tag_t clGrpMem;

};

auto create_sourcing_object(ST_Create_Input st_input, tag_t& sourcing_rev) -> int;
auto create_cost_object(const char* item_id, const char* object_name, tag_t* cl_user, tag_t* cl_group, tag_t& cost_rev) -> int;
auto grant_access(tag_t object_tag) -> int;
auto get_bomline_property(const std::string& dataset_name, const tag_t sor_rev)->std::map<std::string, std::map<std::string, std::string>>;
auto split(const std::string& s, const std::string& delimiter)->vector<string>;
auto change_ownership(tag_t sourcing_item_rev, tag_t sourcing_item, tag_t user_cl) -> void;
auto getCLGroupMember(tag_t root_task, const char* cl_review_task_name)->tag_t;
auto get_cl_display_value(tag_t user_tag)->std::string;
auto is_item_stamp_VF4_Sourcing(tag_t t_item_rev) -> bool;
auto query_sourcing_part(char* part_id, char* sourcing_program) -> int;
auto is_valid_purchase_level(const char* purchase_lvl, std::vector<std::string> valid_purchase)->boolean;


extern int AH_create_purchasing_object(EPM_action_message_t msg)
{
	int   ret_code = ITK_ok;
	tag_t root_task = NULLTAG;
	tag_t sourcing_rev = NULLTAG;
	tag_t clGrpMem = NULLTAG;
	char* car_program = nullptr;
	char* cl_review_task_name = nullptr;
	char* arg_key = nullptr;
	char* arg_value = nullptr;
	char* sourcing_obj_type = nullptr;
	char		ErrorMessage[2048] = "";
	std::string os_user_name;
	std::map<std::string, std::string> prg_2_objType;
	std::vector<std::string> valid_purchase_lvl;

	/*getting argument*/

	/*read mapping sourcing program to object type*/
	char** arr_prg_2objType = NULL;
	int count = 0;
	CHECK_ITK(ret_code, PREF_ask_char_values("VF_SOURCING_PROGRAM_2_OBJECT_TYPE", &count, &arr_prg_2objType));
	for (int i = 0; i < count; i++)
	{
		std::string one_value = arr_prg_2objType[i];
		vector<std::string> v = split(one_value, ";");
		prg_2_objType.insert(std::pair<std::string, std::string>(v[0], v[1]));
	}
	SAFE_MEM_free(arr_prg_2objType);

	/*read valid purchase level list from preference*/
	char** arr_valid_purchaseLvl = NULL;
	int count2 = 0;
	CHECK_ITK(ret_code, PREF_ask_char_values("VF_Sourcing_Valid_PurchaseLevel", &count2, &arr_valid_purchaseLvl));
	for (int i = 0; i < count2; i++)
	{
		valid_purchase_lvl.push_back(arr_valid_purchaseLvl[i]);
	}
	SAFE_MEM_free(arr_valid_purchaseLvl);

	const auto num_args = TC_number_of_arguments(msg.arguments);
	if (num_args > 0)
	{
		for (int i = 0; i < num_args; ++i)
		{
			CHECK_ITK(ret_code, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &arg_key, &arg_value));
			if (ret_code == ITK_ok)
			{
				/*if (tc_strcasecmp(arg_key, "car_program") == 0 && arg_value != nullptr)
				{
					car_program = (char*)MEM_alloc(((int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(car_program, arg_value);
				}*/
				if (tc_strcasecmp(arg_key, "ask_cl_from_task") == 0 && arg_value != nullptr)
				{
					cl_review_task_name = (char*)MEM_alloc(((int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(cl_review_task_name, arg_value);
				}


			}
			SAFE_MEM_free(arg_key);
			SAFE_MEM_free(arg_value);
		}
	}

	/*getting root task*/
	CHECK_ITK(ret_code, EPM_ask_root_task(msg.task, &root_task));

	if (root_task != NULLTAG)
	{
		/*get sor item revision: at this step sor item revision will be moved to reference*/
		int	  total_refer_obj = 0;
		tag_t* refer_obj = nullptr;
		tag_t sor_rev = NULL;
		CHECK_ITK(ret_code, EPM_ask_attachments(root_task, EPM_reference_attachment, &total_refer_obj, &refer_obj));
		for (int i = 0; i < total_refer_obj; ++i)
		{
			char* object_type = nullptr;
			CHECK_ITK(ret_code, WSOM_ask_object_type2(refer_obj[i], &object_type));
			if (tc_strcasecmp(object_type, "VF3_spec_docRevision") == 0 || 
				tc_strcasecmp(object_type, "VF3_VES_BSOR_docRevision") == 0 ||
				tc_strcasecmp(object_type, "VF3_VES_CSOR_docRevision") == 0)
			{
				sor_rev = refer_obj[i];
				SAFE_MEM_free(object_type);
				break;
			}
			SAFE_MEM_free(object_type);
		}

		/*get map bomline property*/
		char* root_task_uid = nullptr;

		ITK__convert_tag_to_uid(root_task, &root_task_uid);
		std::map<std::string, std::map<std::string, std::string>> mapping_partID_bomlineProp;
		if (root_task_uid != nullptr && sor_rev != NULL)
		{
			CHECK_ITK(ret_code, AOM_ask_value_string(sor_rev, "vf4_car_program", &car_program));
			if (prg_2_objType.find(car_program) != prg_2_objType.end())
			{
				sourcing_obj_type = (char*)MEM_alloc(((int)tc_strlen(prg_2_objType.find(car_program)->second.c_str()) + 1) * sizeof(char));
				tc_strcpy(sourcing_obj_type, prg_2_objType.find(car_program)->second.c_str());
			}
			else
			{
				TC_write_syslog("[VF_handlers]__%d__%s__Please add sourcing object type into VF_SOURCING_PROGRAM_2_OBJECT_TYPE for program: %s\n", __LINE__, __FUNCTION__, car_program);
				return SOURCING_INVALID_OBJ_TYPE;
			}

			mapping_partID_bomlineProp = get_bomline_property(std::string(root_task_uid), sor_rev);
			//mapping_partID_bomlineProp = get_bomline_property(std::string("testing8693"));
			if (mapping_partID_bomlineProp.empty())
			{
				sprintf(ErrorMessage, "Could not find data set with name: %s", root_task_uid);
				EMH_store_error_s1(EMH_severity_error, INVALID_BOM_PROPERTY, ErrorMessage);
				return INVALID_BOM_PROPERTY;
			}
		}

		/*get user and group to change ownership*/
		if (cl_review_task_name != nullptr)
		{
			clGrpMem = getCLGroupMember(root_task, cl_review_task_name);
			tag_t tUser = NULLTAG;
			CHECK_ITK(ret_code, SA_ask_groupmember_user(clGrpMem, &tUser));
			os_user_name = get_cl_display_value(tUser);
		}
		int	  total_target_obj = 0;
		tag_t* target_obj = nullptr;

		/*getting target objects*/
		CHECK_ITK(ret_code, EPM_ask_attachments(root_task, EPM_target_attachment, &total_target_obj, &target_obj));
		for (int i = 0; i < total_target_obj; ++i)
		{
			char* item_id = nullptr;
			char* object_name = nullptr;
			char* object_type = nullptr;
			CHECK_ITK(ret_code, AOM_ask_value_string(target_obj[i], "item_id", &item_id));
			//CHECK_ITK(ret_code, WSOM_ask_id_string(target_obj[i], &item_id));
			CHECK_ITK(ret_code, WSOM_ask_name2(target_obj[i], &object_name));
			CHECK_ITK(ret_code, WSOM_ask_object_type2(target_obj[i], &object_type));
			if (tc_strcasecmp(object_type, "VF4_DesignRevision") != 0 && tc_strcasecmp(object_type, "VF4_EBUSDesignRevision") != 0 &&
				tc_strcasecmp(object_type, "VF3_Scooter_partRevision") != 0 && tc_strcasecmp(object_type, "VF4_BP_DesignRevision") != 0 &&
				tc_strcasecmp(object_type, "VF3_manuf_partRevision") != 0 && tc_strcasecmp(object_type, "VF3_car_partRevision") != 0
				&& tc_strcasecmp(object_type, "VF4_Cell_DesignRevision") != 0
				&& tc_strcasecmp(object_type, "VF4_Compo_DesignRevision") != 0
				&& tc_strcasecmp(object_type, "VF4_Service_KitRevision") != 0
				&& tc_strcasecmp(object_type, "VF4_VES_ME_PartRevision") != 0)
			{
				continue;
			}
			TC_write_syslog("[VF_handlers]__%d__%s__Start create sourcing and cost item for part: %s-%s\n", __LINE__, __FUNCTION__, item_id, object_name);

			/*creating sourcing object*/

			//std::vector<string> o = split_string(item_id, '/');
			char* part_rev_uid = nullptr;
			ITK__convert_tag_to_uid(target_obj[i], &part_rev_uid);
			std::map<std::string, std::string> bomline_props;
			if (part_rev_uid != nullptr)
			{
				auto iterator = mapping_partID_bomlineProp.find(std::string(part_rev_uid));
				if (iterator != mapping_partID_bomlineProp.end())
				{
					bomline_props = iterator->second;
				}
			}

			ST_Create_Input st_input = {};
			if (bomline_props.size() == 7)
			{
				try
				{
					st_input.purchase_lvl = bomline_props.at("VL5_purchase_lvl_vf");
					st_input.change_index = bomline_props.at("VL5_change_index");
					st_input.module_grp = bomline_props.at("VL5_module_group");
					st_input.module_name = bomline_props.at("VL5_module_name");
					st_input.main_module = bomline_props.at("VL5_main_module");
					st_input.uom = bomline_props.at("bl_item_uom_tag");
				}
				catch (const std::out_of_range&)
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Could not find bom line property: %s\n", __LINE__, __FUNCTION__, item_id);
				}
			}
			if (!st_input.purchase_lvl.empty() && !st_input.change_index.empty() && is_valid_purchase_level(st_input.purchase_lvl.c_str(), valid_purchase_lvl))
			{
				if (query_sourcing_part(item_id, car_program) == 0)
				{
					st_input.item_id = item_id;
					st_input.object_name = object_name;
					st_input.sourcing_obj_type = sourcing_obj_type;
					st_input.car_program = car_program;
					st_input.cl_user_name = os_user_name;
					st_input.part_obj_type = object_type;
					st_input.clGrpMem = clGrpMem;


					ret_code = create_sourcing_object(st_input, sourcing_rev);
					if (ret_code != ITK_ok)
					{
						TC_write_syslog("[VF_handlers]__%d__%s__Error when create sourcing object: %d\n", __LINE__, __FUNCTION__, ret_code);
					}
					else
					{
						/*creating relation between part revision and sourcing revision*/
						tag_t sourcing_rela_type = NULLTAG;
						tag_t sourcing_rela_created = NULLTAG;
						CHECK_ITK(ret_code, GRM_find_relation_type("VF4_Sourcing_Reference", &sourcing_rela_type));
						CHECK_ITK(ret_code, GRM_create_relation(target_obj[i], sourcing_rev, sourcing_rela_type, NULLTAG, &sourcing_rela_created));
						if (ret_code == ITK_ok)
						{
							CHECK_ITK(ret_code, AOM_refresh(target_obj[i], 0));
							if (ret_code == ITK_ok)
							{
								/*save relation*/
								CHECK_ITK(ret_code, GRM_save_relation(sourcing_rela_created))
							}
						}

					}
				}

				/*thanhpn6-11Aug2021-only stamp Sourcing if part not have Sourcing flag*/
				tag_t* t_release_status_list = nullptr;
				int total_release_status_list = -1;
				CHECK_ITK(ret_code, AOM_ask_value_tags(target_obj[i], "release_status_list", &total_release_status_list, &t_release_status_list));
				std::string release_status;
				for (int inx = 0; inx < total_release_status_list; inx++)
				{
					char* sz_status_tmp = nullptr;
					CHECK_ITK(ret_code, RELSTAT_ask_release_status_type(t_release_status_list[inx], &sz_status_tmp));
					if (release_status.empty() == false)
					{
						release_status.append(",");
					}
					release_status.append(sz_status_tmp);

					SAFE_SM_FREE(sz_status_tmp);
				}

				SAFE_SM_FREE(t_release_status_list);

				//TODO stamping VF4_Sourcing
				if (release_status.find("VF4_Sourcing") == string::npos)
				{
					tag_t	status_object = NULLTAG;
					CHECK_ITK(ret_code, RELSTAT_create_release_status("VF4_Sourcing", &status_object));
					CHECK_ITK(ret_code, RELSTAT_add_release_status(status_object, 1, &target_obj[i], true));

					std::vector<std::string> vec_relation_name;
					vec_relation_name.push_back("IMAN_reference");
					vec_relation_name.push_back("PSBOMView");
					vec_relation_name.push_back("IMAN_Rendering");
					vec_relation_name.push_back("PSBOMViewRevision");
					vec_relation_name.push_back("IMAN_specification");
					vec_relation_name.push_back("IMAN_specification");

					for (auto iterator = 0; iterator < vec_relation_name.size(); iterator++)
					{
						auto relatedObj_counter = 0;
						tag_t relation_object = NULLTAG;
						tag_t* related_object = nullptr;

						CHECK_ITK(ret_code, GRM_find_relation_type(vec_relation_name[iterator].c_str(), &relation_object));
						CHECK_ITK(ret_code, GRM_list_secondary_objects_only(target_obj[i], relation_object, &relatedObj_counter, &related_object));
						for (auto iterator2 = 0; iterator2 < relatedObj_counter; iterator2++)
						{
							CHECK_ITK(ret_code, RELSTAT_add_release_status(status_object, 1, &related_object[iterator2], true));
						}
						SAFE_MEM_free(related_object);
					}
				}

			}
			SAFE_MEM_free(item_id);
			SAFE_MEM_free(object_name);
			SAFE_MEM_free(object_type);
		}
	}
	SAFE_MEM_free(car_program);
	return ret_code;
}

/***************************************************************************************
Function     : create_sourcing_object()
Description  : call create_sourcing_object() for each target object in root task
Input        : None
Output       : None
Author       : thanhpn6
******************************************************************************************/

int create_sourcing_object(ST_Create_Input st_input, tag_t & sourcing_rev)
{
	int   ret_code = ITK_ok;
	tag_t item_type_tag = NULLTAG;
	tag_t itemRev_type_tag = NULLTAG;
	tag_t item_create_input_tag = NULLTAG;
	tag_t itemRev_create_input_tag = NULLTAG;
	tag_t item_tag = NULLTAG;

	/*creating item*/
	CHECK_ITK(ret_code, TCTYPE_find_type(st_input.sourcing_obj_type.c_str(), st_input.sourcing_obj_type.c_str(), &item_type_tag));
	if (item_type_tag == NULLTAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Could not find tag of object type: %s\n", __LINE__, __FUNCTION__, st_input.sourcing_obj_type);
	}
	CHECK_ITK(ret_code, TCTYPE_construct_create_input(item_type_tag, &item_create_input_tag));

	/*find item revision tag type*/
	const std::string sourcing_partRev_object_type = st_input.sourcing_obj_type.append("Revision");
	CHECK_ITK(ret_code, TCTYPE_find_type(sourcing_partRev_object_type.c_str(), sourcing_partRev_object_type.c_str(), &itemRev_type_tag));
	if (itemRev_type_tag == NULLTAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Could not find tag of object type: %s\n", __LINE__, __FUNCTION__, sourcing_partRev_object_type);
	}
	const char* tmp_cl_name = st_input.cl_user_name.c_str();
	const char* tmp_obj_name = st_input.object_name.c_str();
	const char* tmp_item_id = st_input.item_id.c_str();
	const char* tmp_car_program = st_input.car_program.c_str();
	const char* tmp_part_obj_type = st_input.part_obj_type.c_str();
	const char* tmp_purchase_lvl = st_input.purchase_lvl.c_str();
	const char* tmp_change_index = st_input.change_index.c_str();
	const char* tmp_module_grp = st_input.module_grp.c_str();
	const char* tmp_module_name = st_input.module_name.c_str();
	const char* tmp_main_module = st_input.main_module.c_str();
	const char* tmp_uom = st_input.uom.c_str();

	TC_write_syslog("[VF_handlers]__%d__%s__Create sourcing part: %s\n", __LINE__, __FUNCTION__, st_input.item_id);
	tag_t tCLUser = NULLTAG;
	CHECK_ITK(ret_code, SA_ask_groupmember_user(st_input.clGrpMem, &tCLUser));

	CHECK_ITK(ret_code, TCTYPE_construct_create_input(itemRev_type_tag, &itemRev_create_input_tag));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "object_name", 1, &tmp_obj_name));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_bom_vfPartNumber", 1, &tmp_item_id));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_platform_module", 1, &tmp_car_program));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_cl", 1, &tmp_cl_name));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_car_part_bo_type", 1, &tmp_part_obj_type));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_purchasing_level", 1, &tmp_purchase_lvl));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_bom_change_index", 1, &tmp_change_index));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_module_group_bom", 1, &tmp_module_grp));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_module_name", 1, &tmp_module_name));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_main_module_bom", 1, &tmp_main_module));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_bl_unit_of_measure", 1, &tmp_uom));

	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "object_name", 1, &tmp_obj_name));


	CHECK_ITK(ret_code, AOM_set_value_tag(itemRev_create_input_tag, "vf4_cl_2", tCLUser));
	CHECK_ITK(ret_code, AOM_set_value_tag(item_create_input_tag, "revision", itemRev_create_input_tag));
	CHECK_ITK(ret_code, TCTYPE_create_object(item_create_input_tag, &item_tag));
	CHECK_ITK(ret_code, AOM_save_with_extensions(item_tag));
	CHECK_ITK(ret_code, ITEM_ask_latest_rev(item_tag, &sourcing_rev));
	if (item_tag != NULLTAG && strcmp("VFe32", tmp_car_program) != 0)
	{
		/*getting lastest revision of item*/
		change_ownership(sourcing_rev, item_tag, st_input.clGrpMem);
	}
	return ret_code;
}

/***************************************************************************************
Function     : create_cost_object()
Description  : call create_cost_object() for each target object in root task
Input        : None
Output       : None
Author       : thanhpn6
******************************************************************************************/

int create_cost_object(const char* item_id, const char* object_name, tag_t * cl_user, tag_t * cl_group, tag_t & cost_rev)
{
	int   ret_code = ITK_ok;
	tag_t item_type_tag = NULLTAG;
	tag_t item_create_input_tag = NULLTAG;
	tag_t item_tag = NULLTAG;

	/*creating item*/
	CHECK_ITK(ret_code, TCTYPE_find_type("VF4_Cost", "VF4_Cost", &item_type_tag));
	if (item_type_tag == NULLTAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Could not find tag of object type VF4_Cost\n", __LINE__, __FUNCTION__);
	}
	CHECK_ITK(ret_code, TCTYPE_construct_create_input(item_type_tag, &item_create_input_tag));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "object_name", 1, &object_name));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "item_id", 1, &item_id));

	CHECK_ITK(ret_code, TCTYPE_create_object(item_create_input_tag, &item_tag));
	CHECK_ITK(ret_code, ITEM_save_item(item_tag));

	/*getting lastest revision of item*/
	if (item_tag != NULLTAG)
	{
		CHECK_ITK(ret_code, ITEM_ask_latest_rev(item_tag, &cost_rev));
		CHECK_ITK(ret_code, AOM_set_value_string(cost_rev, "object_name", object_name));
	}


	CHECK_ITK(ret_code, ITEM_save_rev(cost_rev));

	CHECK_ITK(ret_code, ITEM_save_item(item_tag));

	return 0;

}

tag_t getCLGroupMember(tag_t root_task, const char* cl_review_task_name)
{
	int    total_task = 0;
	int    ret_code = ITK_ok;
	tag_t* sub_task = nullptr;
	tag_t clGrpMem = NULL_TAG;
	char* os_user_name = nullptr;

	CHECK_ITK(ret_code, EPM_ask_sub_tasks(root_task, &total_task, &sub_task));
	for (int i = 0; i < total_task; ++i)
	{
		char* task_name = nullptr;
		char* task_type = nullptr;

		CHECK_ITK(ret_code, AOM_ask_value_string(sub_task[i], "object_name", &task_name));
		CHECK_ITK(ret_code, AOM_ask_value_string(sub_task[i], "task_type", &task_type));
		//get reviewer when CL task is review task
		if (tc_strcmp(task_name, cl_review_task_name) == 0 && tc_strcmp(task_type, "EPMReviewTask") == 0)
		{
			int		total_child_task = 0;
			tag_t* child_task = nullptr;
			CHECK_ITK(ret_code, EPM_ask_sub_tasks(sub_task[i], &total_child_task, &child_task));
			for (int j = 0; j < total_child_task; j++)
			{
				char* child_task_type = nullptr;
				CHECK_ITK(ret_code, AOM_ask_value_string(child_task[j], "task_type", &child_task_type));
				if (tc_strcmp(child_task_type, "EPMSelectSignoffTask") == 0)
				{
					int		total_att = 0;
					tag_t* att_tag_list = nullptr;

					//get signoff attachments
					CHECK_ITK(ret_code, EPM_ask_attachments(child_task[j], EPM_signoff_attachment, &total_att, &att_tag_list));
					//CHECK_ITK(ret_code, AOM_ask_value_tag(att_tag_list[0], "fnd0Performer", &cl_user));

					SIGNOFF_TYPE_t memType = SIGNOFF_UNDEFINED;
					CHECK_ITK(ret_code, EPM_ask_signoff_member(att_tag_list[0], &clGrpMem, &memType));
					if (memType == SIGNOFF_GROUPMEMBER)
					{
						tag_t tUser = NULLTAG;
						tag_t tGroup = NULLTAG;
						int noOfmember;
						CHECK_ITK(ret_code, SA_ask_groupmember_user(clGrpMem, &tUser));
						CHECK_ITK(ret_code, SA_ask_groupmember_group(clGrpMem, &tGroup));
					}
					else
					{
						TC_write_syslog("[VF_handlers]__%d__%s__Could not get CL group member: %d\n", __LINE__, __FUNCTION__, ret_code);
					}

					SAFE_MEM_free(att_tag_list);
				}
				SAFE_MEM_free(child_task_type);
			}
			break;
		}

		SAFE_MEM_free(task_name);
		SAFE_MEM_free(task_type);
		SAFE_MEM_free(os_user_name);
	}
	SAFE_MEM_free(sub_task);

	return clGrpMem;
}

bool is_item_stamp_VF4_Sourcing(const tag_t t_item_rev)
{
	auto is_sourcing_existed = false;
	auto ret_code = ITK_ok;
	tag_t t_item = NULLTAG;

	char* target_revision_id = nullptr;
	CHECK_ITK(ret_code, AOM_ask_value_string(t_item_rev, "item_revision_id", &target_revision_id));
	CHECK_ITK(ret_code, ITEM_ask_item_of_rev(t_item_rev, &t_item));
	if (ret_code == ITK_ok)
	{
		auto num_rev = 0;
		tag_t* rev_list = nullptr;
		int* refresh_result = nullptr;
		CHECK_ITK(ret_code, ITEM_list_all_revs(t_item, &num_rev, &rev_list));
		CHECK_ITK(ret_code, AOM_refresh_objects_in_bulk(num_rev, rev_list, false, &refresh_result));
		if (ret_code == ITK_ok)
		{
			for (auto i = 0; i < num_rev; ++i)
			{
				tag_t* release_status = nullptr;
				char* revision_id = nullptr;
				auto num_status = 0;
				CHECK_ITK(ret_code, AOM_ask_value_tags(rev_list[i], "release_status_list", &num_status, &release_status));
				CHECK_ITK(ret_code, AOM_ask_value_string(rev_list[i], "item_revision_id", &revision_id));
				if (ret_code == ITK_ok)
				{
					for (auto j = 0; j < num_status; ++j)
					{
						char* status_name = nullptr;
						CHECK_ITK(ret_code, RELSTAT_ask_release_status_type(release_status[j], &status_name));
						/*loop all item revision to find which one has Sourcing flag*/
						if (strcmp(status_name, "VF4_Sourcing") == 0 && strcmp(target_revision_id, revision_id) != 0)
						{
							is_sourcing_existed = true;
						}
					}
				}
				SAFE_MEM_free(release_status);
				SAFE_MEM_free(revision_id);
			}
		}
		SAFE_MEM_free(rev_list);
	}
	SAFE_MEM_free(target_revision_id);
	return is_sourcing_existed;
}

/***************************************************************************************
Function     : create_sourcing_object()
Description  : get dataset by root id then read file to get bomline property
Input        : dataset name is root task id
Output       : map property
Author       : thanhpn6
******************************************************************************************/
std::map<std::string, std::map<std::string, std::string>> get_bomline_property(const std::string & dataset_name, const tag_t sor_rev)
{
	std::map<std::string, std::map<std::string, std::string>> map_prop_bomline;
	auto ret_code = ITK_ok;
	tag_t dataset = NULL_TAG;
	CHECK_ITK(ret_code, AE_find_dataset2(dataset_name.c_str(), &dataset));

	if (ret_code == ITK_ok && dataset != NULL_TAG)
	{
		CHECK_ITK(ret_code, AOM_refresh(dataset, FALSE));
		AE_reference_type_t ref_type;
		tag_t text_file = NULL_TAG;

		CHECK_ITK(ret_code, AE_ask_dataset_named_ref2(dataset, "Text", &ref_type, &text_file));
		if (ret_code != ITK_ok)
		{
			TC_write_syslog("[VF_handlers]__%d__%s__Could not get dataset file: %d\n", __LINE__, __FUNCTION__, ret_code);
		}
		if (ret_code == ITK_ok && text_file != NULL_TAG)
		{
			CHECK_ITK(ret_code, AOM_refresh(text_file, FALSE));
			IMF_file_t file_descriptor = nullptr;
			CHECK_ITK(ret_code, IMF_ask_file_descriptor(text_file, &file_descriptor));
			CHECK_ITK(ret_code, IMF_open_file(file_descriptor, SS_RDONLY));
			//char text_line[1024];
			//while (IMF_read_file_line2(file_descriptor, &text_line) == ITK_ok) //@SKIP_DEPRECATED - WILL REMOVED as fixed
			char* text_line = NULL;
			while (IMF_read_file_line2(file_descriptor, &text_line) == ITK_ok)
			{
				if (text_line)
				{
					std::string strLine(text_line);
					vector<std::string> v = split(text_line, "~~");
					if (v.size() == 7)
					{
						std::map<std::string, std::string> property_value;
						std::string uid = split(v.at(0), "=").at(1);
						for (auto iter = v.begin(); iter != v.end(); iter++)
						{
							vector<std::string> a_pair_prop = split(*iter, "=");
							if (a_pair_prop.size() == 2)
							{
								if (!a_pair_prop[0].empty() && a_pair_prop[0][a_pair_prop[0].length() - 1] == '\n') {
									a_pair_prop[0].erase(a_pair_prop[0].length() - 1);
								}
								if (!a_pair_prop[1].empty() && a_pair_prop[1][a_pair_prop[1].length() - 1] == '\n') {
									a_pair_prop[1].erase(a_pair_prop[1].length() - 1);
								}
								property_value.insert(std::pair<std::string, std::string>(a_pair_prop[0], a_pair_prop[1]));
							}
						}
						map_prop_bomline.insert(std::pair<std::string, std::map<std::string, std::string>>(uid, property_value));

					}

					SAFE_MEM_free(text_line);
				}
			}

			CHECK_ITK(ret_code, IMF_close_file(file_descriptor));
			CHECK_ITK(ret_code, AOM_unload(text_file));
			CHECK_ITK(ret_code, AOM_unload(dataset));
		}
	}
	else
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Get part information from table\n", __LINE__, __FUNCTION__);
		//1-Nov: add logic to read part information table
		tag_t* tbl_row = NULL;
		int total_row = 0;
		CHECK_ITK(ret_code, AOM_ask_table_rows(sor_rev, "vf4_SOR_part_info", &total_row, &tbl_row));
		if (total_row > 0)
		{
			TC_write_syslog("[VF_handlers]__%d__%s__There are total %d in table\n", __LINE__, __FUNCTION__, total_row);
			for (int j = 0; j < total_row; j++)
			{
				char* module_grp = NULL;
				char* module_name = NULL;
				char* main_module = NULL;
				char* pur_lvl = NULL;
				tag_t design_part = NULL;
				CHECK_ITK(ret_code, AOM_ask_value_string(tbl_row[j], "vf4_module_group", &module_grp));
				CHECK_ITK(ret_code, AOM_ask_value_string(tbl_row[j], "vf4_module_name", &module_name));
				CHECK_ITK(ret_code, AOM_ask_value_string(tbl_row[j], "vf4_main_module", &main_module));
				CHECK_ITK(ret_code, AOM_ask_value_string(tbl_row[j], "vf4_purchase_level", &pur_lvl));
				CHECK_ITK(ret_code, AOM_ask_value_tag(tbl_row[j], "vf4_design_part", &design_part));

				char* design_part_uid = nullptr;

				ITK__convert_tag_to_uid(design_part, &design_part_uid);

				std::map<std::string, std::string> property_value;
				property_value.insert(std::pair<std::string, std::string>("UID", design_part_uid));
				property_value.insert(std::pair<std::string, std::string>("VL5_change_index", "NEW"));
				property_value.insert(std::pair<std::string, std::string>("VL5_purchase_lvl_vf", pur_lvl));
				property_value.insert(std::pair<std::string, std::string>("VL5_module_group", module_grp));
				property_value.insert(std::pair<std::string, std::string>("VL5_module_name", module_name));
				property_value.insert(std::pair<std::string, std::string>("VL5_main_module", main_module));
				property_value.insert(std::pair<std::string, std::string>("bl_item_uom_tag", ""));

				map_prop_bomline.insert(std::pair<std::string, std::map<std::string, std::string>>(design_part_uid, property_value));

				SAFE_MEM_free(module_grp);
				SAFE_MEM_free(module_name);
				SAFE_MEM_free(main_module);
				SAFE_MEM_free(pur_lvl);
				SAFE_MEM_free(design_part_uid);
			}
		}
	}
	return map_prop_bomline;
}

vector<string> split(const std::string & s, const std::string & delimiter) {
	size_t pos_start = 0;
	size_t pos_end;
	const size_t delimiter_len = delimiter.length();
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		std::string token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delimiter_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

/***************************************************************************************
Function     : get_cl_display_value()
Description  : get cl value by format user_name (user id)
Input        : tag_t user
Output       : string cl value
Author       : thanhpn6
******************************************************************************************/
std::string get_cl_display_value(tag_t user)
{
	if (user == NULL_TAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Could not get display value of CL because user null\n", __LINE__, __FUNCTION__);
		return "";
	}
	int ret_code = ITK_ok;
	char* os_user_name = nullptr;
	std::string out_put;

	if (ret_code == ITK_ok)
	{
		tag_t person_tag = NULL_TAG;
		char* person_name = nullptr;

		CHECK_ITK(ret_code, SA_ask_user_identifier2(user, &os_user_name));
		CHECK_ITK(ret_code, SA_ask_user_person(user, &person_tag));
		if (ret_code == ITK_ok)
		{
			CHECK_ITK(ret_code, SA_ask_person_name2(person_tag, &person_name));
			if (ret_code == ITK_ok)
			{
				out_put = std::string(person_name) + " (" + std::string(os_user_name) + ")";
			}
		}
		SAFE_MEM_free(person_name);
	}
	return out_put;
}


/***************************************************************************************
Function     : change_ownership()
Description  : change ownership of sourcing part to CL
Input        : tag_t sourcing part
Output       : void
Author       : thanhpn6
******************************************************************************************/
void change_ownership(tag_t sourcing_item_rev, tag_t sourcing_item, tag_t tCLGrpMem)
{
	if (sourcing_item_rev == NULLTAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Could not change ownership to cl because sourcing_item_rev null\n", __LINE__, __FUNCTION__);
		return;
	}
	if (sourcing_item == NULLTAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Could not change ownership to cl because sourcing_item null\n", __LINE__, __FUNCTION__);
		return;
	}

	if (tCLGrpMem == NULLTAG)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Could not change ownership to cl because user_cl null\n", __LINE__, __FUNCTION__);
		return;
	}

	int ret_code = ITK_ok;
	tag_t	item_rev_class = NULLTAG;
	tag_t	sourcing_rev_class = NULLTAG;
	logical is_descendant = false;
	tag_t tUser = NULLTAG;
	tag_t tGroup = NULLTAG;

	CHECK_ITK(ret_code, POM_class_of_instance(sourcing_item_rev, &sourcing_rev_class));
	CHECK_ITK(ret_code, POM_class_id_of_class("ItemRevision", &item_rev_class));
	CHECK_ITK(ret_code, POM_is_descendant(item_rev_class, sourcing_rev_class, &is_descendant));

	CHECK_ITK(ret_code, SA_ask_groupmember_user(tCLGrpMem, &tUser));
	CHECK_ITK(ret_code, SA_ask_groupmember_group(tCLGrpMem, &tGroup));

	/*check for itemrevision*/
	if (is_descendant == true)
	{
		int     total_master_form = 0;
		tag_t	form_relation = NULL;
		tag_t* item_mform = nullptr;
		tag_t* item_rev_mform = nullptr;
		vector<tag_t> objs_tag;
		CHECK_ITK(ret_code, GRM_find_relation_type("IMAN_master_form", &form_relation));
		CHECK_ITK(ret_code, GRM_list_secondary_objects_only(sourcing_item, form_relation, &total_master_form, &item_mform));
		CHECK_ITK(ret_code, GRM_list_secondary_objects_only(sourcing_item_rev, form_relation, &total_master_form, &item_rev_mform));

		objs_tag.push_back(sourcing_item);
		objs_tag.push_back(sourcing_item_rev);
		/*objs_tag.push_back(item_mform[0]);
		objs_tag.push_back(item_rev_mform[0]);*/

		for (auto i = 0; i < objs_tag.size(); ++i)
		{
			logical is_granted = false;
			char* item_id = nullptr;
			CHECK_ITK(ret_code, ITEM_ask_id2(sourcing_item, &item_id));
			CHECK_ITK(ret_code, AM_check_privilege(objs_tag[i], "WRITE", &is_granted));
			if (!is_granted)
			{
				TC_write_syslog("[VF_handlers]__%d__%s__No WRITE access: %s\n", __LINE__, __FUNCTION__, item_id);
				continue;
				//grant_access(objs_tag[i]);
			}
			CHECK_ITK(ret_code, AOM_refresh(objs_tag[i], true));
			CHECK_ITK(ret_code, AOM_set_ownership(objs_tag[i], tUser, tGroup));
			CHECK_ITK(ret_code, AOM_save_with_extensions(objs_tag[i]));
			CHECK_ITK(ret_code, AOM_refresh(objs_tag[i], false));
			if (ret_code != ITK_ok)
			{
				TC_write_syslog("[VF_handlers]__%d__%s__Failed to change ownership for Item %s__%d\n", __LINE__, __FUNCTION__, item_id, ret_code);
			}
			SAFE_MEM_free(item_id);
		}
	}
}

/***************************************************************************************
Function     : grant_access()
Description  : grant access for object
Input        : tag_t object
Output       : int
Author       : thanhpn6
******************************************************************************************/
int grant_access(tag_t object_tag)
{
	tag_t   world_tag = NULLTAG;
	tag_t   write_prev = NULLTAG;
	tag_t   change_ownership_prev = NULL_TAG;
	int ret_code = AOM_refresh(object_tag, POM_modify_lock);
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Grant access failed: %d\n", __LINE__, __FUNCTION__, ret_code);
		return ret_code;
	}

	ret_code = AM_find_named_tag("World", &world_tag);
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Grant access failed: %d\n", __LINE__, __FUNCTION__, ret_code);
		AOM_unlock(object_tag);
		return ret_code;
	}
	ret_code = AM_find_privilege("WRITE", &write_prev);
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Grant access failed: %d\n", __LINE__, __FUNCTION__, ret_code);
		AOM_unlock(object_tag);
		return ret_code;
	}
	ret_code = AM_find_privilege("CHANGE_OWNER", &change_ownership_prev);
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Grant access failed: %d\n", __LINE__, __FUNCTION__, ret_code);
		AOM_unlock(object_tag);
		return ret_code;
	}
	ret_code = AM_grant_privilege(object_tag, world_tag, write_prev);
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Grant access failed: %d\n", __LINE__, __FUNCTION__, ret_code);
		AOM_unlock(object_tag);
		return ret_code;
	}
	ret_code = AM_grant_privilege(object_tag, world_tag, change_ownership_prev);
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Grant access failed: %d\n", __LINE__, __FUNCTION__, ret_code);
		AOM_unlock(object_tag);
		return ret_code;
	}
	ret_code = AM_save_acl(object_tag);
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Grant access failed: %d\n", __LINE__, __FUNCTION__, ret_code);
		AOM_unlock(object_tag);
		return ret_code;
	}

	ret_code = AE_save_myself(object_tag);
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Grant access failed: %d\n", __LINE__, __FUNCTION__, ret_code);
		AOM_unlock(object_tag);
		return ret_code;
	}
	return ret_code;
}

int query_sourcing_part(char* part_id, char* sourcing_program)
{
	tag_t query_object = NULLTAG;
	auto ret_code = ITK_ok;
	CHECK_ITK(ret_code, QRY_find2("Source Part Item", &query_object));
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Cannot find query [Source Part Item]: %d\n", __LINE__, __FUNCTION__, ret_code);
		return -1;
	}

	const int n_entries = 2;
	char* entries[2] = { "VF Part Number", "Sourcing Program" };
	char** values = nullptr;

	values = (char**)MEM_alloc(n_entries * sizeof(char*));

	values[0] = (char*)MEM_alloc(strlen(part_id) + 1);
	tc_strcpy(values[0], part_id);

	values[1] = (char*)MEM_alloc(strlen(sourcing_program) + 1);
	tc_strcpy(values[1], sourcing_program);

	auto num_found = 0;
	tag_t * total_results = nullptr;

	CHECK_ITK(ret_code, QRY_execute(query_object, n_entries, entries, values, &num_found, &total_results));
	if (ret_code != ITK_ok)
	{
		TC_write_syslog("[VF_handlers]__%d__%s__Cannot find sourcing part: %s|%s\n", __LINE__, __FUNCTION__, part_id, sourcing_program);
		SAFE_MEM_free(total_results);
		return -1;
	}
	else
	{
		if (num_found > 0)
		{
			TC_write_syslog("[VF_handlers]__%d__%s__Sourcing part existed: %s|%s\n", __LINE__, __FUNCTION__, part_id, sourcing_program);
		}
		SAFE_MEM_free(total_results);
		return num_found;
	}
}

boolean is_valid_purchase_level(const char* purchase_lvl, std::vector<std::string> valid_purchase)
{
	if (std::find(valid_purchase.begin(), valid_purchase.end(), purchase_lvl) != valid_purchase.end())
	{
		return true;
	}
	return false;
}