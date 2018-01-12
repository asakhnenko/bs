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

#include "mod_exp.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aly");
MODULE_DESCRIPTION("In-kernel encryption module");

static unsigned long buffer_size = 8192;
module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");

unsigned short order = 59;
unsigned short generator = 2;
unsigned short secret = 5;
unsigned short openkey = 32;
unsigned short openkey_sender = 16;

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

static struct buffer *buffer_alloc(unsigned long size)
{
	struct buffer *buf = NULL;

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

// static inline char *encrypt_word(char *start, char *end)
// {
// 	char *orig_start = start, tmp;
//
// 	for (; start < end; start++, end--) {
// 		tmp = *start;
// 		*start = *end;
// 		*end = tmp;
// 	}
//
// 	return orig_start;
// }
//
// static char *encrypt_phrase(char *start, char *end)
// {
// 		char *word_start = start, *word_end = NULL;
// 		while ((word_end = memchr(word_start, ' ', end - word_start)) != NULL) {
// 			encrypt_word(word_start, word_end - 1);
// 			word_start = word_end + 1;
// 		}
// 	 encrypt_word(word_start, end);
//
// 	return encrypt_word(start, end);
// }
static long input2number(char *string, size_t size)
{
	long result = 0;
	int i;
	unsigned int decimal = 1;
	// TODO
	if(size > 9)
	{
		printk("Wrong input!");
		return -1;
	}

	for(i = size - 1; i >= 0; i--)
	{
		if(string[i] < 48 || string[i] > 57)
		{
			printk("Wrong input!");
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

	output = (char*)kmalloc(size, GFP_KERNEL);
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
	return output;
}

static int encrypt_open(struct inode *inode, struct file *file)
{
	struct buffer *buf;
	int err = 0;
	buf = buffer_alloc(buffer_size);
	if (unlikely(!buf)) {
		err = -ENOMEM;
		goto out;
	}
	// is used to associate file descriptor with some random data
	file->private_data = buf;

 out:
	return err;
}

// copies the data from kernel buffer into userspace
static ssize_t encrypt_read(struct file *file, char __user * out,
			    size_t size, loff_t * off)
{
	// copy to buffer
	struct buffer *buf = file->private_data;
	ssize_t result;

	if (mutex_lock_interruptible(&buf->lock)) {
		result = -ERESTARTSYS;
		goto out;
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
	if (copy_to_user(out, buf->read_ptr, size)) {
		result = -EFAULT;
		goto out_unlock;
	}

	// allow reading data in arbitrary chunks
	buf->read_ptr += size;
	result = size;

 out_unlock:
	mutex_unlock(&buf->lock);
 out:
 // return either number of bytes read or an error code
	return result;
}

static ssize_t encrypt_write(struct file *file, const char __user * in,
			     size_t size, loff_t * off)
{
	struct buffer *buf = file->private_data;
	//TODO: WTF (= 0)
	ssize_t result = 0;
	long decrypted;
	unsigned long encrypted;
	char* output;

	if (size > buffer_size) {
		result = -EFBIG;
		goto out;
	}

	if (mutex_lock_interruptible(&buf->lock)) {
		result = -ERESTARTSYS;
		goto out;
	}

	if (copy_from_user(buf->data, in, size)) {
		result = -EFAULT;
		goto out_unlock;
	}

	buf->end = buf->data + size;

	if (buf->end > buf->data)
	{
		printk(KERN_INFO "Starting \n");
		//encrypt_phrase(buf->data, buf->end - 1);
		decrypted = input2number(buf->data, size);
		printk(KERN_INFO "Decrypted %li \n", decrypted);
		if(decrypted < 0)
		{
			goto out;
		}
		encrypted = decrypt(decrypted);
		// TODO free
		output = number2output(encrypted);

		buf->data = output;
		buf->end = buf->data + (size_t)output_size(encrypted);
		printk(KERN_INFO "Debug: %li", buf->end - buf->data);
	}
	buf->read_ptr = buf->data;
	// wake up processes waiting in the queue
	wake_up_interruptible(&buf->read_queue);

	result = size;
 out_unlock:
	mutex_unlock(&buf->lock);
 out:
	return result;
}

static int encrypt_close(struct inode *inode, struct file *file)
{
	struct buffer *buf = file->private_data;

	buffer_free(buf);

	return 0;
}

static struct file_operations encrypt_fops = {
	.owner = THIS_MODULE,
	.open = encrypt_open,
	.read = encrypt_read,
	.write = encrypt_write,
	.release = encrypt_close,
	.llseek = noop_llseek
};

static struct miscdevice encrypt_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "reverse",
	.fops = &encrypt_fops
};

static int __init encrypt_init(void)
{
	if (!buffer_size)
		return -1;

	misc_register(&encrypt_misc_device);
	printk(KERN_INFO
	       "encrypt device has been registered, buffer size is %lu bytes\n",
	       buffer_size);

	return 0;
}

static void __exit encrypt_exit(void)
{
	misc_deregister(&encrypt_misc_device);
	printk(KERN_INFO "encrypt device has been unregistered\n");
}

module_init(encrypt_init);
module_exit(encrypt_exit);
