/*******************************************************************************
File         : AH-assign-workflow-tasks.cpp

Description  : To assign workflow tasks based on preference

Input        : None

Output       : None

Revision History :
Date            Revision    Who              Description
-----------------------------------------------------------------------------
Feb,10 2022     1.0         Pallavi		 Initial Creation

/*******************************************************************************/
#include "Vinfast_Custom.h"
#include <map>
#include <iterator>
#include <base_utils/ScopedSmPtr.hxx>
#include <base_utils/ScopedPtr.hxx>

#define Error (EMH_USER_error_base +280)
int assignTask(tag_t tRootTask, string taskList, string reviewerList);

extern int AH_assign_workflow_tasks(EPM_action_message_t msg)
{
	TC_write_syslog("[VF] ENTER %s", __FUNCTION__);

	int			iRetCode = ITK_ok;
	int			iNumArgs = 0;
	int			iPrefCount = 0;
	int         valCount = 0;
	tag_t		tRootTask = NULLTAG;
	char		ErrorMessage[2048] = "";

	Teamcenter::scoped_smptr<char*> prefValues;
	Teamcenter::scoped_smptr<char> extracted_value;

	iNumArgs = TC_number_of_arguments(msg.arguments);
	TC_write_syslog("TC_number_of_arguments %d\n", iNumArgs);
	if (iNumArgs > 0)
	{
		for (int i = 0; i < iNumArgs; i++)
		{
			Teamcenter::scoped_smptr<char> pcFlag;
			Teamcenter::scoped_smptr<char> pcValue;

			/*getting arguments*/
			CHECK_ITK(iRetCode, ITK_ask_argument_named_value(TC_next_argument(msg.arguments), &pcFlag, &pcValue));
			if (iRetCode == ITK_ok)
			{
				if (tc_strcmp(pcFlag.getString(), "preference_name") == 0 && pcValue != NULL)
				{
					CHECK_ITK(iRetCode, EPM_substitute_keyword(pcValue.get(), &extracted_value));
				}
			}
		}
	}
	else
	{
		iRetCode = EPM_invalid_argument;
	}
	CHECK_ITK(iRetCode, PREF_ask_char_values(extracted_value.getString(), &valCount, &prefValues));

	/*getting root task*/
	CHECK_ITK(iRetCode, EPM_ask_root_task(msg.task, &tRootTask));
	if (tRootTask != 0)
	{
		int	 iAllTasks = 0;
		int	 iTgt = 0;

		multimap<int, string> rankMap;
		Teamcenter::scoped_smptr<tag_t> ptTgt;
		Teamcenter::scoped_smptr<tag_t> ptAllTaskTags;

		CHECK_ITK(iRetCode, EPM_ask_attachments(tRootTask, EPM_target_attachment, &iTgt, &ptTgt));
		//Looping the targets
		logical check = false;
		for (int iLoopTarget = 0; iLoopTarget < iTgt; iLoopTarget++)
		{
			int  propCount = 0;
			Teamcenter::scoped_smptr<char> objType;
			Teamcenter::scoped_smptr<char*> properties;

			CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iLoopTarget], "object_type", &objType));
			if (tc_strcmp(objType.getString(), prefValues[0]) == 0)
			{
				string final = "";
				boolean matchFlag = false;

				CHECK_ITK(iRetCode, EPM__parse_string(prefValues[1], ";", &propCount, &properties));
				//reading the properties from targets
				for (int iLoopCondProp = 0; iLoopCondProp < propCount; iLoopCondProp++)
				{
					Teamcenter::scoped_smptr<char> propValue;
					Teamcenter::scoped_smptr<char> propCheck;
					propCheck = strstr(properties[iLoopCondProp], "%");
					if (propCheck != NULL)
					{
						int  tokenCount = 0;
						tag_t tItem = NULLTAG;
						tag_t classId = NULLTAG;
						logical itemExists = false;

						Teamcenter::scoped_smptr<char> propValue;
						Teamcenter::scoped_smptr<char*> tokenValue;
						Teamcenter::scoped_smptr<char> className;

						CHECK_ITK(iRetCode, ITEM_ask_item_of_rev(ptTgt[iLoopTarget], &tItem));
						CHECK_ITK(iRetCode, POM_class_of_instance(tItem, &classId));
						CHECK_ITK(iRetCode, POM_name_of_class(classId, &className));
						CHECK_ITK(iRetCode, EPM__parse_string(properties[iLoopCondProp], ".", &tokenCount, &tokenValue));
						CHECK_ITK(iRetCode, POM_attr_exists(tokenValue[1], className.getString(), &itemExists));
						if (itemExists)
						{
							CHECK_ITK(iRetCode, AOM_ask_value_string(tItem, tokenValue[1], &propValue));
							if (tc_strcmp(propValue.getString(), "") != 0)
							{
								if (iLoopCondProp == propCount - 1)
								{
									final.append(propValue.getString());
								}
								else
								{
									final.append(propValue.getString());
									final.append(";");
								}
							}
						}
						else
						{
							sprintf(ErrorMessage, "Cannot find attribute: %s in the Item object", tokenValue[1]);
							CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
							return Error;
						}

					}
					else
					{
						tag_t classId = NULLTAG;
						Teamcenter::scoped_smptr<char> className;
						logical itemRevisionExists = false;
						logical test = true;
						CHECK_ITK(iRetCode, POM_attr_exists("%item%.vf6_impacted_module", "Vf6_ECRRevision", &test));

						CHECK_ITK(iRetCode, POM_class_of_instance(ptTgt[iLoopTarget], &classId));
						CHECK_ITK(iRetCode, POM_name_of_class(classId, &className));
						CHECK_ITK(iRetCode, POM_attr_exists(properties[iLoopCondProp], className.getString(), &itemRevisionExists));

						/*sprintf(ErrorMessage, "abc: %s", ptTgt[iLoopTarget]);
						sprintf(ErrorMessage, "abc: %s", classId);
						sprintf(ErrorMessage, "abc: %s", className.getString());*/

						if (itemRevisionExists)
						{
							CHECK_ITK(iRetCode, AOM_ask_value_string(ptTgt[iLoopTarget], properties[iLoopCondProp], &propValue));
							if (tc_strcmp(propValue.getString(), "") != 0)
							{
								if (iLoopCondProp == propCount - 1)
								{
									final.append(propValue.getString());
								}
								else
								{
									final.append(propValue.getString());
									final.append(";");
								}
							}

						}
						else
						{
							sprintf(ErrorMessage, "Cannot find attribute: %s in the target object %s", properties[iLoopCondProp], className.getString());
							CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
							return Error;
						}
					}
				}
				//looping from 4th line onwards for ranking

				for (int i = 3; i < valCount; i++)
				{
					int  rank = 0;
					int  condnCount = 0;
					int  finalCount = 0;
					int  inputCount = 0;

					Teamcenter::scoped_smptr<char*> condnValue;
					Teamcenter::scoped_smptr<char*> finalValue;
					Teamcenter::scoped_smptr<char*> inputValue;

					CHECK_ITK(iRetCode, EPM__parse_string(prefValues[i], "=", &condnCount, &condnValue));
					CHECK_ITK(iRetCode, EPM__parse_string(final.c_str(), ";", &finalCount, &finalValue));
					CHECK_ITK(iRetCode, EPM__parse_string(condnValue[0], ";", &inputCount, &inputValue));
					for (int j = 0; j < finalCount; j++)
					{
						for (int k = j; k < inputCount; k++)
						{
							if (tc_strcmp(finalValue[j], inputValue[k]) == 0)
							{
								rank = rank + 1;
							}
							break;
						}
					}
					rankMap.insert(pair<int, string>(rank, condnValue[1]));
				}
				//processing task assignment on basis of highest Rank
				if (rankMap.rbegin()->first > 0)
				{
					matchFlag = true;
					CHECK_ITK(iRetCode, assignTask(tRootTask, prefValues[2], rankMap.rbegin()->second));
				}
				if (!matchFlag)
				{
					sprintf(ErrorMessage, "Cannot find matched assignment list for this target");
					CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
					return Error;
				}
				check = true;
				break;
			}
		}

		if (!check)
		{
			sprintf(ErrorMessage, "Cannot find object_type: %s in target object", prefValues[0]);
			CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
			return Error;
		}
	}
	TC_write_syslog("[VF] EXIT %s", __FUNCTION__);
	return iRetCode;
}
int assignTask(tag_t tRootTask, string taskList, string reviewerList)
{
	int	 iRetCode = ITK_ok;
	int  taskCount = 0;
	int  reviewerCount = 0;
	char ErrorMessage[2048] = "";
	Teamcenter::scoped_smptr<char*> taskName;
	Teamcenter::scoped_smptr<char*> reviewerIds;

	CHECK_ITK(iRetCode, EPM__parse_string(taskList.c_str(), ";", &taskCount, &taskName));
	CHECK_ITK(iRetCode, EPM__parse_string(reviewerList.c_str(), ";", &reviewerCount, &reviewerIds));

	//reading the properties from targets
	for (int iLooptasks = 0; iLooptasks < taskCount; iLooptasks++)
	{
		tag_t  ptSubTask = NULLTAG;
		CHECK_ITK(iRetCode, EPM_ask_sub_task(tRootTask, taskName[iLooptasks], &ptSubTask));
		if (ptSubTask != NULLTAG)
		{
			for (int iLooprew = iLooptasks; iLooprew < reviewerCount; iLooprew++)
			{
				int  userCount = 0;
				Teamcenter::scoped_smptr<char*> userIds;
				CHECK_ITK(iRetCode, EPM__parse_string(reviewerIds[iLooprew], ",", &userCount, &userIds));
				for (int iLoopUser = 0; iLoopUser < userCount; iLoopUser++)
				{
					Teamcenter::scoped_smptr<char> pcTaskTypeName;
					Teamcenter::scoped_smptr<char> pcTaskName;
					Teamcenter::scoped_smptr<char> pcTaskState;
					tag_t user_tag = NULLTAG;

					CHECK_ITK(iRetCode, SA_find_user2(userIds[iLoopUser], &user_tag));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptSubTask, "task_type", &pcTaskTypeName));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptSubTask, "current_name", &pcTaskName));
					CHECK_ITK(iRetCode, AOM_ask_value_string(ptSubTask, "task_state", &pcTaskState));
					if (user_tag != NULLTAG)
					{
						if (tc_strcmp(pcTaskTypeName.getString(), "EPMReviewTask") == 0 && tc_strcmp(pcTaskState.getString(), "Completed") != 0)
						{
							int	iAllSubTasks = 0;
							Teamcenter::scoped_smptr<tag_t> ptAllSubTaskTags;

							CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptSubTask, &iAllSubTasks, &ptAllSubTaskTags));
							for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
							{
								Teamcenter::scoped_smptr<char> pcSubTaskTypeName;
								CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
								if (tc_strcmp(pcSubTaskTypeName.getString(), "EPMSelectSignoffTask") == 0)
								{
									int     signoffCount = 0;
									Teamcenter::scoped_smptr<tag_t> signoffs;

									CHECK_ITK(iRetCode, EPM_create_adhoc_signoff(ptAllSubTaskTags[iLoopAllSubTasks], user_tag, &signoffCount, &signoffs));
									CHECK_ITK(iRetCode, EPM_set_adhoc_signoff_selection_done(ptAllSubTaskTags[iLoopAllSubTasks], TRUE));
								}
							}
						}

						//assigning user to do task
						if (tc_strcmp(pcTaskTypeName.getString(), "EPMDoTask") == 0 && tc_strcmp(pcTaskState.getString(), "Completed") != 0)
						{
							CHECK_ITK(iRetCode, EPM_assign_responsible_party(ptSubTask, user_tag));
						}
						//assigning user to ConditionTask
						if (tc_strcmp(pcTaskTypeName.getString(), "EPMConditionTask") == 0 && tc_strcmp(pcTaskState.getString(), "Completed") != 0)
						{
							CHECK_ITK(iRetCode, EPM_assign_responsible_party(ptSubTask, user_tag));
						}
						//Acknowledge Task
						if (tc_strcmp(pcTaskTypeName.getString(), "EPMAcknowledgeTask") == 0 && tc_strcmp(pcTaskState.getString(), "Completed") != 0)
						{
							int	iAllSubTasks = 0;
							Teamcenter::scoped_smptr<tag_t> ptAllSubTaskTags;

							CHECK_ITK(iRetCode, EPM_ask_sub_tasks(ptSubTask, &iAllSubTasks, &ptAllSubTaskTags));
							for (int iLoopAllSubTasks = 0; iLoopAllSubTasks < iAllSubTasks; iLoopAllSubTasks++)
							{
								Teamcenter::scoped_smptr<char> pcSubTaskTypeName;
								CHECK_ITK(iRetCode, AOM_ask_value_string(ptAllSubTaskTags[iLoopAllSubTasks], "task_type", &pcSubTaskTypeName));
								if (tc_strcmp(pcSubTaskTypeName.getString(), "EPMSelectSignoffTask") == 0)
								{
									int     signoffCount = 0;
									Teamcenter::scoped_smptr<tag_t> signoffs;

									CHECK_ITK(iRetCode, EPM_create_adhoc_signoff(ptAllSubTaskTags[iLoopAllSubTasks], user_tag, &signoffCount, &signoffs));
									CHECK_ITK(iRetCode, EPM_set_adhoc_signoff_selection_done(ptAllSubTaskTags[iLoopAllSubTasks], TRUE));
								}
							}
						}
					}
					else
					{
						sprintf(ErrorMessage, "Cannot find user: %s or user is inactive", userIds[iLoopUser]);
						CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
						return Error;
					}
				}
				break;
			}
		}
		else
		{
			sprintf(ErrorMessage, "Cannot find task: %s in the workflow", taskName[iLooptasks]);
			CHECK_ITK(iRetCode, EMH_store_error_s1(EMH_severity_error, Error, ErrorMessage));
			return Error;
		}
	}
	return iRetCode;
}