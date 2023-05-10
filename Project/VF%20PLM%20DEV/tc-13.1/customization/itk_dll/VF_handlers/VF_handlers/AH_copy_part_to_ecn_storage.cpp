#include "Vinfast_Custom.h"

std::string getVendorCode(tag_t itemRev) {
	int	iRetCode = ITK_ok;
	tag_t item = NULLTAG;
	Teamcenter::scoped_smptr<char> supplierCode;

	CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(itemRev, &item));
	if (iRetCode == ITK_ok) {
		CHECK_ITK(iRetCode, AOM_ask_value_string(item, "vf4_itm_supplier_code", &supplierCode));
		if (iRetCode == ITK_ok) {
			return supplierCode.getString();
		}
	}
	return "";
}

extern int AH_copy_part_to_ecn_storage(EPM_action_message_t msg) {
	int	iRetCode = ITK_ok;
	char ErrorMessage[2048] = "";
	tag_t tRootTask = NULLTAG;
	const char* ecnObjType = NULLTAG;

	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0) {
		int	iTgt = 0;
		ecnObjType = ("Vf6_ECNRevision");
		Teamcenter::scoped_smptr<tag_t> ptTgt;
		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		for (int i = 0; i < iTgt; i++) {
			tag_t ecnType = NULLTAG;
			tag_t ecnItem = NULLTAG;
			tag_t item_create_input_tag = NULLTAG;
			tag_t rev_create_input_tag = NULLTAG;

			Teamcenter::scoped_smptr<char> objType;
			Teamcenter::scoped_smptr<char> ecrRevType;

			CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[i], &ecnItem));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[i], "object_type", &ecrRevType));
			TC_write_syslog("Item Revision Type %d\n", ecrRevType.getString());
			CHECK_ITK(iRetCode, AOM_ask_value_string(ecnItem, "object_type", &objType));
			if (tc_strcmp(ecrRevType.getString(), ecnObjType) == 0) {
				//Vf6_DDE_store_table: object name
				//vf6_dde_store_table: property name
				int iCountSolutionObjs = 0;
				tag_t tSolution = NULLTAG;
				Teamcenter::scoped_smptr<tag_t> ptSolutionObjs;
				std::string isSend = "Yes";

				CHECK_ITK(iRetCode, GRM_find_relation_type("CMHasSolutionItem", &tSolution));
				CHECK_ITK(iRetCode, GRM_list_secondary_objects_only(ptTgt[i], tSolution, &iCountSolutionObjs, &ptSolutionObjs));

				if (iCountSolutionObjs > 0) {
					tag_t* tableItems = NULL;

					int index = 0;
					if (iRetCode == ITK_ok) {
						ITK_set_bypass(TRUE);
						for (int j = 0; j < iCountSolutionObjs; j++) {
							CHECK_ITK(iRetCode, AOM_refresh(ptTgt[i], TRUE));
							CHECK_ITK(iRetCode, AOM_append_table_rows(ptTgt[i], "vf6_dde_store_table", 1, &index, &tableItems));
							CHECK_ITK(iRetCode, AOM_refresh(tableItems[0], TRUE));
							CHECK_ITK(iRetCode, AOM_set_value_tag(tableItems[0], "vf6_object_reference", ptSolutionObjs[j]));
							CHECK_ITK(iRetCode, AOM_set_value_string(tableItems[0], "vf6_is_send", isSend.c_str()));
							//get item
							tag_t item = NULLTAG;
							Teamcenter::scoped_smptr<char> supplierCode;
							Teamcenter::scoped_smptr<char> supplierName;
							char supplierName2[2048] = "";
							CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptSolutionObjs[j], &item));
							if (iRetCode == ITK_ok) {
								CHECK_ITK(iRetCode, AOM_ask_value_string(item, "vf4cp_itm_supplier_code", &supplierCode));
								CHECK_ITK(iRetCode, AOM_ask_value_string(item, "vf4cp_itm_supplier_name", &supplierName));
								if (tc_strcmp(supplierCode.getString(), "") != 0)
									sprintf_s(supplierName2, "%s (%s)", supplierCode.getString(), supplierName.getString());
							}
							CHECK_ITK(iRetCode, AOM_set_value_string(tableItems[0], "vf6_vendor_code", supplierCode.getString()));
							CHECK_ITK(iRetCode, AOM_set_value_string(tableItems[0], "vf6_vendor_name", supplierName2));
							CHECK_ITK(iRetCode, AOM_save_with_extensions(tableItems[0]));
							CHECK_ITK(iRetCode, AOM_refresh(tableItems[0], FALSE));

							if (tableItems != NULL) {
								MEM_free(tableItems);
								tableItems = NULL;
							}
						}
					}
					CHECK_ITK(iRetCode, AOM_refresh(ptTgt[i], FALSE));
				}
			}
		}
	}
	return iRetCode;
}