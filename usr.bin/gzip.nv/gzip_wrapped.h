#include <sys/nv.h>

#define BUFLEN		(64 * 1024)

void gz_sandbox(int sock);

off_t gz_compress_wrapper(int in, int out, off_t *gsizep, const char *origname, uint32_t mtime, int sock);
off_t gz_uncompress_wrapper(int in, int out, char *pre, size_t prelen, off_t *gsizep,
	      const char *filename, int sock);

void gz_compress_unwrapper(struct nvlist* nvl, int sock);
void gz_uncompress_unwrapper(struct nvlist* nvl, int sock);

off_t gz_compress(int in, int out, off_t *gsizep, const char *origname, uint32_t mtime);
off_t gz_uncompress(int in, int out, char *pre, size_t prelen, off_t *gsizep,
	      const char *filename);

void limit_fds(int in, int out);
