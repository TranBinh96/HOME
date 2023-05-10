#ifndef Vinfast_Custom_h
#define Vinfast_Custom_h

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <tc/tc_startup.h>
#include <textsrv/textserver.h>
#include <epm/epm.h>
#include <epm/epm_errors.h>
#include <epm/epm_toolkit_tc_utils.h>
#include <user_exits/epm_toolkit_utils.h>
#include <tc/tc_util.h>
#include <sa/tcfile.h>
#include <tccore/tctype.h>
#include <tc/tc_arguments.h>
#include <sa/sa.h>
#include <tccore/item.h>
#include <tccore/aom.h>
#include <tccore/project.h>
#include <epm/cr.h>
#include <epm/cr_action_handlers.h>
#include <tccore/custom.h>
#include <ae/ae.h>
#include <tccore/aom_prop.h>
#include <epm/epm_task_template_itk.h>
#include <direct.h>
#include <fclasses/tc_date.h>
#include <tc/emh.h>
#include <tc/emh_errors.h>
#include <tc/envelope.h>
#include <tc/folder.h>
#include <tccore/grm.h>
#include <tccore/grm_errors.h>
#include <tccore/grmtype.h>
#include <ict/ict_userservice.h>
#include <tccore/item.h>
#include <tccore/method.h>
#include <property/nr.h>
#include <pom/pom/pom.h>
#include <pom/pom/pom_errors.h>
#include <tc/preferences.h>
#include <property/prop.h>
#include <ps/ps.h>
#include <sa/sa.h>
#include <sa/sa_errors.h>
#include <ss/ss_const.h>
#include <ss/ss_errors.h>
#include <time.h>
#include <sa/user.h>
#include <user_exits/user_exits.h>
#include <tccore/workspaceobject.h>
#include <tc/wsouif_errors.h>
#include <tccore/project.h>
#include <tc/tc_arguments.h>
#include <sa/groupmember.h>
#include <ecm/ecm.h>
#include <qry/qry.h>
#include <bom/bom.h>
#include <epm/signoff.h>
#include <ics\ics.h>
#include <ics\ics2.h>
#include <res\res_itk.h>
#include <sa/am.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <tc/lm.h>
#include <tc/log.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <set>
#include <lov/lov.h>
#include <cfm/cfm.h>
#include <base_utils/ScopedSmPtr.hxx>
#include <base_utils/ScopedPtr.hxx>
#include <form/form.h>
#include <tc/emh.h>
#include <stdarg.h>
#include <tc/aliaslist.h>
#define DECISION_SIZE            (8)

#define CR_cannot_open_file  EPM_cannot_open_file
#define __ADMIN_LATEST_VALID_STATUS "__ADMIN_LATEST_VALID_STATUS"
#define __ADMIN_LATEST_VALID_STATUS_ONLY "__ADMIN_LATEST_VALID_STATUS_ONLY"
#define __ADMIN_LATEST_VALID_STATUS_SCOOTER "__ADMIN_LATEST_VALID_STATUS_SCOOTER"

/*function declration*/

EPM_decision_t RH_Check_unreleaseed_ItemRevision(EPM_rule_message_t msg);
EPM_decision_t RH_check_SAP_transfer_prop(EPM_rule_message_t msg);
EPM_decision_t RH_check_checkedout_objects(EPM_rule_message_t msg);
EPM_decision_t RH_check_bom_bop_status(EPM_rule_message_t msg);
EPM_decision_t RH_check_workflow_process_status(EPM_rule_message_t msg);
EPM_decision_t RH_check_revision_release_status(EPM_rule_message_t msg);
EPM_decision_t RH_check_task_state(EPM_rule_message_t msg);
EPM_decision_t RH_check_release_assembly_escooter(EPM_rule_message_t msg);
EPM_decision_t RH_Part_Not_Include_Release_Statuses(EPM_rule_message_t msg);
EPM_decision_t RH_check_revision_baseline(EPM_rule_message_t msg);
EPM_decision_t RH_is_include_baseline_revisions(EPM_rule_message_t msg);
EPM_decision_t RH_is_ECN_valid_to_trigger(EPM_rule_message_t msg);
EPM_decision_t RH_check_object_properties(EPM_rule_message_t msg);
EPM_decision_t RH_check_dde_status(EPM_rule_message_t msg);
EPM_decision_t RH_check_dde_storage_exception(EPM_rule_message_t msg);

extern int AH_send_email_to_all(EPM_action_message_t msg);
extern int AH_send_email_to_all_participants(EPM_action_message_t msg);
extern int AH_attach_cost_form(EPM_action_message_t msg);
extern int AH_send_mail_notification_workflow_participants(EPM_action_message_t msg);
extern int AH_change_ownership_cl_to_cl(EPM_action_message_t msg);
extern int AH_assign_access_cl_to_buyer(EPM_action_message_t msg);
extern int AH_create_purchasing_object(EPM_action_message_t msg);
extern int AH_add_release_sts_sol_items(EPM_action_message_t msg);
extern int AH_add_release_sts_sol_items_scooter(EPM_action_message_t msg);
extern int AH_attach_manufac_form(EPM_action_message_t msg);
extern int AH_route_process_by_dataset(EPM_action_message_t msg);
extern int AH_send_parts_transfer_request(EPM_action_message_t msg);
extern int AH_MCR_create_purchasing_object(EPM_action_message_t msg);
extern int AH_create_escooter_plant_form(EPM_action_message_t msg);
extern int AH_create_plant_form_afs(EPM_action_message_t msg);
extern int AH_create_or_fill_plant_form(EPM_action_message_t msg);
extern int AH_send_parts_transfer_request(EPM_action_message_t msg);
extern int AH_send_email_to_all_participants_car(EPM_action_message_t msg);
extern int AH_notify(EPM_action_message_t msg);
extern int AH_notify_scooter_relase_all(EPM_action_message_t msg);
extern int AH_copy_costImpactForm_2_costObject(EPM_action_message_t msg);
extern int AH_send_email_to_all_participants_scooter(EPM_action_message_t msg);
extern int AH_assign_workflow_tasks(EPM_action_message_t msg);
extern int AH_send_mail_EPM_noti_workflow_participants(EPM_action_message_t msg);
extern int AH_create_ecn(EPM_action_message_t msg);
extern int AH_set_number_property(EPM_action_message_t msg);
extern int AH_cpy_piece_cost_form(EPM_action_message_t msg);
extern int AH_create_mcn(EPM_action_message_t msg);
//extern int AH_update_partExtraInfoForm(EPM_action_message_t msg);
extern int AH_copy_part_to_ecn_storage(EPM_action_message_t msg);
extern int AH_create_dde_from_ecn_storage(EPM_action_message_t msg);
extern int AH_jes_sos_notification_worklow(EPM_action_message_t msg);
extern int AH_send_mail_update_status_jes(EPM_action_message_t msg);
extern int AH_set_reviewer_to_jes_form(EPM_action_message_t msg);

std::vector<std::string> split_string(std::string str, char delimiter);
std::vector<std::string> split_string2(std::string str, std::string delimiter);
logical has_valid_status(char* pcStatus, char* pcReqStatus);
bool containElement(std::vector<std::string> v, std::string key);
std::string getCurrentTimeStampStr();
char* getPreferenceValue(char* prefName);
void sendEmail(char* mailSubject, std::string mailBody, std::vector<std::string> mailToVector, logical isDebug);
std::string genHtmlTitle(char* value);
std::string genHtmlPropertyName(char* value);
std::string genHtmlPropertyValue(std::string value);
std::string genHtmlTableHeader(char* value);
std::string genHtmlTableBody(char* value);
std::string genHtmlLink(std::string link, char* name);
std::string genHtmlNote(char* value);
char* getUserMailOfTask(tag_t task);
std::string getNextStatus(const std::string& currentStatus);
std::string getLatestValidStatus(const std::string & statusProgressionInput, tag_t tRev, const char* isDebug);
std::string getValidChildrenStatusesStr(const std::string& currentStatus);
std::string getDefaultStatusProgression();

using namespace std;
/* Macro definition */
#define CHECK_ITK(iSts,fn){\
	if(iSts == ITK_ok){\
		if((iSts = (fn) ) != ITK_ok){\
			char* error_str = NULL;\
			EMH_ask_error_text(iSts, &error_str);\
			TC_write_syslog ("+++ERROR: %d ERROR MSG: %s. Error in line %d, function %s",\
				iSts, error_str, __LINE__, __FUNCTION__);\
			MEM_free(error_str);\
		}\
	}\
}

#define LOG(fn){\
	if (isDebug && (tc_strcasecmp(isDebug, "true") == 0)) \
		fn;\
}

#define SAFE_MEM_free(x)\
  if(x != NULL)\
  MEM_free(x);               \
  x = NULL;

/*RH-check-SAP-transfer-prop*/
#define     CHECK_PLANT_CODE            (EMH_USER_error_base + 90)
#define     CHECK_PLANT_PPAP			(EMH_USER_error_base + 100)
#define     CHECK_PLANT_MAKE_BUY		(EMH_USER_error_base + 110)
#define     CHECK_PLANT_FORM            (EMH_USER_error_base + 120)
#define     CHECK_MATERIAL_TYPE         (EMH_USER_error_base + 130)
#define     CHECK_TRACEABLE_PART        (EMH_USER_error_base + 140)
#define     CHECK_SAP_PLANT             (EMH_USER_error_base + 150)
#define     CHECK_CHANGE_REASON         (EMH_USER_error_base + 160)
#define     CHECK_CHANGE_TYPE			(EMH_USER_error_base + 170)
#define     CHECK_DISP_CODE				(EMH_USER_error_base + 180)
#define     CHECK_MODEL_YEAR            (EMH_USER_error_base + 200)
#define     CHECK_ACTION				(EMH_USER_error_base + 210)
#define     CHECK_DISC					(EMH_USER_error_base + 220)

/*RH-check-object-property*/
#define     CHECKOUT_STATUS				(EMH_USER_error_base + 60)

/*RH-check-unreleased-IR*/
#define     CHECK_ASSEMBLY				(EMH_USER_error_base + 70)
#define     CHECK_STATUS				(EMH_USER_error_base + 80)

/*RH-check-workflow-process*/
#define     CHECK_WORKFLOW				(EMH_USER_error_base + 230)

/*RH-check-release-status*/
#define     CHECK_REV_STS				(EMH_USER_error_base + 240)

#define		SOURCING_PART_EXISTED		(EMH_USER_error_base + 250)
#define		SOURCING_INVALID_OBJ_TYPE   (EMH_USER_error_base + 260)
#define		INVALID_BOM_PROPERTY		(EMH_USER_error_base + 270)
#define		MISSING_ARGUMENT			(EMH_USER_error_base + 290)
#define     INVALID_PARAMETER           (EMH_USER_error_base + 300)
#define     INTERNAL_ERROR              (EMH_USER_error_base + 999)
#define     CHECK_SUBGROUP				(EMH_USER_error_base + 280)
#endif

// trim from start (in place)
inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}
