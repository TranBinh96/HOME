#include "Vinfast_Custom.h"
/*******************************************************************************
File         : AH_attach_manufac_form.cpp

Description  : Create MCN form to get decide MCN. Only place on Perform-Signoff Task
  
Input        : None
                        
Output       : None

Author       : VinFast

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Nov,22 2019     1.0         Duc		     Initial Creation
Jan,01 2020		1.1			Duc			 Create MCN form, Purchasing form
Jan,06 2020		1.2			Duc			 Create Is Chairman Approval form
*******************************************************************************/

extern int AH_attach_manufac_form(EPM_action_message_t msg)
{
	int			iRetCode			= ITK_ok;
    tag_t		tRootTask			= NULLTAG;
	int count = 0;
	tag_t currenttask = NULLTAG;
	currenttask = msg.task;
	//tag_t user = NULLTAG;
	//tag_t group = NULLTAG;
	//tag_t cl_user = NULLTAG;
	char *ojbType = NULLTAG;
	char *approver_name=NULL;
	int countsignoff=0;
	tag_t *attSignoff = NULLTAG;
    /*getting root task*/
    CHECK_ITK(iRetCode,EPM_ask_root_task(msg.task,&tRootTask));
	if(tRootTask != 0)
    {
        int			iTgt				= 0;
        tag_t		*ptTgt				= NULL;
		/*getting target objects*/
		//TC_write_syslog("find attachments on task");
		CHECK_ITK(iRetCode,EPM_ask_attachments(tRootTask,EPM_target_attachment,&iTgt,&ptTgt));
		//TC_write_syslog("end find attachments on task");
		 for(int iInd = 0; iInd < iTgt; iInd++)
        {
            int		iProp			= 0;
            int		iOldForm		= 0;
			int countTask = 0;
			int taskCount = 0;
			int countReviewTask = 0;
			int countSignoff = 0;
			tag_t *tagObjects = NULLTAG;
			tag_t *subReviewTask = NULLTAG;
			tag_t *subtasks = NULLTAG;
			tag_t performer = NULLTAG;
            tag_t	tItem			= NULLTAG;
            tag_t	tChangeItem		= NULLTAG;
            tag_t	tPropType		= NULLTAG;
            tag_t   tFormRel		= NULLTAG;
            tag_t	*ptProp			= NULL;
            tag_t   *ptOldForm		= NULL;
			tag_t    *childTasks	= NULLTAG;
            char	*pcObjType		= NULL;
			char	*parent_task_name = NULL;
			char	*taskType	= NULL;
			char	*task_name	= NULL;
			tag_t   tManuForm					= NULLTAG;
			tag_t   tCreateInputManuForm		= NULLTAG;
			 string   szManuForm				= "";
			 tag_t   tManuCostForm				= NULLTAG;
			 tag_t   tCrtdManuRel				= NULLTAG;
			 tag_t   parent_task				= NULLTAG;
            CHECK_ITK(iRetCode,WSOM_ask_object_type2(ptTgt[iInd],&pcObjType));
			//TC_write_syslog("objType: %s", pcObjType);
            /*for object of ECR Revision*/
            if(tc_strcmp(pcObjType,"Vf6_ECRRevision") == 0 )
            {
                CHECK_ITK(iRetCode,GRM_find_relation_type ("Vf6_MCN_Relation", &tFormRel));
                CHECK_ITK(iRetCode,GRM_list_secondary_objects_only(ptTgt[iInd],tFormRel,&iOldForm,&ptOldForm));
				 CHECK_ITK(iRetCode,TCTYPE_find_type ("Vf6_MCNDecide_Form","Form",&tManuForm));
							if(iOldForm < 3)
							{
                                if(iRetCode == ITK_ok && tManuForm != NULLTAG)
                                {
                                    CHECK_ITK(iRetCode,TCTYPE_construct_create_input (tManuForm, &tCreateInputManuForm));
                                    if(iRetCode == ITK_ok && tCreateInputManuForm != NULLTAG)
                                    {
                                        /*set name*/
										//Ask parent task
									//	CHECK_ITK(iRetCode, EPM_ask_parent_task(currenttask, &parent_task));
										CHECK_ITK(iRetCode, EPM_ask_name2(currenttask, &parent_task_name));
										std::string Str = std::string(parent_task_name);
										if(Str.compare("Purchase Review") == 0)
										{
											szManuForm  = "Purchase Business Review";
											CHECK_ITK(iRetCode,AOM_set_value_string(tCreateInputManuForm, "object_name", szManuForm.c_str()));
										}
										else if(Str.compare("Manufacturing Review") == 0)
										{
											szManuForm = "Is MCN Required";
											CHECK_ITK(iRetCode, AOM_set_value_string(tCreateInputManuForm, "object_name", szManuForm.c_str()));
										}
										else if(Str.compare("Change Approval Board") == 0)
										{
											szManuForm = "Is Chairman Approval";
											CHECK_ITK(iRetCode, AOM_set_value_string(tCreateInputManuForm, "object_name", szManuForm.c_str()));
										}
										else
										{
											szManuForm = "Dont know what use for";
											CHECK_ITK(iRetCode, AOM_set_value_string(tCreateInputManuForm, "object_name", szManuForm.c_str()));
										}
                                        if(iRetCode == ITK_ok)
                                        {
                                            /*create object*/
                                            CHECK_ITK(iRetCode,TCTYPE_create_object (tCreateInputManuForm, &tManuCostForm));
                                            if(iRetCode == ITK_ok && tManuCostForm != NULLTAG)
                                            {
												/*Change ownership*/
												
                                                /*create relation between form and ecr rev*/
                                                CHECK_ITK(iRetCode,GRM_create_relation(ptTgt[iInd],tManuCostForm,tFormRel,NULLTAG,&tCrtdManuRel));
                                               if(iRetCode == ITK_ok && tCrtdManuRel != NULLTAG)
                                                {
                                                    /*save form*/
                                                    CHECK_ITK(iRetCode,AOM_save (tManuCostForm));
                                                    if(iRetCode == ITK_ok)
                                                    {
                                                        CHECK_ITK(iRetCode,AOM_refresh (tManuCostForm, 0));
                                                        if(iRetCode == ITK_ok)
                                                        {
                                                            /*save relation*/
                                                            CHECK_ITK(iRetCode,GRM_save_relation (tCrtdManuRel));
                                                        }
                                                    }	
                                                }
                                            }
                                        }
			                        }
		                        }
							/*	CHECK_ITK(iRetCode, EPM_ask_sub_tasks(tRootTask, &taskCount, &childTasks));
								for(int m=0;m < taskCount, m++)
								{
									//Check task type
									CHECK_ITK(iRetCode, AOM_ask_value_string(childTasks[m], "task_type", &taskType));
									if(tc_strcmp(taskType, "EPMConditionTask") == 0)
									{
										//check task name
										CHECK_ITK(iRetCode, AOM_ask_value_string(childTasks[m], "task_name", &task_name));
										if(tc_strcmp(task_name, "Purchase Review") == 0)
										{
											//Find reviewer
											CHECK_ITK(iRetCode, EPM_ask_attachments(childTasks[m], EPM_signoff_attachment, ))
										}
									}
								}
								//CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_, &countTask, &subtasks));
								for(int k=0;k< countTask;k++)
								{
									tag_t member_tag = NULLTAG;
									SIGNOFF_TYPE_t member_type_tag;
									CHECK_ITK(iRetCode, EPM_ask_signoff_member(subtasks[k], &member_tag, &member_type_tag));
									if(member_type_tag == SIGNOFF_GROUPMEMBER)
									{
										tag_t user_tag = NULLTAG;
										tag_t group_tag = NULLTAG;
										int noOfmember ;
										CHECK_ITK(iRetCode, SA_ask_groupmember_user(member_tag, &user_tag));  
										CHECK_ITK(iRetCode, SA_ask_groupmember_group(member_tag, &group_tag));
										char *user_id = "";
										char *group_name = "";
										CHECK_ITK(iRetCode, SA_ask_user_identifier2(user_tag, &user_id));
										CHECK_ITK(iRetCode, SA_ask_group_name2(group_tag, &group_name));
										//TC_write_syslog("userID: %s\n", user_id);
										//TC_write_syslog("Group name: %s\n", group_name);
										
										CHECK_ITK(iRetCode, AOM_set_ownership(tManuCostForm, user_tag,group_tag));
										//CHECK_ITK(iRetCode, AOM_save(tManuCostForm));
										SAFE_MEM_free(user_id);
										SAFE_MEM_free(group_name);
									}
								
								}*/
								
							}
							else
							{
								TC_write_syslog("Object already have forms");
							}
	
			}
			
			
		
			
			

			
		 }
	}
	return iRetCode;
}
		 
	
	