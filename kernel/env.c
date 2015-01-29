/*
 * env.c
 * 
 * Author: Fernando Rodriguez
 * Email: frodriguez.developer@outlook.com
 * 
 * Copyright 2014-2015 Fernando Rodriguez
 * All rights reserved.
 * 
 * This code is distributed for educational purposes
 * only. It may not be distributed without written 
 * permission from the author.
 */

#include <arch/platform.h>
#include <mutex.h>
#include <kheap.h>
#include <scheduler.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>

/*
 * Free the environment variables of the current task
 */
void free_env(void)
{
	mutex_busywait(&current_task->env_lock);
	if (likely(current_task->envp != NULL))
	{
		char **envs;
		envs = current_task->envp;
		while (*envs != NULL)
		{
			free(*envs);
			envs++;
		}
		free(current_task->envp);
	}
	current_task->envp = NULL;	
	mutex_release(&current_task->env_lock);
}


/*
 * Unsets an environment variable
 */
int unsetenv(const char *name)
{
	char **envp, *envs;
	const char *namep;
	
	if (unlikely(name == NULL || strlen(name) == 0))
		return EINVAL;
	if (unlikely(current_task->envp == NULL))
		return 0;
	
	envp = current_task->envp;
	while (*envp != NULL)
	{
		envs = *envp;
		namep = name;
		while (*envs != NULL && *envs != '=' && *namep == NULL)
		{
			if (*envs != *namep)
				break;
			envs++;
			namep++;
		}
		
		if (likely(*envs == '=' && *namep == NULL))
		{
			free(*envp);
			while (envp[1] != NULL)
			{
				envp[0] = envp[1];
				envp++;
			}
			return 0;
		}
	}
	
	return 0;
}

/*
 * Sets and environment variable
 */
int setenv(const char *name, const char *value, int overwrite)
{
	char **envp;
	char *envs, *namep;
	
	/*
	 * TODO: Make sure we dont overrun the string table
	 */
	
	if (unlikely(name == NULL || strlen(name) == 0))
		return EINVAL;
	namep = (char*) name;
	while (*namep != NULL)
		if (unlikely(*namep++ == '='))
			return EINVAL;
	
	mutex_busywait(&current_task->env_lock);
	if (unlikely(current_task->envp == NULL))
	{
		current_task->envp = malloc(CONFIG_ENV_INITIAL_SIZE);
		if (unlikely(current_task->envp == NULL))
		{
			mutex_release(&current_task->env_lock);
			return ENOMEM;
		}
		memset(current_task->envp, 0, CONFIG_ENV_INITIAL_SIZE);
	}
	
	/*
	 * First check where the last entry is stored.
	 */
	envp = current_task->envp;
	while (*envp != NULL)
	{
		envs = *envp;
		namep = (char*) name;
		while (*envs != NULL && *envs != '=' && *namep != NULL)
		{
			if (*envs != *namep)
				break;	
			envs++;
			namep++;
		}
		if (*envs == '=' && *namep == NULL)
		{
			if (!overwrite)
			{
				mutex_release(&current_task->env_lock);
				return -1;
			}
			else 
			{
				free(*envp);
				break;
			}
		}
		envp++;
	}
	
	/* assert(*envp == NULL); */
	
	/*
	 * Update the entry and return
	 */
	envs = malloc(strlen(name) + strlen(value) + 2);
	if (unlikely(envs == NULL))
		return ENOMEM;
	*envp++ = envs;
	*envp = NULL;
	while (*name != NULL)
		*envs++ = *name++;
	*envs++ = '=';
	while (*value != NULL)
		*envs++ = *value++;
	*envs = NULL;
	mutex_release(&current_task->env_lock);
	return 0;
}

