/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdanish <mdanish@student.42abudhabi.ae>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/04 17:24:25 by maabdull          #+#    #+#             */
/*   Updated: 2024/05/13 18:59:37 by mdanish          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# define PROMPT "minishell> "
# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define BLUE "\033[34m"
# define B_YELLOW "\033[33;1m"
# define RESET "\033[0m"

# include "libft.h"
# include <readline/history.h>
# include <readline/readline.h>
# include <signal.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/wait.h>
# include <termios.h>
# include <time.h>
# include <unistd.h>

/** STRUCTURES **/

typedef struct s_cmd t_cmd;
typedef struct s_cmd_exec t_cmd_exec;
typedef struct s_env_node t_env_node;
typedef struct s_minishell t_minishell;
typedef struct s_prompt t_prompt;
typedef struct s_token t_token;

enum e_token_types
{
	PIPE,
	LESS,
	DBL_LESS,
	GREAT,
	DBL_GREAT,
	WORD,
	ERR
};

enum e_cmd_types
{
	CMD_EXEC
};

struct s_token
{
	int			type;
	char		*content;
};

struct s_prompt
{
	char		*previous;
	char		*current;
};

struct s_env_node
{
	char		*env_name;
	char		*env_content;
	bool		env_print;
	t_env_node	*next;
};

struct s_minishell
{
	int			token_count;
	t_token		*tokens;
	t_prompt	*prompt;
	t_env_node	*env_variables;
};

/*
 * General command structure.
 *
 * Every parse function returns this and can be casted to another cmd type
 * because they all share the type variable
 * */
struct s_cmd
{
	int			type;
};

struct s_cmd_exec
{
	int			type;
	char		**tokens;
};

/** GLOBAL VARIABLE **/
extern int		g_status_code;

/** FUNCTIONS **/
// Parsing
t_cmd			*create_exec_cmd(t_minishell *minishell);
void			parse(t_minishell *minishell, char *line);

// Execution
void			exec_builtin(char **cmd);
int				exec_cmd(char **cmd, char **env);
void			run_cmd(t_cmd *cmd, char **env);

// Built-ins
int				cd(char **cmd);
void			echo(char **cmd);
bool			is_builtin(char *str);

// Cleanup
void			free_cmd(t_cmd *cmd);
void			free_tokens(t_minishell *minishell);

// General
t_prompt		*init_prompt(void);
void			update_prompt(t_prompt *prompt);

// Miscellaneous
void	ft_lstadd_back(t_env_node **list, t_env_node *new_node);

#endif
