#ifndef RINGBUF_H
#define RINGBUF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ringbuf {

	unsigned int     in;
	unsigned int     out;
	unsigned int     mask;
	void             *data;
	unsigned int     free_bytes;
	unsigned int     size;
	
} ringbuf;


int ringbuf_init(ringbuf *rb, void *buf, unsigned int size);

int ringbuf_data_available(ringbuf *rb);

unsigned int ringbuf_in(ringbuf *rb, const void *buf, unsigned int len);

unsigned int ringbuf_out(ringbuf *rb, void *buf, unsigned int len);

unsigned int ringbuf_out_peek(ringbuf *rb, void *buf, unsigned int len);

unsigned int ringbuf_out_all(struct ringbuf *rb, void *buf);

#ifdef __cplusplus
}
#endif

#endif

