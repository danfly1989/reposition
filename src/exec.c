/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daflynn <daflynn@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 12:14:06 by daflynn           #+#    #+#             */
/*   Updated: 2025/09/16 12:14:14 by daflynn          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	find_last_index(t_dat *d, char ***cmd)
{
	int	i;

	i = d->tot - 1;
	while (i >= 0)
	{
		if (cmd[i] && cmd[i][0])
			return (i);
		i--;
	}
	return (-1);
}

void	ft_execute_pipeline(t_dat *d, char ***cmd)
{
	int		**fd;
	pid_t	*pids;
	int		last_index;

	last_index = find_last_index(d, cmd);
	if (last_index == -1)
		return ;
	if (ft_strcmp(cmd[0][0], "./minishell") == 0)
	{
		ft_nested_minishell(d, cmd[0], NULL);
		return ;
	}
	fd = init_fd_array(d->tot);
	if (!fd || !ft_create_pipes(fd, d->tot))
	{
		if (fd)
			ft_free_fd(fd);
		return ;
	}
	pids = ft_fork_children(d, cmd, fd);
	ft_close_pipes(fd, d->tot);
	ft_wait_children(pids, d->tot, last_index);
	ft_set_main_signals();
	ft_free_fd(fd);
	free(pids);
}

/*Static to remind that this is a split from
exec command*/
static void	ft_execve_error(char *cmd)
{
	int	code;

	if (errno == EACCES || errno == EISDIR)
		code = 126;
	else if (errno == ENOENT)
		code = 127;
	else
		code = 1;
	ft_putstr_fd(cmd, 2);
	ft_putstr_fd(": ", 2);
	ft_putendl_fd(strerror(errno), 2);
	exit(code);
}

void	ft_exec_command(t_dat *d, char **cmd)
{
	char	*cmd_path;

	if (!cmd || !cmd[0] || !*cmd[0])
		exit(127);
	if (ft_is_pipe_builtin(cmd[0]))
	{
		ft_execute_builtin_in_child(d, cmd);
		exit(g_last_exit_status);
	}
	cmd_path = ft_get_cmd_path(d, cmd[0], 0);
	if (!cmd_path)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmd[0], 2);
		ft_putendl_fd(": command not found", 2);
		exit(127);
	}
	execve(cmd_path, cmd, d->evs);
	free(cmd_path);
	ft_execve_error(cmd[0]);
}

void	ft_external_functions(t_dat *data, char *line)
{
	char	***cmd;

	(void)line;
	if (!data || !data->xln || !data->xln[0])
		return ;
	if (!ft_validate_syntax(data->xln))
		return ;
	ft_list_to_env_array(data);
	data->no_pipes = ft_count_pipes(data->xln);
	if (!data->no_pipes && !ft_count_redirections(data->xln))
	{
		if (ft_all_valid_lvar(data, data->xln))
			ft_update_local_variables(data);
		if (ft_handle_builtin(data, data->st))
			return ;
	}
	cmd = ft_parse_cmd(data, 0, 0, 0);
	if (!cmd)
		return ;
	ft_execute_pipeline(data, cmd);
	ft_clean_cmd(cmd);
	ft_free_string_array(data->evs);
	data->evs = NULL;
}
