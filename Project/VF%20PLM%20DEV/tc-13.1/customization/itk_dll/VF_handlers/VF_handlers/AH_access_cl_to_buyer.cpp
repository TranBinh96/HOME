/*******************************************************************************
File         : AH_assign_access_cl_to_buyer.cpp

Description  : To provide access to buyer
  
Input        : None
						
Output       : None

Author       : Siemens

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
OCT,07 2019     1.0         Kantesh		 Initial Creation

/*******************************************************************************/
#include "Vinfast_Custom.h"

extern int AH_assign_access_cl_to_buyer(EPM_action_message_t msg)
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
		char		*pcUserID			= NULL;
		
		CHECK_ITK(iRetCode,EPM_ask_sub_tasks(tRootTask,&iAllTasks,&ptAllTaskTags));
		CHECK_ITK(iRetCode,EPM_ask_attachments(tRootTask,EPM_target_attachment,&iTgt,&ptTgt));
		/*getting user id of all participants from review task*/
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
						CHECK_ITK(iRetCode,SA_ask_user_identifier2(tUser,&pcUserID));
						break;
					}
				}
			}

		}

		/*getting person to set vf4_cl*/
		tag_t tPerson             = NULLTAG;
		char* personName          = NULL;
		std::string vf4BuyerValue =  "";
		if(tUser != NULLTAG)
		{
			CHECK_ITK(iRetCode, SA_ask_user_person(tUser, &tPerson));
			if(iRetCode == ITK_ok)
			{
				CHECK_ITK(iRetCode, SA_ask_person_name2(tPerson, &personName));
				if(iRetCode == ITK_ok)
				{
					vf4BuyerValue = std::string(personName) + " (" + std::string(pcUserID) + ")";
				}
			}
		}

		/*getting attachments*/
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
				tag_t	tWritePrev				= NULLTAG;
				tag_t	tReadPrev				= NULLTAG;
				tag_t	tAccessor				= NULLTAG;
				bool    isGrantSuccess          = false;
				/*getting accessor*/
				CHECK_ITK(iRetCode,AM_find_accessor("User",pcUserID,&tAccessor));
				if(iRetCode == ITK_ok)
				{
					CHECK_ITK(iRetCode,AOM_refresh(tRev,POM_modify_lock));
					if(iRetCode == ITK_ok)
					{
						CHECK_ITK(iRetCode,AM_find_privilege("WRITE", &tWritePrev));
						if(iRetCode == ITK_ok)
						{
							CHECK_ITK(iRetCode,AM_find_privilege("READ", &tReadPrev));
							if(iRetCode == ITK_ok)
							{
								/*provide read access*/
								CHECK_ITK(iRetCode,AM_grant_privilege(tRev, tAccessor , tReadPrev));
								if(iRetCode == ITK_ok)
								{
									/*provide write access*/
									CHECK_ITK(iRetCode,AM_grant_privilege(tRev, tAccessor , tWritePrev));
									if(iRetCode == ITK_ok)
									{
										CHECK_ITK(iRetCode,AM_save_acl(tRev));
										if(iRetCode == ITK_ok)
										{
											isGrantSuccess = true;
											CHECK_ITK(iRetCode,AOM_unlock(tRev));
										}
										else
										{
											TC_write_syslog("Failed to save ACL \n");
											CHECK_ITK(iRetCode,AOM_unlock(tRev));
										}
									}
									else
									{
										TC_write_syslog("Failed to grant Read Access for given user %s\n",pcUserID);
										CHECK_ITK(iRetCode,AOM_unlock(tRev));
									}
								}
								else
								{
									TC_write_syslog("Failed to grant Read Access for given user %s\n",pcUserID);
									CHECK_ITK(iRetCode,AOM_unlock(tRev));
								}
							}
						}
					}
					else
					{
						TC_write_syslog("Failed to lock for modify\n");
					}
				}
				else
				{
					TC_write_syslog("Failed to get Accessor for given user %s\n",pcUserID);
				}
				/*set value for vf4_buyer*/
				char* objType = NULL;
				if(isGrantSuccess)
				{
					CHECK_ITK(iRetCode, WSOM_ask_object_type2(tRev,&objType));
					if(std::string (objType).compare("VF4_line_itemRevision") == 0||
						std::string (objType).compare("VF4_Bus_LineItmRevision") == 0||
						std::string (objType).compare("VF4_Car_LineItmRevision") == 0||
						std::string (objType).compare("VF4_scp_line_itmRevision") == 0)
					{
						CHECK_ITK(iRetCode, AOM_refresh(tRev, true));
						CHECK_ITK(iRetCode, AOM_set_value_string(tRev, "vf4_buyer", vf4BuyerValue.c_str()));
						CHECK_ITK(iRetCode, AOM_save_with_extensions(tRev));
						CHECK_ITK(iRetCode, AOM_refresh(tRev, false));
					}
				}
				
			}
		}

	}

	return iRetCode;
}