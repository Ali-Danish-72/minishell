/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maabdull <maabdull@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 13:43:49 by maabdull          #+#    #+#             */
/*   Updated: 2024/05/04 13:08:59 by maabdull         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include "minishell.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int		g_status_code;

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

/*
 * Creates a child process and executes the specified command
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
			return (fprintf(stderr, "%s: command not found\n", cmd_original), 127);
		cmd[0] = absolute_cmd;
	}
	else
	{
		if (access(cmd[0], F_OK) == -1)
			return (fprintf(stderr, "%s: no such file or directory\n",
					cmd_original), 127);
		else if (access(cmd[0], X_OK) == -1)
			return (fprintf(stderr, "%s: permission denied\n", cmd_original),
				126);
	}
	pid = fork();
	if (pid == 0)
	{
		execve(cmd[0], cmd, env);
		fprintf(stderr, "%s: command not found\n", cmd_original);
		free(cmd_original);
		exit(127);
	}
	waitpid(pid, &status, 0);
	free(cmd_original);
	// free(absolute_cmd);
	return (status);
}

void	handle_sigint(int signum)
{
	(void)signum;
	puts("");
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
	g_status_code = 130;
}

/**
 * @brief Return a prompt with the shells name and the users location
 *
 * TODO: Use the minishell data structure as input to keep track of previous
 * location
 */
char	*update_prompt(void)
{
	int		i;
	char	*temp;
	char	*prompt;
	char	*home_dir;
	char	*current_dir;

	prompt = YELLOW "msh" RESET "[" B_YELLOW;
	current_dir = getcwd(NULL, 0);
	home_dir = getenv("HOME");
	i = 0;
	if (home_dir && current_dir)
	{
		i = ft_strlen(home_dir);
		if (ft_strncmp(current_dir, home_dir, i) == 0)
		{
			prompt = ft_strjoin(prompt, "~");
			temp = prompt;
			prompt = ft_strjoin(prompt, current_dir + i);
			free(temp);
		}
		else
			prompt = ft_strjoin(prompt, current_dir + i);
	}
	else
		prompt = ft_strjoin(prompt, current_dir + i);
	temp = prompt;
	prompt = ft_strjoin(prompt, RESET "] ");
	free(temp);
	free(current_dir);
	return (prompt);
}

int	count_quotations(char *line)
{
	int	count;
	int	i;
	char	quotes_found;

	i = -1;
	count = 0;
	quotes_found = '\0';
	while (line[++i])
	{
		if (line[i] == '"')
		{
			if (!quotes_found)
			{
				quotes_found = '"';
				count++;
			}
			else if (quotes_found == '"')
			{
				quotes_found = '\0';
				count++;
			}
			continue ;
		}
		if (line[i] == '\'')
		{
			if (!quotes_found)
			{
				quotes_found = '\'';
				count++;
			}
			else if (quotes_found == '\'')
			{
				quotes_found = '\0';
				count++;
			}
			continue ;
		}
	}
	return (count);
}

/**
 * @brief Counts how many tokens there are in the provided input string
 * Uses spaces as a delimiter but ignores them in quoted strings
 */
int	count_tokens(char *input)
{
	int		i;
	int		j;
	bool	space_found;
	char	quotes_found;

	i = 0;
	j = 0;
	quotes_found = '\0';
	space_found = false;
	while (*input == ' ')
		input++;
	while (input[i])
	{
		if (input[i] == '"')
		{
			if (!quotes_found)
				quotes_found = '"';
			else if (quotes_found == '"')
				quotes_found = '\0';
		}
		if (input[i] == '\'')
		{
			if (!quotes_found)
				quotes_found = '\'';
			else if (quotes_found == '\'')
				quotes_found = '\0';
		}
		if (!quotes_found && !space_found && input[i] == ' ')
		{
			if (input[i + 1] && input[i + 1] != ' ')
			{
				j++;
				space_found = true;
			}
		}
		if (!quotes_found && input[i] != ' ')	
			space_found = false;
		i++;
	}
	if (i > 0)
		j++;
	return (j);
}

int	get_token_type(char *content)
{
	if (!content || !content[0])
		return (ERR);
	if (!content[1])
	{
		if (content[0] == '|')
			return (PIPE);
		if (content[0] == '>')
			return (GREAT);
		if (content[0] == '<')
			return (LESS);
	}
	else if (!content[2])
	{
		if (content[0] == '>' && content[1] == '>')
			return (DBL_GREAT);
		if (content[0] == '<' && content[1] == '<')
			return (DBL_LESS);
	}
	return (WORD);
}

/**
 * @brief Skip the leading whitespace characters by moving the pointer to the
 * head of the string ahead.
 * Move the pointer to the head of the string to the start of the next token
 * or '\0' if there isn't one and return this token.
 *
 * @usage
 *
 * Calling the function with the input:
 * - '       This token '
 *
 * will result in the function returning "This" and input pointing to 't' in
 * 'token'.
 *
 * Calling the function with the input:
 * - 'token'
 *
 * will result in the function returning "token" and input pointing to '\0'.
 */
char	*get_token(char	**input)
{
	int	i;
	char	*token;
	char	*string;
	char	quotes_found;
	bool	space_found;

	i = 0;
	quotes_found = '\0';
	space_found = false;
	token = NULL;
	string = *input;
	while (*string == ' ')
		string++;
	while (string[i])
	{
		// TODO: Move quote checking to a dedicated separate function
		if (string[i] == '"')
		{
			if (!quotes_found)
				quotes_found = '"';
			else if (quotes_found == '"')
				quotes_found = '\0';
		}
		if (string[i] == '\'')
		{
			if (!quotes_found)
				quotes_found = '\'';
			else if (quotes_found == '\'')
				quotes_found = '\0';
		}
		// TODO: Change from space only to is_whitespace function
		if (!quotes_found && !space_found && i > 0 && string[i] == ' ')
		{
			space_found = true;
			break ;
		}
		if (space_found && !quotes_found && string[i] != ' ')	
			space_found = false;
		i++;
	}
	token = malloc(i + 1);
	if (!token)
		return (NULL);
	ft_strlcpy(token, string, i + 1);
	// token[i] = '\0';
	string += i;
	while (*string == ' ')
		string++;
	*input = string;
	return (token);
}

// TODO: Remove debug function
void	print_token(t_token token)
{
	char	*type;

	switch (token.type) {
		case PIPE:
			type = "|";
			break ;
		case LESS:
			type = "<";
			break ;
		case DBL_LESS:
			type = "<<";
			break ;
		case GREAT:
			type = ">";
			break ;
		case DBL_GREAT:
			type = ">>";
			break ;
		default:
			type = "Word";
			break ;
	}
	puts("{");
	printf("\tContent: %s,\n\tType: %s\n", token.content, type);
	puts("}");
}

t_token	*tokenize(t_minishell *minishell, char *input)
{
	t_token	*tokens;
	int	i;
	int	token_count;

	i = 0;
	token_count = count_tokens(input);
	minishell->token_count = token_count;
	tokens = malloc((token_count + 1) * sizeof(t_token));
	while (i < token_count)
	{
		tokens[i].content = get_token(&input);
		// puts(tokens[i].content);
		tokens[i].type = get_token_type(tokens[i].content);
		// print_token(tokens[i]);
		i++;
	}
	tokens[i].content = NULL;
	return (tokens);
}

t_cmd	*create_exec_cmd(t_minishell *minishell)
{
	t_cmd_exec	*exec_cmd;
	t_token	*tokens;
	int	i;

	i = 0;
	exec_cmd = malloc(sizeof(t_cmd_exec));
	exec_cmd->type = CMD_EXEC;
	exec_cmd->tokens = malloc((minishell->token_count + 1) * sizeof(char *));
	tokens = minishell->tokens;
	while (tokens[i].content)
	{
		exec_cmd->tokens[i] = ft_strdup(tokens[i].content);
		i++;
	}
	exec_cmd->tokens[i] = NULL;
	// ft_printarr(exec_cmd->tokens);
	return ((t_cmd *) exec_cmd);
}

void	parse(t_minishell *minishell, char *line)
{
	if (count_quotations(line) % 2 != 0)
		fprintf(stderr, RED "Open quotes detected, command rejected.\n" RESET);
	minishell->tokens = tokenize(minishell, line);
}

bool	is_builtin(char *str)
{
	char	*builtins[8];
	int		i;

	i = 0;
	builtins[0] = "echo";
	builtins[1] = "cd";
	builtins[2] = "pwd";
	builtins[3] = "export";
	builtins[4] = "unset";
	builtins[5] = "env";
	builtins[6] = "exit";
	builtins[7] = NULL;
	while (builtins[i])
	{
		if (ft_strncmp(str, builtins[i], ft_strlen(builtins[i])) == 0)
			return (true);
		i++;
	}
	return (false);
}

// TODO: Fix this temporary solution
void	echo(char **cmd)
{
	int		i;
	bool	display_newline;

	i = 0;
	display_newline = true;
	if (ft_strncmp(cmd[i], "-n", 2) == 0)
	{
		i++;
		display_newline = false;
	}
	while (cmd[i])
	{
		printf("%s ", cmd[i]);
		i++;
	}
	if (display_newline)
		puts("");
}

int	cd(char **cmd)
{
	if (cmd[1])
		return (fprintf(stderr, "cd: too many arguments\n"), 1);
	chdir(cmd[0]);
	return (0);
}

void	exec_builtin(char **cmd)
{
	if (ft_strncmp(cmd[0], "echo", 4) == 0)
		echo(cmd + 1);
	if (ft_strncmp(cmd[0], "cd", 2) == 0)
		g_status_code = cd(cmd + 1);
}

void	free_tokens(t_minishell *minishell)
{
	int	i;

	i = 0;
	while (minishell->tokens[i].content)
	{
		free(minishell->tokens[i].content);
		i++;
	}
	free(minishell->tokens);
}

void	run_cmd(t_cmd *cmd, char **env)
{
	t_cmd_exec	*cmd_exec;

	if (cmd->type == CMD_EXEC)
	{
		cmd_exec = (t_cmd_exec *) cmd;
		exec_cmd(cmd_exec->tokens, env);
	}
}

void	free_cmd(t_cmd *cmd)
{
	t_cmd_exec	*cmd_exec;
	int	i;

	i = 0;
	if (cmd->type == CMD_EXEC)
	{
		cmd_exec = (t_cmd_exec *) cmd;
		while (cmd_exec->tokens[i])
		{
			free(cmd_exec->tokens[i]);
			i++;
		}
		free(cmd_exec->tokens);
		free(cmd_exec);
	}
}

/*
 * Loops until EOF is detected and reads user input using readline
 * and executes the command in a child process and finally
 * frees the user read line.
 * NOTE: Readline causes still reachable memory leaks
 */
int	main(int argc, char *argv[] __attribute__((unused)), char **env)
{
	char	*line;
	char	*prompt;
	t_minishell minishell;
	t_cmd	*cmd;

	// (void)env;
	if (argc != 1)
		return (puts("Minishell can not run external files."), 1);
	g_status_code = 0;
	signal(SIGINT, handle_sigint);
	prompt = update_prompt();
	line = readline(prompt);
	while (line)
	{
		add_history(line);
		parse(&minishell, line);
		// tokens = ft_split(line, ' ');
		// if (is_builtin(tokens[0]))
		// 	exec_builtin(tokens);
		// else
		// 	exec_cmd(tokens, env);
		cmd = create_exec_cmd(&minishell);
		run_cmd(cmd, env);
		free_cmd(cmd);
		free(line);
		free_tokens(&minishell);
		free(prompt);
		prompt = update_prompt();
		line = readline(prompt);
	}
	free(prompt);
	puts("exit");
	return (g_status_code);
}
