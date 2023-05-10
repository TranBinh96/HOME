#include "Vinfast_Custom.h"

#include <map>

extern int AH_send_parts_transfer_request(EPM_action_message_t msg)
{
	int			iRetCode				= ITK_ok;
	int			iNumArg					= 0;
	tag_t		tRootTask				= NULLTAG;

	std:string status("");
	std::map<std::string, int> allowedStatusesAndAge;

	char *isDebug = NULL;
	int ret_code = ITK_ok;
	CHECK_ITK(ret_code, PREF_ask_char_value("VF_PLM_DEBUG", 0, &isDebug));

	LOG(TC_write_syslog("[VF] Enter %s\n", __FUNCTION__));

	/*getting root task*/
	CHECK_ITK(iRetCode,EPM_ask_root_task(msg.task,&tRootTask));
	if(tRootTask != 0)
	{
		int			iTgt				= 0;
		tag_t		*ptTgt				= NULL;

		/*getting target objects*/
		CHECK_ITK(iRetCode,EPM_ask_attachments(tRootTask,EPM_target_attachment,&iTgt,&ptTgt));

		for (int i = 0; i < iTgt; i++)
		{
			tag_t partRev = ptTgt[i];
			logical isItemRev = false;
			tag_t itemRevType = NULLTAG;
			char *objType = NULL;
			CHECK_ITK(iRetCode,AOM_ask_value_string(partRev, "object_type", &objType));
			//CHECK_ITK(iRetCode, TCTYPE_find_type("ItemRevision", "ItemRevision", &itemRevType));
			//CHECK_ITK(iRetCode, TCTYPE_is_type_of(partRev, itemRevType, &isItemRev));
			isItemRev = string(objType).find("Revision") != string::npos;
			LOG(TC_write_syslog("[VF] isItemRev: %d\n", isItemRev));

			if (isItemRev)
			{
				char *partId = NULL;
				char *revId = NULL;
				char *uid = NULL;
				CHECK_ITK(iRetCode,AOM_ask_value_string(partRev, "item_id", &partId));
				CHECK_ITK(iRetCode,AOM_ask_value_string(partRev, "item_revision_id", &revId));
				
				
				LOG(TC_write_syslog("[VF] item_id: %s\n", partId));
				LOG(TC_write_syslog("[VF] revId: %s\n", revId));
				LOG(TC_write_syslog("[VF] object_type: %s\n", objType));
				

				ITK__convert_tag_to_uid(partRev, &uid);

				std::string json;
				json.append("{");
				json.append("type:\"TRANSFER_MATERIAL_SAP\",");
				json.append("uid:\"").append(uid).append("\"");
				json.append("}");

				try
				{
					char *targetDir = "";
					CHECK_ITK(ret_code, PREF_ask_char_value("VF_SAP_TRANSFER_TRIGGERING_FOLDER", 0, &targetDir));
					targetDir = (targetDir && strlen(targetDir) > 0) ? targetDir : "\\\\10.128.49.113\\SAP_Transfer\\sap_mm\\";
					std::string jsonFileName(targetDir);
					jsonFileName.append(partId).append("_").append(revId).append("_").append(objType).append(".json");
					LOG(TC_write_syslog("[VF] targetDir: %s\n", targetDir));
					LOG(TC_write_syslog("[VF] jsonFileName: %s\n", jsonFileName.c_str()));
					LOG(TC_write_syslog("[VF] json: %s\n", json.c_str()));
					std::ofstream os(jsonFileName.c_str());
					os << json;
					os.close();
					/*LOG(TC_write_syslog("[VF] checkWrittenFile\n"));
					ifstream checkWrittenFile;
					checkWrittenFile.open(jsonFileName.c_str());
					if (!checkWrittenFile.fail())
					{
					LOG(TC_write_syslog("[VF] write success\n"));
					checkWrittenFile.close();
					tag_t part = NULLTAG;
					tag_t* itemMasterForm = NULLTAG;
					time_t     now = time(0);
					struct tm  tstruct;
					char       buf[80];
					int itemMasterFormCount = -1;
					tstruct = *localtime(&now);
					strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
					std::string val("SAP_TRIGGERED#");
					val.append(buf);
					LOG(TC_write_syslog("[VF] val: %s\n", val.c_str()));
					CHECK_ITK(ret_code, AOM_ask_value_tag(partRev, "items_tag", &part));
					CHECK_ITK(ret_code, AOM_ask_value_tags(part, "IMAN_master_form", &itemMasterFormCount, &itemMasterForm));
					if (itemMasterFormCount == 1 && itemMasterForm != NULL)
					{
					CHECK_ITK(ret_code, AOM_refresh(itemMasterForm[0], TRUE));
					CHECK_ITK(ret_code, AOM_set_value_string(itemMasterForm[0], "object_desc", val.c_str()));
					CHECK_ITK(ret_code, AOM_save(itemMasterForm[0]));
					CHECK_ITK(ret_code, AOM_refresh(itemMasterForm[0], FALSE));
					MEM_free(itemMasterForm);
					}
					else
					{
					TC_write_syslog("[VF]ERROR: Cannot find Master Form in part %s-%s\n", partId, objType);
					}
					}*/
				}
				catch (const std::ios_base::failure &err)
				{
					TC_write_syslog("[VF] ERROR: err.what: %s\n", err.what());
				}

				SAFE_SM_FREE(partId);
				SAFE_SM_FREE(revId);
				SAFE_SM_FREE(uid);
			}

			SAFE_SM_FREE(objType);
		}

		SAFE_SM_FREE(ptTgt);
	}

	LOG(TC_write_syslog("[VF] Leave %s\n", __FUNCTION__));
	SAFE_SM_FREE(isDebug);

	return iRetCode;
}