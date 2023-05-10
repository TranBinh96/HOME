#include "Vinfast_Custom.h"

EPM_decision_t RH_check_dde_storage_exception(EPM_rule_message_t msg) {
	int	iRetCode = ITK_ok;
	char ErrorMessage[2048] = "";
	int	iNumArg = 0;
	tag_t tRootTask = NULLTAG;
	char* pcFlag = NULL;
	char* pcValue = NULL;
	char* statusList = NULL;
	char* relationName = NULL;

	EPM_decision_t decision = EPM_go;

	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0) {
		int	iTgt = 0;
		const char* ecnObjType = ("Vf6_ECNRevision");
		Teamcenter::scoped_smptr<tag_t> ptTgt;

		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		for (int i = 0; i < iTgt; i++) {
			tag_t ecnItem = NULLTAG;

			Teamcenter::scoped_smptr<char> objType;
			Teamcenter::scoped_smptr<char> ecnRevType;

			CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[i], &ecnItem));
			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[i], "object_type", &ecnRevType));
			TC_write_syslog("Item Revision Type %d\n", ecnRevType.getString());
			CHECK_ITK(iRetCode, AOM_ask_value_string(ecnItem, "object_type", &objType));
			if (tc_strcmp(ecnRevType.getString(), ecnObjType) == 0) {
				tag_t* tableRow = NULL;
				int numberOfRow = 0;
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

							if (tc_strcmp(vendorCodeValue, "") == 0) {
								decision = EPM_nogo;
								char ErrorMessage[2048] = "";
								sprintf_s(ErrorMessage, "Vendor Information was not filled. Please fill in Vendor Information in Data Exchange Table.");
								EMH_store_error_s1(EMH_severity_user_error, CHECK_REV_STS, ErrorMessage);
								break;
							}
						}
					}
				}
			}
		}
	}
	return decision;
}