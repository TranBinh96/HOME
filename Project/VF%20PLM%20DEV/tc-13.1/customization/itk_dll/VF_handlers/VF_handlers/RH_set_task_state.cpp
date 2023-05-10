/*******************************************************************************
File         : RH_check_task_state.cpp

Description  : To check state of all task
  
Input        : None
                        
Output       : None

Author       : Siemens

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Jan,17 2020     1.0         Kantesh		 Initial Creation

/*******************************************************************************/
#include "Vinfast_Custom.h"

EPM_decision_t RH_check_task_state(EPM_rule_message_t msg)
{
	int			iRetCode				= ITK_ok;
	tag_t		tRootTask				= NULLTAG;
	EPM_decision_t decision				= EPM_go;

	CHECK_ITK(iRetCode,EPM_ask_root_task(msg.task,&tRootTask));
	if(tRootTask != 0)
	{
		int			iAllTasks			= 0;
		tag_t		*ptAllTaskTags		= NULL;

		CHECK_ITK(iRetCode,EPM_ask_sub_tasks(tRootTask,&iAllTasks,&ptAllTaskTags));
		for(int iLoopAllTasks = 0 ; iLoopAllTasks < iAllTasks; iLoopAllTasks++)
		{
			char		*pcTaskState		= NULL;

			CHECK_ITK(iRetCode,AOM_ask_value_string(ptAllTaskTags[iLoopAllTasks],"task_state",&pcTaskState));
			/*if any of the task state is pending then do not start the task*/
			if(tc_strcmp("Started",pcTaskState) == 0)
			{
				decision = EPM_nogo;
				break;
			}
			else
			{
				continue;
			}
		}
	}

	return decision;
}