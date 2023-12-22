#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/proc_fs.h> 
#include <linux/uaccess.h> 
#include <linux/version.h> 
#include <linux/time.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0) 
#define HAVE_PROC_OPS 
#endif 

#define procfs_name "tsulab" 

static struct proc_dir_entry *our_proc_file; 

int minutes(void){
    struct timespec64 now;
    struct tm tm;
    ktime_get_real_ts64(&now);
    time64_to_tm(now.tv_sec, 0, &tm);

    if(tm.tm_min==0&&tm.tm_sec==0)
        tm.tm_hour = 24 - tm.tm_hour;
    else 
        tm.tm_hour = 23 - tm.tm_hour;

    if(tm.tm_sec==0)
        tm.tm_min = (60 - tm.tm_min)%60;
    else
        tm.tm_min = 59 - tm.tm_min;
    tm.tm_sec = (60 - tm.tm_sec)%60;

    return (tm.tm_hour*60+tm.tm_min);
}

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset) { 
    char s[4];
    int min=minutes();
    int div=1000;
    int flag=0;
    int i;
    int len=0;
    ssize_t ret;
    for(i=0; i<4; i++){
        if((min/div+'0')!='0'||flag){
            s[len]=min/div+'0';
            len++;
            flag=1;
        }
        min=min%div;
        div/=10;
    }
    if(len==0){
        s[0]='0';
        len++;
    }
        
    ret = len; 

    if (*offset >= len || copy_to_user(buffer, s, len)) { 
        pr_info("copy_to_user failed\n"); 
        ret = 0; 
    } else { 
        pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name); 
        *offset += len; 
    } 

    return ret; 
}

#ifdef HAVE_PROC_OPS 
static const struct proc_ops proc_file_fops = { 
    .proc_read = procfile_read, 
}; 
#else 
static const struct file_operations proc_file_fops = { 
    .read = procfile_read, 
}; 
#endif 

static int __init procfs1_init(void) { 
    our_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops); 
    if (NULL == our_proc_file) { 
        proc_remove(our_proc_file); 
        pr_alert("Error:Could not initialize /proc/%s\n", procfs_name); 
        return -ENOMEM; 
    } 
    pr_info("/proc/%s created\n", procfs_name); 
    return 0; 
} 

 

static void __exit procfs1_exit(void) { 
    proc_remove(our_proc_file); 
    pr_info("/proc/%s removed\n", procfs_name); 
} 

module_init(procfs1_init); 
module_exit(procfs1_exit); 
MODULE_LICENSE("GPL");