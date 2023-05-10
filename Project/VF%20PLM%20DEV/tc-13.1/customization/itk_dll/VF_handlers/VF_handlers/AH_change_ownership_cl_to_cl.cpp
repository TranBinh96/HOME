/*******************************************************************************
File         : AH_change_ownership_cl_to_cl.cpp

Description  : To change ownership
  
Input        : None
                        
Output       : None

Author       : Siemens

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
OCT,04 2019     1.0         Kantesh		 Initial Creation

/*******************************************************************************/
#include "Vinfast_Custom.h"

extern int AH_change_ownership_cl_to_cl(EPM_action_message_t msg)
{
    int			iRetCode				= ITK_ok;
    tag_t		tRootTask				= NULLTAG;

    /*getting root task*/
    CHECK_ITK(iRetCode,EPM_ask_root_task(msg.task,&tRootTask));
    if(tRootTask != 0)
    {
        int			iAllTasks			= 0;
        int			iTgt				= 0;
        tag_t		*ptTgt				= NULL;
        tag_t		*ptAllTaskTags		= NULL;
        tag_t		tUser				= NULLTAG;
        tag_t		tGroup				= NULLTAG;
        
        CHECK_ITK(iRetCode,EPM_ask_sub_tasks(tRootTask,&iAllTasks,&ptAllTaskTags));
        CHECK_ITK(iRetCode,EPM_ask_attachments(tRootTask,EPM_target_attachment,&iTgt,&ptTgt));
        /*getting user and group tag of all participants from review task*/
        for(int iLoopAllTasks = 0 ; iLoopAllTasks < iAllTasks; iLoopAllTasks++)
        {
            char		*pcTaskTypeName		= NULL;

            CHECK_ITK(iRetCode,AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks], "task_type", &pcTaskTypeName));
            if (tc_strcmp(pcTaskTypeName,"EPMReviewTask") == 0) 
            {
                int		iAllSubTasks			= 0;
                tag_t	*ptAllSubTaskTags		= NULL;

                CHECK_ITK(iRetCode,EPM_ask_sub_tasks(ptAllTaskTags[iLoopAllTasks],&iAllSubTasks,&ptAllSubTaskTags));
                for(int iLoopAllSubTasks = 0 ; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
                {	
                    char *pcSubTaskTypeName		= NULL;

                    CHECK_ITK(iRetCode,AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
                    if(tc_strcmp(pcSubTaskTypeName,"EPMSelectSignoffTask")==0)
                    {
                        int		iNumAttachs				= 0;
                        tag_t	*ptAttachsTagList		= NULL;

                        //get signoff attachments
                        CHECK_ITK(iRetCode,EPM_ask_attachments(ptAllSubTaskTags[iLoopAllSubTasks], EPM_signoff_attachment, &iNumAttachs, &ptAttachsTagList));
                        CHECK_ITK(iRetCode,AOM_ask_value_tag(ptAttachsTagList[0],"fnd0Performer",&tUser));
                        CHECK_ITK(iRetCode,POM_ask_user_default_group(tUser,&tGroup));
                        break;
                    }
                }
            }

        }

        /*getting target attachment*/
        for(int iInd = 0; iInd < iTgt; iInd++)
        {
            tag_t	tClass					= NULLTAG;
            tag_t	tRevClass				= NULLTAG;
            tag_t	tRev					= NULLTAG;
            logical lItemRevision			= false;

            tRev = ptTgt[iInd];

            CHECK_ITK(iRetCode,POM_class_of_instance(tRev , &tClass ));
            CHECK_ITK(iRetCode,POM_class_id_of_class("ItemRevision" , &tRevClass ));
            CHECK_ITK(iRetCode,POM_is_descendant(tRevClass, tClass , &lItemRevision ));
            
            /*check for itemrevision*/
            if(lItemRevision == true)
            {
                int     iMForm                  = 0;
                int     iRMForm                 = 0;
                tag_t	tItem					= NULLTAG;
                tag_t	tForm					= NULLTAG;
                tag_t	tHomeFolder				= NULLTAG;
                tag_t	*ptMForm				= NULL;
                tag_t	*ptRMForm				= NULL;

                vector<tag_t> tList;
                CHECK_ITK(iRetCode,ITEM_ask_item_of_rev(tRev , &tItem ));
                CHECK_ITK(iRetCode,GRM_find_relation_type("IMAN_master_form", &tForm));
                CHECK_ITK(iRetCode,GRM_list_secondary_objects_only (tItem, tForm, &iMForm, &ptMForm));
                CHECK_ITK(iRetCode,GRM_list_secondary_objects_only (tRev, tForm, &iRMForm, &ptRMForm));

                tList.push_back(tItem);
                tList.push_back(tRev);
                tList.push_back(ptMForm[0]);
                tList.push_back(ptRMForm[0]);

                for(int iCnt = 0; iCnt < tList.size(); iCnt++)
                {
                    CHECK_ITK(iRetCode,AOM_set_ownership(tList[iCnt],tUser,tGroup));
                    if(iRetCode != ITK_ok)
                    {
                        char	*pcItem		= NULL;

                        CHECK_ITK(iRetCode,ITEM_ask_id2(tItem,&pcItem));
                        TC_write_syslog("Failed to change ownership for Item %s\n",pcItem);
                    }
                }
            }
        }

    }

    return iRetCode;
}