
#include <string.h>
#include "ringbuf.h"


#define MIN(x, y) ((x) > (y)?(y):(x))
#define MAX(x, y) ((x) > (y)?(x):(y))


static unsigned int ringbuf_unused(ringbuf *rb) {
	return (rb->mask + 1) - (rb->in - rb->out);
}


static char is_power_of_2(unsigned int n) {
	return (n != 0 && ((n & (n - 1)) == 0));
}

/*
* 函数名称: ringbuf_init
* 参数说明: rb: 要初始化的ringbuf变量; buffer: ringbuf存储内存地址; size: ringbuf大小
* 功能描述: 对指定大小 ringbuf 结构成员进行初始化
* 返回值:   0: 初始化成功 1：失败
*/
int ringbuf_init(ringbuf *rb, void *buffer, unsigned int size) {
	if (!is_power_of_2(size)) {
		return -1;
	}

	rb->in   = 0;
	rb->out  = 0;
	rb->data = buffer;

	if (size < 2) {
		rb->mask = 0;
		return -1;
	}
	
	rb->mask       = size - 1;
	rb->free_bytes = size;
	rb->size       = size;
	
	return 0;
}

int ringbuf_data_available(ringbuf *rb) {
	return (rb->size - rb->free_bytes);
}

/*
* 函数名称：ringbuf_copy_in
* 参数说明：rb:目标FIFO; src:要写入的数据; len:要写入的数据长度；offset:写入起始地址
* 功能描述：把src中的数据写入到 ringbuf 中offset位置处
* 返回值: 无
*/
static void ringbuf_copy_in(ringbuf *rb, const void *src, unsigned int len, unsigned int offset)
{
	unsigned int l;
	
	unsigned int size = rb->mask + 1;
	
	offset &= rb->mask;
	
	l = MIN(len, size - offset);

	memcpy((unsigned  char *)rb->data + offset, src, l);
	memcpy(rb->data, (unsigned  char *)src + l, len - l);
}

/*
* 函数名称：ringbuf_put
* 参数说明：rb:目标ringbuf; buf:要写入的数据; len:要写入的数据长度
* 功能描述：把 buf中len长度数据写入ringbuf
* 返回值:   成功写入的数据长度
*/
unsigned int ringbuf_in(ringbuf *rb, const void *buf, unsigned int len)
{
	unsigned int l;
	
	l = ringbuf_unused(rb);
	if (len > l) { len = l;}
		
	ringbuf_copy_in(rb, buf, len, rb->in);
	rb->in += len;

	rb->free_bytes -= len;
	return len;
}

/*
* 函数名称：ringbuf_copy_out
* 参数说明: rb:目标ringbuf; dst:读出数据的存储地址; len:读出数据长度
* 功能描述：从ringbuf中offset位置读出len长度的数据到buf中
* 返回值：  读出数据的长度
*/
static void ringbuf_copy_out(ringbuf *rb, void *dst, unsigned int len, unsigned int offset) {
	unsigned int l;
	unsigned int size = rb->mask + 1;
	
	offset &= rb->mask;
	
	l = MIN(len, size - offset);

	memcpy(dst, (unsigned  char *)rb->data + offset, l);
	memcpy((unsigned  char *)dst + l, rb->data, len - l);	
}

unsigned int ringbuf_out_peek(ringbuf *rb, void *buf, unsigned int len) {

	unsigned int l;

	l = rb->in - rb->out;
	if (len > l) { len = l; }		

	ringbuf_copy_out(rb, buf, len, rb->out);

	return len;
}

unsigned int ringbuf_out(ringbuf *rb, void *buf, unsigned int len)
{
	len = ringbuf_out_peek(rb, buf, len);
	rb->out += len;

	rb->free_bytes += len;
		
	return len;
}

unsigned int ringbuf_out_all(ringbuf *rb, void *buf)
{
	unsigned int len;

	len = rb->size - rb->free_bytes;

	if (len == 0) { return 0; }

	len = ringbuf_out_peek(rb, buf, len);
	rb->out += len;

	rb->free_bytes += len;
		
	return len;
	
}

