/***************************************************************************************
Function     : AH_create_source_part_ecr_release()
Description  : call create_source_part_ecr_release() for each target object in root task
Input        : None
Output       : None
Author       : binhtt28
******************************************************************************************/
#include "Vinfast_Custom.h"
#include <map>

LinkedList setPartID2StPart;
LinkedList setPartID2StPartP;
LinkedList setPartID2StPartDEL;

int getPart(std::string partType, tag_t* tPart);
int getBOM(std::string id, tag_t* tBOM);
int getECR(std::string item_id, std::string partType, tag_t* tECR);
int getValidPurchasingPart();

extern int AH_create_source_part_ecr_release(EPM_action_message_t msg) {
	int   iRetCode = ITK_ok;
	//Get Source Part
	tag_t tPart = NULLTAG;
	getPart("BP VF33", &tPart);

	return iRetCode;
}

int getPart(std::string sourcing)
{
	int iRetCode = ITK_ok;
	int iCount = 0;
	tag_t tQuery = NULLTAG;
	std::string partNumber = "*";
	const char* queryName = "Source Part";
	Teamcenter::scoped_smptr<tag_t> tQryResults;

	CHECK_ITK(iRetCode, QRY_find2(queryName, &tQuery));
	if (tQuery == NULLTAG)
	{
		TC_write_syslog("ERROR: Cannot find query %s:\n", queryName);
		return ITK_ok;
	}

	char* queryEntries[] = { "VF Part Number", "Sourcing Program" };
	char** queryValues = (char**)MEM_alloc(sizeof(char*) * 2);

	queryValues[0] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(partNumber.c_str()) + 1));
	queryValues[1] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(sourcing.c_str()) + 1));

	queryValues[0] = tc_strcpy(queryValues[0], partNumber.c_str());
	queryValues[1] = tc_strcpy(queryValues[1], sourcing.c_str());

	CHECK_ITK(iRetCode, QRY_execute(tQuery, 2, queryEntries, queryValues, &iCount, &tQryResults));
	for (int i = 0; i < iCount; i++) {
		char* partID = NULL;
		char* purLvl = NULL;
		CHECK_ITK(iRetCode, AOM_ask_value_string(tQryResults[i], "vf4_bom_vfPartNumber", &partID));
		CHECK_ITK(iRetCode, AOM_ask_value_string(tQryResults[i], "vf4_purchasing_level", &purLvl));
		setPartID2StPart.insert(partID, tQryResults[i]);
		if (strcmp(partID, "") != NULL && strcmp(purLvl, "DEL") != 0)
		{
			setPartID2StPartP.insert(partID, tQryResults[i]);
		}
		else if (strcmp(partID, "") != NULL && strcmp(purLvl, "DEL") == 0) {
			setPartID2StPartDEL.insert(partID, tQryResults[i]);
		}
	}
	SAFE_SM_FREE(queryValues[0]);
	SAFE_SM_FREE(queryValues[1]);
	SAFE_SM_FREE(queryValues);

	return iRetCode;
}
int getECR(std::string id) {
	int iRetCode = ITK_ok;
	int iCount = 0;
	tag_t tQuery = NULLTAG;
	const char* queryName = "__TNH_FindECRApproved_by_ProposalPart";
	Teamcenter::scoped_smptr<tag_t> tQryResults;

	CHECK_ITK(iRetCode, QRY_find2(queryName, &tQuery));
	if (tQuery == NULLTAG)
	{
		TC_write_syslog("ERROR: Cannot find query %s:\n", queryName);
		return ITK_ok;
	}
	char* queryEntries[] = { "ID" };

	char** queryValues = (char**)MEM_alloc(sizeof(char*) * 2);

	queryValues[0] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(id.c_str()) + 1));

	queryValues[0] = tc_strcpy(queryValues[0], id.c_str());

	CHECK_ITK(iRetCode, QRY_execute(tQuery, 1, queryEntries, queryValues, &iCount, &tQryResults));
	for (int i = 0; i < iCount; i++) {

	}
	if (iCount == 0)
	{

		TC_write_syslog("[VFCost] More than 1 Items found for given ECR [%s] \n", id.c_str());
	}

	SAFE_SM_FREE(queryValues[0]);
	SAFE_SM_FREE(queryValues[1]);
	SAFE_SM_FREE(queryValues);

	return iRetCode;

}
int getBOM(std::string item_id, std::string partType, tag_t tECR) {
	int iRetCode = ITK_ok;
	int iCount = 0;
	tag_t tQuery = NULLTAG;
	const char* queryName = "Item...";
	Teamcenter::scoped_smptr<tag_t> tQryResults;

	CHECK_ITK(iRetCode, QRY_find2(queryName, &tQuery));
	if (tQuery == NULLTAG)
	{
		TC_write_syslog("ERROR: Cannot find query %s:\n", queryName);
		return ITK_ok;
	}
	char* queryEntries[] = { "Item ID", "Type" };

	char** queryValues = (char**)MEM_alloc(sizeof(char*) * 2);

	queryValues[0] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(item_id.c_str()) + 1));
	queryValues[1] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(partType.c_str()) + 1));

	queryValues[0] = tc_strcpy(queryValues[0], item_id.c_str());
	queryValues[1] = tc_strcpy(queryValues[0], partType.c_str());

	CHECK_ITK(iRetCode, QRY_execute(tQuery, 1, queryEntries, queryValues, &iCount, &tQryResults));
	for (int i = 0; i < iCount; i++) {
		char* partID = NULL;
		char* purLvl = NULL;
	}
	if (iCount == 0)
	{
		TC_write_syslog("[VFCost] More than 1 Items found for given Bom \n");
	}

	SAFE_SM_FREE(queryValues[0]);
	SAFE_SM_FREE(queryValues[1]);
	SAFE_SM_FREE(queryValues);

	return iRetCode;

}
int getValidPurchasingPart(tag_t* BomLine, tag_t* aPart)
{



	return 0;
}

