#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/string.h>
#include<linux/uaccess.h>

#include "list_word.h"

#define LOG(str) (printk(KERN_ALERT "SIMPLE DRIVER: %s\n", str))

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Celeste");

dev_t dev = MKDEV(55, 12);
static struct class *dev_class; // class are file that apear in /sys/class
static struct cdev cdev;

static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *filp, char *buf, size_t len,loff_t * off);
static ssize_t my_write(struct file *filp, const char *buf, size_t len, loff_t * off);


static struct file_operations file_ops =
{
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
    .open = my_open,
    .release = my_release,
};

static int my_open(struct inode *inode, struct file *file)
{
    return (0);
}

static int my_release(struct inode *inode, struct file *file)
{
    return (0);
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    size_t random, index, word_size;
    char *buffer = kmalloc(len + 1, GFP_KERNEL);
    size_t ret;

    if (buffer == NULL)
        return (-1);
    for (index = 0; index < len - 1; index++) {
        get_random_bytes(&random, 1);
        random %= NB_WORD;
        word_size = strlen(word_list[random]);
        if (word_size + index > len) {
            index--;
            continue;
        }
        strcpy(buffer + index, word_list[random]);
        index += word_size;
        buffer[index] = ' ';
    }
    buffer[index - 1] = '\n';
    buffer[index] = '\0';
    *off += index;
    filp->f_pos = *off;
    ret = index + 1 - copy_to_user(buf, buffer, index + 1);
    kfree(buffer);
    return (ret);
}

static ssize_t my_write(struct file *filp, const char *buf, size_t len, loff_t * off)
{
    return (0);
}


static int __init hello_init(void)
{
    LOG("initialisation");
    if (register_chrdev_region(dev, 1, "simple_drv_dev") < 0) { // allocation of the major and minor
	    LOG("error can't create the dev\n");
        return (-1);
    }
    dev_class = class_create(THIS_MODULE, "simple_drv_class");
    if (dev_class == NULL) {
	    LOG("error can't create the class\n");
        goto r_device;
    }
    if (device_create(dev_class, NULL, dev, NULL, "simple_drv_device") == NULL) {
	    LOG("error can't create the device\n");
        goto r_class;
    }
    cdev_init(&cdev, &file_ops);
    if (cdev_add(&cdev, dev, 1) < 0) {
	    LOG("can't add file ops\n");
        goto r_class;
    }

    return 0;

r_class:
    class_destroy(dev_class);
r_device:
    unregister_chrdev_region(dev, 1); // unalloate the major and minor
    return (-1);
}

static void __exit hello_exit(void)
{
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    unregister_chrdev_region(dev, 1); // unalloate the major and minor
	LOG("the hello_world module exit succesfully\n");
}

module_init(hello_init);
module_exit(hello_exit);
