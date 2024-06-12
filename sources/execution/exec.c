/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maabdull <maabdull@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/04 14:47:16 by maabdull          #+#    #+#             */
/*   Updated: 2024/06/11 22:24:51 by maabdull         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Mallocs the full path to a command
// or returns NULL if no command was found
char	*find_cmd(char *cmd)
{
	char	*final_cmd;
	char	**dirs;
	char	*path;
	int		j;

	final_cmd = NULL;
	path = getenv("PATH");
	if (!path)
		return (NULL);
	dirs = ft_split(path, ':');
	j = 0;
	while (dirs[j])
	{
		final_cmd = ft_char_strjoin(dirs[j], cmd, '/');
		if (access(final_cmd, X_OK) == 0)
			break ;
		free(final_cmd);
		final_cmd = NULL;
		j++;
	}
	ft_free_2d_arr(dirs);
	return (final_cmd);
}

char	**convert_cmd_exec(t_token_node *tokens)
{
	int	i;
	t_token_node *current;
	char	**str_tokens;

	i = 0;
	current = tokens;
	while (current)
	{
		i++;
		current = current->next;
	}
	str_tokens = malloc(sizeof(char *) * (i + 1));
	// TODO: Handle malloc fail properly
	if (!str_tokens)
		exit(1);
	i = 0;
	current = tokens;
	while (current)
	{
		str_tokens[i] = ft_strdup(current->current->content);
		current = current->next;
		i++;
	}
	str_tokens[i] = NULL;
	return (str_tokens);
}

/**
 * TODO: Check if command is builtin here
 */
void	run_cmd(t_cmd *cmd, char **env)
{
	t_cmd_exec	*cmd_exec;

	if (cmd->type == CMD_EXEC)
	{
		cmd_exec = (t_cmd_exec *) cmd;
		// if (is_builtin(cmd_exec->tokens[0].current->content))
		// 	exec_builtin(cmd_exec->tokens);
		// else
		exec_cmd(convert_cmd_exec(cmd_exec->tokens), env);
	}
}

/**
 * @brief Creates a child process and executes the specified command
 * Accepts an array of strings as input that will be passed to execve
 * following the format:
 * {command, options}
 */
int	exec_cmd(char **cmd, char **env)
{
	int		status;
	int		pid;
	char	*absolute_cmd;
	char	*cmd_original;

	cmd_original = cmd[0];
	if (!ft_strchr(cmd[0], '/'))
	{
		absolute_cmd = find_cmd(cmd[0]);
		if (!absolute_cmd)
		{
			ft_putstr_fd(cmd_original, 2);
			ft_putendl_fd(": command not found", 2);
			return (127);
		}
		cmd[0] = absolute_cmd;
	}
	else
	{
		if (access(cmd[0], F_OK) == -1)
		{
			ft_putstr_fd(cmd_original, 2);
			ft_putendl_fd(": no such file or directory", 2);
			return (127);
		}
		else if (access(cmd[0], X_OK) == -1)
		{
			ft_putstr_fd(cmd_original, 2);
			ft_putendl_fd(": Permission denied", 2);
			return (126);
		}
	}
	pid = fork();
	if (pid == 0)
	{
		execve(cmd[0], cmd, env);
		ft_putstr_fd(cmd_original, 2);
		ft_putendl_fd(": command not found", 2);
		free(cmd_original);
		exit(127);
	}
	waitpid(pid, &status, 0);
	free(cmd_original);
	// free(absolute_cmd);
	return (status);
}

// void	exec_builtin(char **cmd)
// {
// 	if (ft_strncmp(cmd[0], "echo", 5) == 0)
// 		ft_echo(cmd + 1);
// 	if (ft_strncmp(cmd[0], "cd", 3) == 0)
// 		g_status_code = ft_cd(cmd + 1);
// }
