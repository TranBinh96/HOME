#include "Vinfast_Custom.h"

struct DDE_Create_Input {
	std::string item_id;
	std::string object_name;
	std::string object_type;
	std::string object_desc;
	std::string status;
	std::string revision_rule;
	std::string export_type;
};

auto createDDEObject(DDE_Create_Input dde_input, tag_t& dde_item) -> int;
auto setRelated(tag_t primaryObject, tag_t secondaryObject, tag_t relationObject) -> int;

tag_t getVendor(std::string vendorCode) {
	tag_t vendor = NULLTAG;
	int iRetCode = ITK_ok;
	int iCount = 0;
	tag_t tQuery = NULLTAG;
	const char* queryName = "Vendor Contact Search";
	tag_t* tQryResults;

	CHECK_ITK(iRetCode, QRY_find2(queryName, &tQuery));
	if (tQuery == NULLTAG) {
		TC_write_syslog("ERROR: Cannot find query %s:\n", queryName);
		return ITK_ok;
	}

	char* queryEntries[] = { "Last Name" };
	char** queryValues = (char**)MEM_alloc(sizeof(char*) * 1);

	queryValues[0] = (char*)MEM_alloc(tc_strlen(vendorCode.c_str()) + 1);
	queryValues[0] = tc_strcpy(queryValues[0], vendorCode.c_str());

	/*queryValues[1] = (char*)MEM_alloc(tc_strlen("0") + 1);
	queryValues[1] = tc_strcpy(queryValues[1], "0");*/

	CHECK_ITK(iRetCode, QRY_execute(tQuery, 1, queryEntries, queryValues, &iCount, &tQryResults));
	if (iCount == 1) {
		vendor = tQryResults[0];
	}
	else {
		TC_write_syslog("[] More than 1 Items found for given Vendor Code [%s]\n", vendorCode.c_str());
	}

	SAFE_SM_FREE(queryValues[0]);
	SAFE_SM_FREE(queryValues);

	return vendor;
}



extern int AH_create_dde_from_ecn_storage(EPM_action_message_t msg) {
	int	iRetCode = ITK_ok;
	char ErrorMessage[2048] = "";
	tag_t tRootTask = NULLTAG;
	const char* ecnObjType = NULLTAG;
	const char* ddeObjType = NULLTAG;

	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0) {
		int	iTgt = 0;
		ecnObjType = ("Vf6_ECNRevision");
		ddeObjType = ("VF4_SCDDE");
		Teamcenter::scoped_smptr<tag_t> ptTgt;
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		for (int i = 0; i < iTgt; i++) {
			tag_t ecnItem = NULLTAG;
			tag_t dde_item_create_input_tag = NULLTAG;
			tag_t dde_rev_create_input_tag = NULLTAG;

			Teamcenter::scoped_smptr<char> objType;
			Teamcenter::scoped_smptr<char> ecnRevType;
			Teamcenter::scoped_smptr<char> ecnID;
			Teamcenter::scoped_smptr<char> ecnRevision;
			Teamcenter::scoped_smptr<char> ecnName;

			CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[i], &ecnItem));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[i], "object_type", &ecnRevType));
			TC_write_syslog("Item Revision Type %d\n", ecnRevType.getString());
			CHECK_ITK(iRetCode, AOM_ask_value_string(ecnItem, "object_type", &objType));
			if (tc_strcmp(ecnRevType.getString(), ecnObjType) == 0) {
				//Vf6_DDE_store_table: object name
				//vf6_dde_store_table: property name
				tag_t* tableRow = NULL;
				int numberOfRow = 0;

				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[i], "item_id", &ecnID));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[i], "item_revision_id", &ecnRevision));
				CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[i], "object_name", &ecnName));

				DDE_Create_Input dde_input = {};
				dde_input.object_type = "VF4_SCDDE";
				std::stringstream name;
				std::stringstream desc;

				name << ecnID.getString() << "/" << ecnRevision.getString();
				desc << ecnID.getString() << "/" << ecnRevision.getString() << "-" << ecnName.getString();

				dde_input.object_name = name.str();
				dde_input.object_desc = desc.str();
				dde_input.status = "REQUEST";
				dde_input.revision_rule = "VINFAST_RELEASE_RULE";
				dde_input.export_type = "Default";

				std::map<std::string, std::vector<tag_t>> ddeMap;
				CHECK_ITK(iRetCode, AOM_ask_table_rows(ptTgt[i], "vf6_dde_store_table", &numberOfRow, &tableRow));
				if (numberOfRow > 0) {
					for (int j = 0; j < numberOfRow; j++) {
						char* isSendValue = NULL;
						tag_t part = NULLTAG;
						char* vendorCodeValue = NULL;
						CHECK_ITK(iRetCode, AOM_ask_value_string(tableRow[j], "vf6_is_send", &isSendValue));
						if (tc_strcmp(isSendValue, "Yes") == 0) {
							CHECK_ITK(iRetCode, AOM_ask_value_string(tableRow[j], "vf6_vendor_code", &vendorCodeValue));
							CHECK_ITK(iRetCode, AOM_ask_value_tag(tableRow[j], "vf6_object_reference", &part));

							std::vector<std::string> vendorCodes = split_string(vendorCodeValue, ';');
							for (int iVendorCode = 0; iVendorCode < vendorCodes.size(); iVendorCode++) {
								std::string vendorCodeTmp = vendorCodes[iVendorCode];
								ltrim(vendorCodeTmp);
								rtrim(vendorCodeTmp);

								if (ddeMap.find(vendorCodeTmp) == ddeMap.end()) {
									std::vector<tag_t> vec1;
									vec1.push_back(part);
									ddeMap[vendorCodeTmp] = vec1;
								}
								else {
									std::map<std::string, std::vector<tag_t>>::iterator var = ddeMap.find(vendorCodeTmp);
									vector<tag_t>::iterator it = std::find(var->second.begin(), var->second.end(), part);
									if (it == var->second.end()) {
										var->second.push_back(part);
									}
								}
							}
						}
					}

					if (ddeMap.size() > 0) {
						tag_t iman_rela_type = NULLTAG;
						tag_t contact_rela_type = NULLTAG;
						tag_t implementBy_rela_type = NULLTAG;
						tag_t cmImplements = NULLTAG;
						CHECK_ITK(iRetCode, GRM_find_relation_type("IMAN_requirement", &iman_rela_type));
						CHECK_ITK(iRetCode, GRM_find_relation_type("ContactInCompany", &contact_rela_type));
						CHECK_ITK(iRetCode, GRM_find_relation_type("Vf6_ecn_data_exchange", &implementBy_rela_type));

						CHECK_ITK(iRetCode, AOM_refresh(ptTgt[i], TRUE));
						for (auto x : ddeMap) {
							tag_t dde_item = NULLTAG;
							iRetCode = createDDEObject(dde_input, dde_item);
							if (iRetCode == ITK_ok) {
								for (tag_t part : x.second) {
									iRetCode = setRelated(dde_item, part, iman_rela_type);
								}
								string vender = x.first;
								std::string vendorCode = vender;
								std::string vendorCodebyItem = "";
								if (vender.find("VINES_", 0) != -1)
									vendorCodebyItem.append(vender);
								else {
									vendorCodebyItem.append("VINES_");
									vendorCodebyItem.append(vender);
								}
								tag_t _vendor = getVendor(vendorCode);
								tag_t vendorItem = getVendor(vendorCodebyItem);
								if (_vendor != NULL) {
									iRetCode = setRelated(dde_item, _vendor, contact_rela_type);
								}
								if (vendorItem != NULL) {
									iRetCode = setRelated(dde_item, vendorItem, contact_rela_type);
								}
								CHECK_ITK(iRetCode, GRM_create_relation(ptTgt[i], dde_item, implementBy_rela_type, NULLTAG, &cmImplements));
								CHECK_ITK(iRetCode, GRM_save_relation(cmImplements));
							}
						}
						CHECK_ITK(iRetCode, AOM_refresh(ptTgt[i], FALSE));
					}
				}
			}
		}
	}
	return iRetCode;
}

int setRelated(tag_t primaryObject, tag_t secondaryObject, tag_t relationObject) {
	int iRetCode = ITK_ok;

	tag_t relation_created = NULLTAG;
	CHECK_ITK(iRetCode, GRM_create_relation(primaryObject, secondaryObject, relationObject, NULLTAG, &relation_created));
	CHECK_ITK(iRetCode, AOM_refresh(primaryObject, 0));
	CHECK_ITK(iRetCode, GRM_save_relation(relation_created));

	if (relation_created != NULL) {
		relation_created = NULL;
	}

	return iRetCode;
}

int createDDEObject(DDE_Create_Input dde_input, tag_t& dde_item) {
	int ret_code = ITK_ok;
	tag_t item_type_tag = NULLTAG;
	tag_t itemRev_type_tag = NULLTAG;
	tag_t item_create_input_tag = NULLTAG;
	tag_t itemRev_create_input_tag = NULLTAG;
	//tag_t item_tag = NULLTAG;

	/*creating item*/
	CHECK_ITK(ret_code, TCTYPE_find_type(dde_input.object_type.c_str(), dde_input.object_type.c_str(), &item_type_tag));
	if (item_type_tag == NULLTAG) {
		TC_write_syslog("[VF_handlers]__%d__%s__Could not find tag of object type: %s\n", __LINE__, __FUNCTION__, dde_input.object_type);
	}
	CHECK_ITK(ret_code, TCTYPE_construct_create_input(item_type_tag, &item_create_input_tag));

	/*find item revision tag type*/
	const std::string dde_partRev_object_type = dde_input.object_type.append("Revision");
	CHECK_ITK(ret_code, TCTYPE_find_type(dde_partRev_object_type.c_str(), dde_partRev_object_type.c_str(), &itemRev_type_tag));
	if (itemRev_type_tag == NULLTAG) {
		TC_write_syslog("[VF_handlers]__%d__%s__Could not find tag of object type: %s\n", __LINE__, __FUNCTION__, dde_partRev_object_type);
	}
	const char* tmp_obj_name = dde_input.object_name.c_str();
	const char* tmp_obj_desc = dde_input.object_desc.c_str();
	const char* tmp_status = dde_input.status.c_str();
	const char* tmp_revision_rule = dde_input.revision_rule.c_str();
	const char* tmp_export_type = dde_input.export_type.c_str();

	TC_write_syslog("[VF_handlers]__%d__%s__Create sourcing part: %s\n", __LINE__, __FUNCTION__, dde_input.item_id);
	tag_t tCLUser = NULLTAG;

	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "object_name", 1, &tmp_obj_name));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "object_desc", 1, &tmp_obj_desc));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "vf4_status", 1, &tmp_status));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "vf4_revision_rule", 1, &tmp_revision_rule));
	CHECK_ITK(ret_code, TCTYPE_set_create_display_value(item_create_input_tag, "vf4_export_type", 1, &tmp_export_type));

	CHECK_ITK(ret_code, TCTYPE_construct_create_input(itemRev_type_tag, &itemRev_create_input_tag));

	CHECK_ITK(ret_code, AOM_set_value_tag(item_create_input_tag, "revision", itemRev_create_input_tag));
	CHECK_ITK(ret_code, TCTYPE_create_object(item_create_input_tag, &dde_item));
	CHECK_ITK(ret_code, AOM_save_with_extensions(dde_item));
	//CHECK_ITK(ret_code, ITEM_ask_latest_rev(item_tag, &dde_rev));

	return ret_code;
}