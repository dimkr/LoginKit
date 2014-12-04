#ifndef _LOGINKIT_H_INCLUDED
#	define _LOGINKIT_H_INCLUDED

#	include <sys/types.h>

int sd_pid_get_session(pid_t pid, char **session);
int sd_pid_get_owner_uid(pid_t pid, uid_t *uid);
int sd_pid_get_machine_name(pid_t pid, char **name);
int sd_pid_get_unit(pid_t pid, char **unit);

/*
 TODO:
   int sd_pid_get_user_unit(pid_t pid, char **unit);
   int sd_pid_get_machine_name(pid_t pid, char **name);
   int sd_pid_get_slice(pid_t pid, char **slice);

   int sd_peer_get_session(int fd, char **session);
   int sd_peer_get_unit(int fd, char **unit);
   int sd_peer_get_user_unit(int fd, char **unit);
   int sd_peer_get_owner_uid(int fd, uid_t *uid);
   int sd_peer_get_slice(int fd, char **slice);
 */

#endif
