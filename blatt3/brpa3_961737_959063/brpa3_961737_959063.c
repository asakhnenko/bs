#include <linux/init.h>		/* __init and __exit macroses */
#include <linux/kernel.h>	/* KERN_INFO macros */
#include <linux/module.h>	/* required for all kernel modules */
#include <linux/moduleparam.h>	/* module_param() and MODULE_PARM_DESC() */

#include <linux/fs.h>		/* struct file_operations, struct file */
#include <linux/miscdevice.h>	/* struct miscdevice and misc_[de]register() */
#include <linux/mutex.h>	/* mutexes */
#include <linux/string.h>	/* memchr() function */
#include <linux/slab.h>		/* kzalloc() function */
#include <linux/sched.h>	/* wait queues */
#include <linux/uaccess.h>	/* copy_{to,from}_user() */

#include "brpa3.h"
#include "mod_exp.h"

/**
* PLEASE NOTE!
* Some parts of this character device are inspired by https://www.linuxvoice.com/be-a-kernel-hacker/
*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("961737 959063");
MODULE_DESCRIPTION("In-kernel decryption module");

static unsigned long buffer_size = 8192;
module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");

static unsigned short order = 59;
module_param(order, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(order, "order");

static unsigned short generator = 2;
module_param(generator, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(generator, "generator");

static unsigned short secret = 5;
module_param(secret, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(secret, "secret");

static unsigned short openkey = 32;
module_param(openkey, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(openkey, "openkey");

static unsigned short openkey_sender = 16;
module_param(openkey_sender, ushort, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(openkey_sender, "openkey_sender");


struct buffer {
	// read wait queue head
	wait_queue_head_t read_queue;
	struct mutex lock;
	// data points to the string, end to the charater after
	char *data, *end;
	// where to start reading
	char *read_ptr;
	unsigned long size;
};

struct buffer *buf;
unsigned int almost_done;
unsigned int mistake_in_buffer;

static struct buffer *buffer_alloc(unsigned long size)
{
	//struct buffer *
	buf = NULL;

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (unlikely(!buf))
		goto out;

	buf->data = kzalloc(size, GFP_KERNEL);
	if (unlikely(!buf->data))
		goto out_free;

	init_waitqueue_head(&buf->read_queue);

	mutex_init(&buf->lock);

	/* It's unused for now, but may appear useful later */
	buf->size = size;

 out:
	return buf;

 out_free:
	kfree(buf);
	return NULL;
}

static void buffer_free(struct buffer *buffer)
{
	kfree(buffer->data);
	kfree(buffer);
}

static void updateOpenkey(void)
{
	openkey = mod_exp(generator, secret, order);
}

static long input2number(char *string, size_t size)
{
	long result = 0;
	int i;
	unsigned int decimal = 1;
	// TODO
	if(size > 9)
	{
		printk("Wrong input! Too big");
		return -1;
	}

	for(i = size - 1; i >= 0; i--)
	{
		if(string[i] < 48 || string[i] > 57)
		{
			printk("Wrong input! Only numbers are allowed");
			return -1;
		}
		result = result + (int)(string[i] - 48) * decimal;
		decimal = decimal * 10;
	}
	return result;
}

static unsigned long decrypt(unsigned int decrypted)
{
	unsigned short power = secret * (order - 2);
	unsigned long b = mod_exp(openkey_sender, power, order);
	return mod_exp((b * decrypted), 1, order);
}

static unsigned int output_size(unsigned long number)
{
	unsigned int size;

	size = 0;
	while(number > 0)
	{
		size = size + 1;
		number =  number / 10;
	}
	return size;
}

static char* number2output(unsigned long number)
{
	unsigned int size;
	char* output;
	int i;

	if(number < 0)
	{
		printk("Wrong number!");
		return 0;
	}

	size = output_size(number);

	output = (char*)kmalloc(size+1, GFP_KERNEL);
	if(output == NULL)
	{
		printk("kmalloc panic");
		return 0;
	}

	for(i = size-1; i >= 0; i--)
	{
		output[i] = (number % 10) + 48;
		number = number / 10;
	}

	output[size] = '\n';
	return output;
}

static int brpa3_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Opening device \n");
	//struct buffer *buf
	buf->read_ptr = buf->data;
	file->private_data = buf;
	almost_done = 0;
	return 0;
}

// copies the data from kernel buffer into userspace
static ssize_t brpa3_read(struct file *file, char __user * out,
			    size_t size, loff_t * off)
{
	// copy to buffer
	//struct buffer *
	ssize_t result;
	buf = file->private_data;

	if(mistake_in_buffer == 1)
	{
		printk(KERN_INFO "Previous write had incorrect input \n");
		return -1;
	}

	if(almost_done == 1)
	{
		almost_done = 0;
		return 0;
	}

	// there can be many processes waiting for the data (that's why while())
	// for end of file or file to become available
	while (buf->read_ptr == buf->end) {
		mutex_unlock(&buf->lock);
		if (file->f_flags & O_NONBLOCK) {
			result = -EAGAIN;
			goto out;
		}
		// make sure it is interruptible
		if (wait_event_interruptible
		    (buf->read_queue, buf->read_ptr != buf->end)) {
			result = -ERESTARTSYS;
			goto out;
		}
		if (mutex_lock_interruptible(&buf->lock)) {
			result = -ERESTARTSYS;
			goto out;
		}
	}

	// to make sure that userspace is not empty yet
	size = min(size, (size_t) (buf->end - buf->read_ptr));
	// copy from buffer to user space
	// it fails if the pointer is wrong (don't trust anything from the kernel)
	printk(KERN_INFO "Reading %s\n",buf->read_ptr);
	if (copy_to_user(out, buf->read_ptr, size)) {
		result = -EFAULT;
		goto out;
	}

	// allow reading data in arbitrary chunks
	// buf->read_ptr += size;
	result = size;
	almost_done = 1;

 out:
 // return either number of bytes read or an error code
	return result;
}

static ssize_t brpa3_write(struct file *file, const char __user * in,
			     size_t size, loff_t * off)
{
	ssize_t result = 0;
	long decrypted;
	unsigned long encrypted;
	char* output;

	buf = file->private_data;

	printk(KERN_INFO "Writing \n");

	if (size > buffer_size) {
		result = -EFBIG;
		goto out;
	}

	printk(KERN_INFO "Debug Write 1 \n");
	if (mutex_lock_interruptible(&buf->lock)) {
		result = -ERESTARTSYS;
		goto out;
	}

	printk(KERN_INFO "Debug Write 2 \n");
	if (copy_from_user(buf->data, in, size)) {
		result = -EFAULT;
		goto out_unlock;
	}

	buf->end = buf->data + size;
	printk(KERN_INFO "Debug Write 3 \n");
	if (buf->end > buf->data)
	{
		// Do the calculations
		decrypted = input2number(buf->data, size);
		printk(KERN_INFO "Received %li \n", decrypted);
		if(decrypted < 0)
		{
			result = -1;
			mistake_in_buffer = 1;
			buf->read_ptr = buf->end;
			goto out_unlock;
		}
		mistake_in_buffer = 0;
		encrypted = decrypt(decrypted);
		output = number2output(encrypted);
		printk(KERN_INFO "Result %s \n", output);

		// // Remove previously allocated space
		// kfree(buf->data);

		// Reassign
		buf->data = output;
		buf->end = buf->data + (size_t)output_size(encrypted)+1;
	}

	buf->read_ptr = buf->data;
	// wake up processes waiting in the queue
	printk(KERN_INFO "Debug Write 4 \n");
	wake_up_interruptible(&buf->read_queue);

	result = size;
 out_unlock:
	mutex_unlock(&buf->lock);
 out:
 	printk(KERN_INFO "Write out \n");
	return result;
}

static int brpa3_close(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Closing \n");
	return 0;
}

static long brpa3_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	unsigned short tmp;
	switch (cmd)
	{
		case BRPA3_SET_SECRET:
			if(copy_from_user(&tmp, (unsigned short*)arg, sizeof(unsigned short)))
			{
				printk(KERN_INFO "Couldn't copy secret");
				return -EACCES;
			}
			if(tmp < 0 || tmp > order)
			{
				printk(KERN_INFO "Incorrect secret value");
				return -EINVAL;
			}
			secret = tmp;
			printk(KERN_INFO "New secret: %d", secret);
			updateOpenkey();
			printk(KERN_INFO "New openkey: %d", openkey);
			break;
		case BRPA3_SET_OPENKEY:
			if(copy_from_user(&tmp, (unsigned short*)arg, sizeof(unsigned short)))
			{
				printk(KERN_INFO "Couldn't copy openkey");
				return -EACCES;
			}
			openkey_sender = tmp;
			printk(KERN_INFO "New openkey_sender: %d", openkey_sender);
			break;
		case BRPA3_GET_OPENKEY:
			if(copy_to_user((unsigned short*)arg, &openkey, sizeof(unsigned short)))
			{
				printk(KERN_INFO "Couldn't return openkey");
				return -EACCES;
			}
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static struct file_operations brpa3_fops = {
	.owner = THIS_MODULE,
	.open = brpa3_open,
	.read = brpa3_read,
	.write = brpa3_write,
	.release = brpa3_close,
	// .ioctl was renamed into unlocked_ioclt
	.unlocked_ioctl = brpa3_ioctl,
	.llseek = noop_llseek
};

static struct miscdevice brpa3_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "brpa3_961737_959063",
	.fops = &brpa3_fops
};

static int __init brpa3_init(void)
{
	int err;
	if (!buffer_size)
		return -1;

	misc_register(&brpa3_misc_device);
	printk(KERN_INFO
	       "brpa3_961737_959063 device has been registered, buffer size is %lu bytes\n",
	       buffer_size);

	err = 0;
 	buf = buffer_alloc(buffer_size);
 	if (unlikely(!buf)) {
 		err = -ENOMEM;
 		goto out;
 	}
	mistake_in_buffer = 0;
out:
	return err;
}

static void __exit brpa3_exit(void)
{
	buffer_free(buf);
	misc_deregister(&brpa3_misc_device);
	printk(KERN_INFO "brpa3_961737_959063 device has been unregistered\n");
}

module_init(brpa3_init);
module_exit(brpa3_exit);
