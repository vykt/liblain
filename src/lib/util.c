//standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//system headers
#include <dirent.h>
#include <errno.h>

#include <sys/types.h>

#include <linux/limits.h>

//local headers
#include "memcry.h"
#include "util.h"
#include "debug.h"


#define _LINE_LEN PATH_MAX + 128



/*
 *  --- [INTERNAL] ---
 */

//convert the first line of /proc/pid/status to a name (comm)
DBG_STATIC DBG_INLINE
void _line_to_name(const char * line_buf, char * name_buf) {

    line_buf += 5;
    char * name_str;

    //for every character in the line
    for (int i = 0; i < _LINE_LEN; ++i) {

        if (line_buf[i] == ' ' || line_buf[i] == '\t') continue;
        name_str = (char *) line_buf + 1;
        name_str[strcspn(name_str, "\n")] = '\0';
        break;
    }

    strncpy(name_buf, name_str, NAME_MAX);
    
    return;
}


/*
 *  Use `/proc/pid/status` to get the name of a process. This is
 *  how utilities like `ps` and `top` fetch process names.
 */
 
DBG_STATIC
int _get_status_name(char * name_buf, const pid_t pid) {

    int ret;
    char * fret;

    FILE * fs;

	char path_buf[PATH_MAX];
    char line_buf[_LINE_LEN];
    

    //build path
    snprintf(path_buf, PATH_MAX, "/proc/%d/status", pid);

    //get name
    fs = fopen(path_buf, "r");
    if (fs == NULL) {
        mc_errno = MC_ERR_PROC_STATUS;
        return -1;
    }

    //read top line containing name (comm) of process
    fret = fgets(line_buf, _LINE_LEN, fs);
    if (fret == NULL) {
        fclose(fs);
        mc_errno = MC_ERR_PROC_STATUS;
        return -1;
    }

    //close file stream
    ret = fclose(fs);
    if (ret == -1) {
        mc_errno = MC_ERR_PROC_STATUS;
        return -1;
    }

    //extract name from line
    _line_to_name(line_buf, name_buf);

    //replace trailing newline in the name
    name_buf[strcspn(name_buf, "\n")] = '\0'; //replace trailing newline

    return 0;
}



/*
 *  --- EXTERNAL
 */

char * mc_pathname_to_basename(const char * pathname) {

    char * basename = strrchr(pathname, (int) '/');
    
    if (basename == NULL) return (char *) pathname;
    return basename + 1;
}


pid_t mc_pid_by_name(const char * comm, cm_vct * pid_vector) {

	int ret;
	int first_recorded = 0;

    char name_buf[NAME_MAX];

	pid_t temp_pid;
    pid_t first_pid = -1;

	DIR * ds;
	struct dirent * d_ent;



    //initialise vector
    ret = cm_new_vct(pid_vector, sizeof(pid_t));
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    //open proc directory
	ds = opendir("/proc");
	if (ds == NULL) {
        mc_errno = MC_ERR_PROC_NAV;
        return -1;
    }

	//while entries left
	while ((d_ent = readdir(ds)) != NULL) {

		//if the directory belongs to a process
		if (d_ent->d_type != DT_DIR || !(isdigit(d_ent->d_name[0]) > 0)) continue;

        //convert dir name to pid_t
        temp_pid = (pid_t) strtoul(d_ent->d_name, NULL, 10);
        if (errno == ERANGE) {
            closedir(ds);
            cm_del_vct(pid_vector);
            mc_errno = MC_ERR_PROC_NAV;
            return -1;
        }

        //get the name of this pid
        ret = _get_status_name(name_buf, temp_pid);
        if (ret) {
            closedir(ds);
            cm_del_vct(pid_vector);
            return -1;
        }

        //if found a match
        ret = strcmp(name_buf, comm);
        if (!ret) {
            
            //add pid_t to list of potential PIDs
            ret = cm_vct_apd(pid_vector, (cm_byte *) &temp_pid);
            if (ret) {
                closedir(ds);
                cm_del_vct(pid_vector);
                mc_errno = MC_ERR_LIBCMORE;
                return -1;
            }

            //save first pid
            if (!first_recorded) {
                first_pid = temp_pid;
                ++first_recorded;
            }

        }//end if found process with matching name

	}//end while entries left
	
	ret = closedir(ds);
	if (ret) {
        cm_del_vct(pid_vector);
        mc_errno = MC_ERR_PROC_NAV;
        return -1;
    }

	return first_pid;
}


int mc_name_by_pid(const pid_t pid, char * name_buf) {

    int ret;

    //get name for this pid
    ret = _get_status_name(name_buf, pid);
    if (ret) return -1;

    return 0;
}


void mc_bytes_to_hex(const cm_byte * inp, const int inp_len, char * out) {

    cm_byte nibble;
    int count;

    //for each byte of input
	for (int byte_index = 0; byte_index < inp_len; ++byte_index) {

        count = 0;

        //for each nibble of byte
        for (int nibble_count = 1; nibble_count >= 0; --nibble_count) {

            //assign nibble
            nibble = inp[byte_index];
            if (nibble_count == 1) nibble = nibble >> 4;
            nibble = nibble & 0x0F;

            //convert nibble
            if (nibble < 0xA) {
                out[(byte_index * 2) + count] = 0x30 + nibble;
            } else {
                out[(byte_index * 2) + count] = 0x61 + nibble - 0xa;
            }

            ++count;

        } //end for each nibble

	} //end for each byte

    //add null terminator
    out[inp_len * 2] = '\0';

    return;
}
