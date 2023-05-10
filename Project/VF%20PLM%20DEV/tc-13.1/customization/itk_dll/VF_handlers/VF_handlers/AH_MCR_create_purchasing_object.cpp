/***************************************************************************************
Function     : AH_MCR_create_purchasing_object()
Description  : call mcr_create_sourcing_object() for each target object in root task
Input        : None
Output       : None
Author       : thanhpn6
******************************************************************************************/
#include "Vinfast_Custom.h"

//int mcr_create_sourcing_object(const char* item_id, const char* object_name, const char* sourcing_obj_type, const char* car_program, std::string cl_user, tag_t& sourcing_rev, const char* default_change_index, const char* default_purchase_level, const char* object_type);
int mcr_create_sourcing_object(const char* item_id, const char* object_name, const char* sourcing_obj_type, const char* car_program, const std::string& cl_user, tag_t& sourcing_rev, const char* change_index, const char* purchase_lvl, const char* part_obj_type);
int mcr_create_cost_object(const char* item_id, const char* object_name, tag_t* cl_user, tag_t* cl_group, tag_t& cost_rev);
std::string mcr_get_cl_user_name(tag_t root_task, tag_t &cl_user, tag_t &cl_group, string &cl_mail, const char* default_change_index);
bool mcr_isSourcingExisted(tag_t t_item_rev);
std::map<std::string, std::map<std::string, std::string>> mcr_getBOMLineProperty(std::string datasetName);
void sendMail(const vector<string> &toMails, const string &subject, const string &body);


//vector<string> split (const std::string& s, const std::string& delimiter) {
//	size_t pos_start = 0;
//	size_t pos_end;
//	const size_t delimiter_len = delimiter.length();
//	std::vector<std::string> res;
//
//	while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
//		std::string token = s.substr (pos_start, pos_end - pos_start);
//		pos_start = pos_end + delimiter_len;
//		res.push_back (token);
//	}
//
//	res.push_back (s.substr (pos_start));
//	return res;
//}

extern int AH_MCR_create_purchasing_object(EPM_action_message_t msg)
{
	int   ret_code            = ITK_ok;
	tag_t root_task           = NULLTAG;
	tag_t sourcing_rev        = NULLTAG;
	tag_t cost_rev            = NULLTAG;
	tag_t cl_user             = NULLTAG;
	tag_t cl_group			  = NULLTAG;
	string cl_mail;
	int num_args			  = 0;
	char log_message [2048]   = "";
	char *default_purchase_level		  = NULL;
	char *default_change_index = NULL;
	char *arg_key			  = NULL;	
	char *arg_value	          = NULL;
	
	char *cl_review_task_name = NULL;
	std::string os_user_name  = "";
	
	/*getting argument*/

	TC_write_syslog("\n[VF_handlers] ENTER function %s", __FUNCTION__);

	/*read mapping sourcing program to object type*/
	std::map<std::string, std::string> prg_2_objType;
	char **arr_prg_2objType = NULL;
	int count = 0;

	CHECK_ITK(ret_code, PREF_ask_char_values("VF_SOURCING_PROGRAM_2_OBJECT_TYPE", &count, &arr_prg_2objType));
	for(int i = 0; i < count; i++)
	{
		std::string one_value = arr_prg_2objType[i];
		vector<std::string> v = split_string2(one_value, ";");
		prg_2_objType.insert(std::pair<std::string, std::string>(v[0],v[1]));
	}

	SAFE_MEM_free(arr_prg_2objType);
	num_args = TC_number_of_arguments(msg.arguments);
	if (num_args > 0)
	{
		for(int i = 0; i < num_args; ++i)
		{
			CHECK_ITK(ret_code, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &arg_key, &arg_value));
			if (ret_code == ITK_ok )
			{
				if (tc_strcasecmp(arg_key, "default_purchase_level") == 0 && arg_value != NULL)
				{
					default_purchase_level = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(default_purchase_level, arg_value);
				}
				if(tc_strcasecmp(arg_key, "default_change_index") == 0 && arg_value != NULL)
				{
					default_change_index = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(default_change_index, arg_value);
				}
				if(tc_strcasecmp(arg_key, "ask_cl_from_task") == 0 && arg_value != NULL)
				{
					cl_review_task_name = (char*) MEM_alloc(( (int)tc_strlen(arg_value) + 1) * sizeof(char));
					tc_strcpy(cl_review_task_name, arg_value);
				}
			}
			SAFE_MEM_free(arg_key);
			SAFE_MEM_free(arg_value);
		}
	}

	/*getting root task*/
	CHECK_ITK(ret_code, EPM_ask_root_task(msg.task, &root_task));

	if(root_task !=  NULLTAG)
	{
		/*get map bomline property*/
		char* rootTaskUID = NULL;

		//ITK__convert_tag_to_uid(root_task, &rootTaskUID);
		//std::map<std::string, std::map<std::string, std::string>> bomLineProperty;
		//if(rootTaskUID != NULL)
		//{
		//	if((default_purchase_level == NULL || default_purchase_level[0] == '\0') && (default_change_index == NULL || default_change_index[0] == '\0'))
		//	{
		//		bomLineProperty = mcr_getBOMLineProperty(std::string(rootTaskUID));
		//		//bomLineProperty = mcr_getBOMLineProperty(std::string("testing18042020"));
		//	}
		//}

		/*get user and group to change ownership*/
		if(cl_review_task_name != NULL)
		{
			os_user_name = mcr_get_cl_user_name(root_task, cl_user, cl_group, cl_mail, cl_review_task_name);
		}
		int	  total_target_obj   = 0;
		tag_t *target_obj = NULLTAG;

		/*getting target objects*/
		CHECK_ITK(ret_code, EPM_ask_attachments(root_task, EPM_target_attachment, &total_target_obj, &target_obj));
		for(int i = 0; i < total_target_obj; ++i)
		{
			char* target_item_id   = NULL;
			char* target_object_type = NULL;
			char* car_program = NULL;
			char *sourcing_obj_type   = NULL;
			CHECK_ITK(ret_code, WSOM_ask_id_string(target_obj[i], &target_item_id));
			CHECK_ITK(ret_code, WSOM_ask_object_type2(target_obj[i], &target_object_type));
			if(tc_strcasecmp(target_object_type, "VF4_MCRRevision") == 0)
			{
				CHECK_ITK(ret_code, AOM_ask_value_string(target_obj[i], "vf4_car_program", &car_program));

				if(prg_2_objType.find(car_program) != prg_2_objType.end())
				{
					sourcing_obj_type = (char*) MEM_alloc(( (int)tc_strlen(prg_2_objType.find(car_program)->second.c_str()) + 1) * sizeof(char));
					tc_strcpy(sourcing_obj_type, prg_2_objType.find(car_program)->second.c_str());
				}
				else
				{
					TC_write_syslog("[VF_handlers]__%d__%s__Please add sourcing object type into VF_SOURCING_PROGRAM_2_OBJECT_TYPE for program: %s\n", __LINE__, __FUNCTION__, car_program);
					return SOURCING_INVALID_OBJ_TYPE;
				}

				/*if(tc_strcasecmp(car_program, "S&S") == 0 || tc_strcasecmp(car_program, "V8") == 0)
				{
					sourcing_obj_type = "VF4_line_item";
				}
				else if(tc_strcasecmp(car_program, "EBUS") == 0)
				{
					sourcing_obj_type = "VF4_Bus_LineItm";
				}
				else if(tc_strcasecmp(car_program, "C-CUV") == 0)
				{
					sourcing_obj_type = "VF4_Car_LineItm";
				}
				else if(tc_strcasecmp(car_program, "SCP") == 0)
				{
					sourcing_obj_type = "VF4_scp_line_itm";
				}
				else if(tc_strcasecmp(car_program, "Escooter") == 0)
				{
					sourcing_obj_type = "VF4_ES_LineItm";
				}
				else if(tc_strcasecmp(car_program, "Battery") == 0)
				{
					sourcing_obj_type = "VF4_BatteryLine";
				}
				else
				{
					TC_write_syslog("\n[VF_handlers]__%d__%s__Error Invalid Car Program: %s", __LINE__, __FUNCTION__, car_program);
				}*/

					tag_t proposal_relation = NULLTAG;
					int count = 0;
					tag_t *second_obj = NULL;

					int nRows = -1;
					tag_t *rows = NULL;
					CHECK_ITK(ret_code, AOM_ask_table_rows(target_obj[i], "vf4_bom_attributes", &nRows, &rows));
					vector<string> purchaseLevels, changeIndexes;
					vector<tag_t> parts;

					for (int rowInx = 0; rowInx < nRows; rowInx++)
					{
						tag_t part;
						char *purchaseLevel;
						char *changeIndex;

						CHECK_ITK(ret_code, AOM_ask_value_tag(rows[rowInx], "vf4_part_revision", &part));
						CHECK_ITK(ret_code, AOM_ask_value_string(rows[rowInx], "vf4_purchase_level", &purchaseLevel));
						CHECK_ITK(ret_code, AOM_ask_value_string(rows[rowInx], "vf4_change_index", &changeIndex));

						purchaseLevels.push_back(purchaseLevel);
						changeIndexes.push_back(changeIndex);
						parts.push_back(part);

						char *partID = NULL;
						CHECK_ITK(ret_code, AOM_ask_value_string(part, "item_id", &partID));

						SAFE_SM_FREE(partID);
						SAFE_SM_FREE(purchaseLevel);
						SAFE_SM_FREE(changeIndex);
					}

					SAFE_SM_FREE(rows);

					count = parts.size();
					for(int j = 0; j < count; j++)
					{
						if (purchaseLevels[j].compare("P") != 0 && purchaseLevels[j].compare("BL") != 0 && purchaseLevels[j].compare("DPS") != 0 && purchaseLevels[j].compare("DPT") != 0) 
						{
							continue;
						}

						char *pro_item_id	= NULL;
						char *pro_object_type	= NULL;
						char *pro_object_name = NULL;
						CHECK_ITK(ret_code, WSOM_ask_object_type2(parts[j], &pro_object_type));
						//CHECK_ITK(ret_code, WSOM_ask_name2(target_obj[i], &pro_object_name));
						CHECK_ITK(ret_code, WSOM_ask_name2(parts[j], &pro_object_name));
						CHECK_ITK(ret_code, WSOM_ask_id_string(parts[j], &pro_item_id));
						std::vector<string> o = split_string(pro_item_id, '/');
						/*creating sourcing object*/
						if(tc_strcasecmp(pro_object_type, "VF3_manuf_partRevision") == 0)
						{
							TC_write_syslog("\n[VF_handlers]__%d__%s__Start create sourcing and cost item for part: %s-%s", __LINE__, __FUNCTION__, pro_item_id, pro_object_name);
							ret_code = mcr_create_sourcing_object(o.at(0).c_str(), pro_object_name, sourcing_obj_type, car_program, os_user_name, sourcing_rev, changeIndexes[j].c_str(), purchaseLevels[j].c_str(), pro_object_type);
							if(ret_code != ITK_ok)
							{
								TC_write_syslog("\n[VF_handlers]__%d__%s__Error when create sourcing object: %d", __LINE__, __FUNCTION__, ret_code);
							}

							if (sourcing_rev != NULLTAG)
							{
								/*creating relation between part revision and sourcing revision*/
								TC_write_syslog("\n @@@@@@@@@@@ [NN] attaching created source part @@@@@@@\n");
								tag_t sourcing_rela_type      = NULLTAG;
								tag_t sourcing_rela_created   = NULLTAG;
								CHECK_ITK(ret_code, GRM_find_relation_type ("VF4_Sourcing_Reference", &sourcing_rela_type));
								CHECK_ITK(ret_code, AOM_refresh (parts[j], FALSE));
								CHECK_ITK(ret_code, GRM_create_relation(parts[j], sourcing_rev, sourcing_rela_type, NULLTAG, &sourcing_rela_created));
								TC_write_syslog("\n @@@@@@@@@@@ [NN] creating relation\n");
								if(ret_code == ITK_ok)
								{
									TC_write_syslog("\n @@@@@@@@@@@ [NN] created relation\n");
									/*save relation*/
									CHECK_ITK(ret_code, GRM_save_relation (sourcing_rela_created));
								}
							}

							if (cl_user != NULLTAG && cl_group != NULLTAG && sourcing_rev != NULLTAG)
							{
								// TODO: add one more parameter to enable change ownership
								CHECK_ITK(ret_code, AOM_set_ownership(sourcing_rev, cl_user, cl_group));
							}
						}

						SAFE_MEM_free(pro_item_id);
						SAFE_MEM_free(pro_object_type);
						SAFE_MEM_free(pro_object_name);
					}
				SAFE_MEM_free(second_obj);
			}
			SAFE_MEM_free(target_item_id);
			SAFE_MEM_free(target_object_type);
			SAFE_MEM_free(car_program);
			SAFE_MEM_free(sourcing_obj_type);

		}
	}
	SAFE_MEM_free(default_purchase_level);
	SAFE_MEM_free(default_change_index);
	return ret_code;
}

/***************************************************************************************
Function     : mcr_create_sourcing_object()
Description  : call mcr_create_sourcing_object() for each target object in root task
Input        : None
Output       : None
Author       : thanhpn6
******************************************************************************************/

int mcr_create_sourcing_object(const char* item_id, const char* object_name, const char* sourcing_obj_type, const char* car_program, const std::string& cl_user, tag_t& sourcing_rev, const char* change_index, const char* purchase_lvl, const char* part_obj_type)
{
	int   ret_code                 = ITK_ok;
	tag_t item_type_tag            = NULLTAG;
	tag_t itemRev_type_tag         = NULLTAG;
	tag_t item_create_input_tag    = NULLTAG;
	tag_t itemRev_create_input_tag = NULLTAG;
	tag_t item_tag                 = NULLTAG;
	
	/*creating item*/
		CHECK_ITK(ret_code, TCTYPE_find_type(sourcing_obj_type,  sourcing_obj_type, &item_type_tag))
		if(item_type_tag == NULLTAG)
		{
			TC_write_syslog("[VF]__%d__%s__Could not find tag of object type: %s\n", __LINE__, __FUNCTION__, sourcing_obj_type);
		}
		CHECK_ITK(ret_code, TCTYPE_construct_create_input (item_type_tag, &item_create_input_tag))
		
		/*find item revision tag type*/
		const std::string sourcing_partRev_object_type = std::string(sourcing_obj_type).append("Revision");
		CHECK_ITK(ret_code, TCTYPE_find_type(sourcing_partRev_object_type.c_str(),  sourcing_partRev_object_type.c_str(), &itemRev_type_tag))
		if(itemRev_type_tag == NULLTAG)
		{
			TC_write_syslog("[VF]__%d__%s__Could not find tag of object type: %s\n", __LINE__, __FUNCTION__, sourcing_partRev_object_type);
		}
		const char* tmp_cl = cl_user.c_str();
		CHECK_ITK(ret_code, TCTYPE_construct_create_input(itemRev_type_tag,&itemRev_create_input_tag))
		CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "object_name", 1, &object_name))
		CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_bom_vfPartNumber", 1, &item_id))
		CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_platform_module", 1, &car_program))
		CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_cl", 1, &tmp_cl))
		CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_car_part_bo_type", 1, &part_obj_type))
		CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_purchasing_level", 1, &purchase_lvl))
		CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_bom_change_index", 1, &change_index))
		
		CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "object_name", 1, &object_name))
		CHECK_ITK(ret_code, AOM_set_value_tag(item_create_input_tag, "revision", itemRev_create_input_tag))
		CHECK_ITK(ret_code, TCTYPE_create_object(item_create_input_tag, &item_tag))
		CHECK_ITK(ret_code, AOM_save_with_extensions(item_tag))

		tag_t revTag = NULLTAG;
		CHECK_ITK(ret_code, ITEM_ask_latest_rev(item_tag, &revTag))
		if (revTag != NULLTAG) sourcing_rev = revTag;

	return ret_code;
}

//int mcr_create_sourcing_object(const char* item_id, const char* object_name, const char* sourcing_obj_type, const char* car_program, std::string cl_user, tag_t& sourcing_rev,
//						   const char* default_change_index, const char* default_purchase_level, const char* part_obj_type)
//{
//	int   ret_code                 = ITK_ok;
//	tag_t item_type_tag            = NULLTAG;
//	tag_t itemRev_type_tag         = NULLTAG;
//	tag_t item_create_input_tag    = NULLTAG;
//	tag_t itemRev_create_input_tag = NULLTAG;
//	tag_t item_tag                 = NULLTAG;
//	tag_t user_tag                 = NULLTAG;
//	char  log_message [2048]       = "";
//
//	sourcing_rev = NULLTAG;
//
//	/*creating item*/
//	CHECK_ITK(ret_code, TCTYPE_find_type(sourcing_obj_type,  sourcing_obj_type, &item_type_tag));
//	if(item_type_tag == NULLTAG)
//	{
//		TC_write_syslog("\n[VF_handlers] Could not find tag of object type %s.", sourcing_obj_type);
//	}
//	CHECK_ITK(ret_code, TCTYPE_construct_create_input (item_type_tag, &item_create_input_tag));
//	
//
//	/*find item revision tag type*/
//	std::string sourcingRevObjType = std::string(sourcing_obj_type).append("Revision");
//	CHECK_ITK(ret_code, TCTYPE_find_type(sourcingRevObjType.c_str(),  sourcingRevObjType.c_str(), &itemRev_type_tag));
//	if(itemRev_type_tag == NULLTAG)
//	{
//		TC_write_syslog("\n[VF_handlers] Could not find tag of object type %s.", sourcingRevObjType);
//	}
//	const char* tmpCL = cl_user.c_str();
//	CHECK_ITK(ret_code, TCTYPE_construct_create_input(itemRev_type_tag,&itemRev_create_input_tag));
//	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "object_name", 1, &object_name));
//	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_bom_vfPartNumber", 1, &item_id));
//	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_platform_module", 1, &car_program));
//	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_cl", 1, &tmpCL));
//	//CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_car_program", 1, &part_obj_type));
//	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_purchasing_level", 1, &default_purchase_level));
//	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(itemRev_create_input_tag, "vf4_bom_change_index", 1, &default_change_index));
//	
//	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "object_name", 1, &object_name));
//	CHECK_ITK(ret_code, AOM_set_value_tag(item_create_input_tag, "revision", itemRev_create_input_tag));
//	CHECK_ITK(ret_code, TCTYPE_create_object(item_create_input_tag, &item_tag));
//	CHECK_ITK(ret_code, AOM_save_with_extensions(item_tag));
//	/*getting lastest revision of item*/
//	if(item_tag != NULLTAG)
//	{
//		CHECK_ITK(ret_code, ITEM_ask_latest_rev(item_tag, &sourcing_rev));
//	}
//
//	return 0;
//
//}

std::string mcr_get_cl_user_name(tag_t root_task, tag_t &cl_user, tag_t &cl_group, string &cl_mail, const char* cl_review_task_name)
{
	int    total_task = 0;
	int    ret_code = ITK_ok;
	tag_t* sub_task = NULL;
	char*  os_user_name = NULL;
	std::string out_put = "";

	CHECK_ITK(ret_code, EPM_ask_sub_tasks(root_task, &total_task, &sub_task));
	for(int i = 0; i < total_task; ++i)
	{
		char* task_name = NULL;
		char* task_type = NULL;

		CHECK_ITK(ret_code, AOM_ask_value_string(sub_task[i],"object_name", &task_name));
		CHECK_ITK(ret_code, AOM_ask_value_string(sub_task[i],"task_type", &task_type));
		if (tc_strcmp(task_name, cl_review_task_name) == 0 && tc_strcmp(task_type, "EPMReviewTask") == 0) 
		{
			int		total_child_task = 0;
			tag_t	*child_task		 = NULL;
			CHECK_ITK(ret_code, EPM_ask_sub_tasks(sub_task[i], &total_child_task, &child_task));
			for(int j = 0 ; j < total_child_task; j++)
			{
				char*   child_task_type  = NULL;
				CHECK_ITK(ret_code, AOM_ask_value_string(child_task[j], "task_type", &child_task_type));
				if(tc_strcmp(child_task_type, "EPMSelectSignoffTask") == 0)
				{
					int		total_att			= 0;
					tag_t	*att_tag_list		= NULLTAG;
					tag_t   group_member        = NULLTAG;

					//get signoff attachments
					CHECK_ITK(ret_code, EPM_ask_attachments(child_task[j], EPM_signoff_attachment, &total_att, &att_tag_list));
					if (total_att  > 0)
					{
						CHECK_ITK(ret_code, AOM_ask_value_tag(att_tag_list[0],"fnd0Performer", &cl_user));

						if(ret_code == ITK_ok && cl_user != NULLTAG)
						{
							CHECK_ITK(ret_code, AOM_ask_value_tag(att_tag_list[0],"group_member", &group_member));
							CHECK_ITK(ret_code, AOM_ask_value_tag(group_member,"group", &cl_group));
							tag_t tPerson = NULL_TAG;
							char* personName = NULL;
							char *personMail = NULL;

							CHECK_ITK(ret_code, SA_ask_user_identifier2(cl_user, &os_user_name));
							CHECK_ITK(ret_code, SA_ask_user_person(cl_user, &tPerson));
							if(ret_code == ITK_ok)
							{
								CHECK_ITK(ret_code, SA_ask_person_name2(tPerson, &personName));
								char *szCLMail = NULL;
								CHECK_ITK(ret_code, SA_ask_person_attr2(tPerson, "PA9", &szCLMail));
								cl_mail = szCLMail;
								if(ret_code == ITK_ok)
								{
									out_put = std::string(personName) + " (" + std::string(os_user_name) + ")";
								}
								SAFE_SM_FREE(szCLMail);
							}
							SAFE_MEM_free(personName);
						}
					}
					
					SAFE_MEM_free(att_tag_list);
				}
				SAFE_MEM_free(child_task_type);
			}
		}

		SAFE_MEM_free(task_name);
		SAFE_MEM_free(task_type);
		SAFE_MEM_free(os_user_name);
	}
	SAFE_MEM_free(sub_task);

	return out_put;
}

bool mcr_isSourcingExisted(tag_t t_item_rev)
{
	bool mcr_isSourcingExisted = false;
	bool isAttached2NewItem = false;
	int    ret_code	= ITK_ok;
	tag_t t_item	= NULLTAG;
	char* target_revision_id = NULL;
	CHECK_ITK(ret_code, AOM_ask_value_string(t_item_rev, "item_revision_id", &target_revision_id));
	CHECK_ITK(ret_code, ITEM_ask_item_of_rev(t_item_rev , &t_item));
	if(ret_code == ITK_ok)
	{
		int num_rev		= 0;
		tag_t* rev_list	= NULL;
		CHECK_ITK(ret_code, ITEM_list_all_revs(t_item, &num_rev, &rev_list));

		if(ret_code == ITK_ok)
		{
			for(int i = 0; i < num_rev; ++i)
			{
				tag_t *release_status	= NULL;
				char *revision_id		= NULL;
				int num_status				= 0;
				CHECK_ITK(ret_code, AOM_ask_value_tags(rev_list[i], "release_status_list", &num_status, &release_status));
				CHECK_ITK(ret_code, AOM_ask_value_string(rev_list[i], "item_revision_id", &revision_id));
				if(ret_code == ITK_ok)
				{
					for(int j = 0; j < num_status; ++j)
					{
						char* status_name = NULL;
						CHECK_ITK(ret_code, RELSTAT_ask_release_status_type(release_status[j], &status_name));
						/*loop all item revision to find which one has Sourcing flag*/
						if(strcmp(status_name, "VF4_Sourcing") == 0 && strcmp(target_revision_id, revision_id) != 0)
						{
							mcr_isSourcingExisted = true;
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
	return mcr_isSourcingExisted;
}

/***************************************************************************************
Function     : mcr_create_sourcing_object()
Description  : get dataset by root id then read file to get bomline property
Input        : dataset name is root task id
Output       : map property
Author       : thanhpn6
******************************************************************************************/
std::map<std::string, std::map<std::string, std::string>> mcr_getBOMLineProperty(std::string datasetName)
{
	std::map<std::string, std::map<std::string, std::string>> bomlineProperty;
	int retCode	= ITK_ok;
	tag_t dataset = NULL_TAG;
	CHECK_ITK(retCode, AE_find_dataset2(datasetName.c_str(), &dataset));

	if(retCode == ITK_ok && dataset != NULL_TAG)
	{
		CHECK_ITK(retCode, AOM_refresh(dataset, TRUE));
		AE_reference_type_t ref_type;
		tag_t textFile = NULL_TAG;

		CHECK_ITK(retCode, AE_ask_dataset_named_ref2(dataset, "Text", &ref_type, &textFile));
		if(retCode == ITK_ok && textFile != NULL_TAG)
		{
			CHECK_ITK(retCode, AOM_refresh(textFile, TRUE));
			IMF_file_t fileDescriptor = NULL;
			CHECK_ITK(retCode, IMF_ask_file_descriptor(textFile, &fileDescriptor));
			CHECK_ITK(retCode, IMF_open_file(fileDescriptor, SS_RDONLY));
			char textLine[SS_MAXLLEN + 1];
			//while (IMF_read_file_line(fileDescriptor, textLine) == ITK_ok)//@SKIP_DEPRECATED
			{
				std::string strLine(textLine);
				vector<std::string> v = split_string2(textLine, "~~");
				if(v.size() == 3)
				{
					std::map<std::string, std::string> propertyValue;
					std::string uid = split_string2(v.at(0), "=").at(1);
					for(auto iter = v.begin(); iter != v.end(); iter++)
					{
						vector<std::string> pairProperty = split_string2(*iter, "=");
						if(pairProperty.size() == 2)
						{
							propertyValue.insert(std::pair<std::string, std::string>(pairProperty[0],pairProperty[1]));
						}
					}
					bomlineProperty.insert(std::pair<std::string, std::map<std::string, std::string>>(uid, propertyValue));

				}
			}

			CHECK_ITK(retCode, IMF_close_file(fileDescriptor));
			CHECK_ITK(retCode, AOM_unload(textFile));
			CHECK_ITK(retCode, AOM_unload(dataset));
		}

	}

	TC_write_syslog("\n[VF_handlers] LEAVE function %s", __FUNCTION__);
	return bomlineProperty;
}

void sendMail(const vector<string> &toMails, const string &subject, const string &body)
{
	char *pcMailAddress = USER_new_file_name("cr_notify_dset","TEXT","txt",1);
	char *pcMailBodyFileName = pcMailBodyFileName = USER_new_file_name("cr_notify_dset","TEXT","html",1);
	char caMailDetails[BUFSIZ +1];
	char		*portNo = NULL;
	int iRetCode = ITK_ok;
	int iPrefCount = -1, portPref_count = -1;
	char *pcServerName = NULL;

	caMailDetails[0] = '\0';
	#if defined(WNT)
		tc_strcpy(caMailDetails, "%TC_BIN%\\tc_mail_smtp ");
	#else
		tc_strcpy(caMailDetails, "$TC_BIN/tc_mail_smtp ");
	#endif

	

	tc_strcat( caMailDetails, "-to_list_file=\"" );
	tc_strcat( caMailDetails, pcMailAddress );
	tc_strcat( caMailDetails, "\" " );

	/*getting server name*/
	if(iRetCode == ITK_ok)
	{
		//CHECK_ITK(iRetCode,PREF_set_search_scope( TC_preference_site ));//@SKIP_DEPRECATED - WILL REMOVED
	}
	if(iRetCode == ITK_ok)
	{
		CHECK_ITK(iRetCode,PREF_ask_value_count( "Mail_server_name", &iPrefCount ));
	}

	if((iRetCode == ITK_ok) && iPrefCount != 0 )
	{
		CHECK_ITK(iRetCode,PREF_ask_char_value("Mail_server_name", 0, &pcServerName ));
		if(iRetCode == ITK_ok)
		{
			tc_strcat( caMailDetails, "-server=\"" );
			tc_strcat(caMailDetails, pcServerName );
			tc_strcat(caMailDetails, "\" " );
		}
		SAFE_MEM_free(pcServerName);
	}

	/*getting port number*/
	if( iRetCode == ITK_ok )
	{
		CHECK_ITK(iRetCode,PREF_ask_value_count( "Mail_server_port", &portPref_count ));
	}

	if( (iRetCode == ITK_ok) && portPref_count != 0 )
	{
		CHECK_ITK(iRetCode,PREF_ask_char_value( "Mail_server_port", 0, &portNo ));
		if(iRetCode == ITK_ok)
		{
			tc_strcat( caMailDetails, "-port=\"" );
			tc_strcat(caMailDetails, portNo );
			tc_strcat(caMailDetails, "\" " );
		}
		SAFE_MEM_free(portNo);
	}

	if(iRetCode == ITK_ok)
	{
		tc_strcat( caMailDetails, " -user=\"" );
		tc_strcat(caMailDetails, "PLMNotification@vinfast.vn" );
		tc_strcat(caMailDetails, "\"" );
	}

	/*get subject*/
	tc_strcat( caMailDetails, " -subject=\"" );
	tc_strcat( caMailDetails, subject.c_str() );
	tc_strcat( caMailDetails, "\"" );

	tc_strcat( caMailDetails, " -body=\"" );
	tc_strcat(caMailDetails, pcMailBodyFileName);
	tc_strcat(caMailDetails, "\"" );
	
	// write to mail address to pcMailAddress
	FILE *fMailAddress = fopen(pcMailAddress, "w");
	if (fMailAddress != NULL)
	{
		for (const auto toMail : toMails)
		{
			fprintf(fMailAddress, "%s\n",toMail.c_str());
		}
		fclose(fMailAddress);	
	}

	// write mail body to pcMailBodyFileName
	FILE *fMailBody = fopen(pcMailBodyFileName, "w");
	if (fMailBody != NULL)
	{
		fprintf(fMailBody, "%s", body.c_str());
		fclose(fMailBody);
	}

	system(caMailDetails);
	remove(pcMailAddress);
	remove(pcMailBodyFileName);
}

