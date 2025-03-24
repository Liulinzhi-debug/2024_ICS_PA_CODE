#include <fs.h>


typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR,FD_EVENT, FD_DISPINFO,FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t read_from_file(void *buf, size_t offset, size_t len) {
    
    return ramdisk_read(buf, offset, len);
}

size_t write_to_file(const void *buf, size_t offset, size_t len) {
    
    return ramdisk_write(buf, offset, len);
}


/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout",0, 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr",0, 0, 0, invalid_read, serial_write},
  [FD_EVENT]  = {"/dev/events", 0, 0, 0, events_read, invalid_write},
  [FD_DISPINFO]= {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
  [FD_FB]     ={"/dev/fb", 0, 0, 0, invalid_read, fb_write},
#include "files.h"
};

#define FILE_num (sizeof(file_table) / sizeof(file_table[0]))
#define min(x, y) ((x < y) ? x : y)
#define max(x, y) ((x > y) ? x : y)

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T ev=io_read(AM_GPU_CONFIG);
  int w=ev.width;
  int h=ev.height;
  file_table[5].size=w*h*sizeof(uint32_t);
}


int fs_open(const char *pathname, int flags, int mode)
{
   size_t index ;
  for ( index = 0; index < FILE_num; index++)
  {
    //printf("file_table[%d].name 是:%s\n当前pathname 是: %s\n",index, file_table[index].name, pathname);
    if (strcmp(file_table[index].name, pathname) == 0)
    {
      file_table[index].open_offset = 0;
      return index;
    }
  }
   if(index==FILE_num)
   {
   printf("pathname %s 不存在\n", pathname);
   return -1;
   }
   return 0;
}

size_t fs_read(int fd, void *buf, size_t len)
{
  // printf("fd 是 %d, size 是 %x, open offset 是 %x, 读取长度是 %x\n", fd, file_table[fd].size, file_table[fd].open_offset, len);
   assert(fd >= 0 && fd < FILE_num);

    if (fd > 0 && fd < 5)
    //if (file_table[fd].read != NULL)
    return file_table[fd].read(buf, 0, len);

     len = min(len, file_table[fd].size - file_table[fd].open_offset);
   
    size_t count = read_from_file(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);

     file_table[fd].open_offset += count;

    return count;
}


int fs_close(int fd)
{

    return 0;
}


size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd >= 0 && fd < FILE_num);
   //if (fd > 0 && fd < 5)
    if (file_table[fd].write != NULL)
    {
 //printf("read fd is %d, size is %d, offset is %d, to read len is %d\n", fd, file_table[fd].size, file_table[fd].open_offset, len);
    return file_table[fd].write(buf, file_table[fd].open_offset, len);
    }

  //printf("read fd is %d, size is %d, offset is %d, to read len is %d\n", fd, file_table[fd].size, file_table[fd].open_offset, len);
  len = min(len, file_table[fd].size - file_table[fd].open_offset);
  //printf("read some thing from %d with size %d\n", file_table[fd].disk_offset + file_table[fd].open_offset, len);
  // panic("FUCK I am here");
  size_t count = write_to_file(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  //printf("len is %d\n", count);
  //printf("read open_offset is %d, size is %d, res is %d\n", file_table[fd].open_offset, file_table[fd].size, file_table[fd].open_offset < file_table[fd].size);
    
   file_table[fd].open_offset += len;
  //printf("read open_offset is %d, size is %d, res is %d\n", file_table[fd].open_offset, file_table[fd].size, file_table[fd].open_offset < file_table[fd].size);
  
  return count;
}


size_t fs_lseek(int fd, size_t offset, int whence) {
  assert(fd >= 0 && fd < FILE_num);

  size_t new_offset = file_table[fd].open_offset;
  //printf("这是fs_lseek change seek of fd %d\n", fd);
  switch (whence) {
    case SEEK_SET:
      new_offset = offset;
      break;
    case SEEK_CUR:
      new_offset += offset;
      break;
    case SEEK_END:
      new_offset = file_table[fd].size + offset;
      break;
    default:
    Log("Invalid whence value: %d", whence);
      return (size_t)-1;  
  }

  if (new_offset > file_table[fd].size) {
    Log("Seek position out of bounds");
    return (size_t)-1;  
  }
  //printf("这是fs_lseek offset is %d\n", file_table[fd].open_offset);
  file_table[fd].open_offset = new_offset;
  return new_offset;
}