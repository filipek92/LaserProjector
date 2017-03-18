#include <errno.h>
#include <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include "main.h"

int _write(int file, char *data, int len)
{
   if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
   {
      errno = EBADF;
      return -1;
   }
   HAL_StatusTypeDef status =
   HAL_USART_Transmit(&pc_usart, (uint8_t*)data, len, 1000);

   return (status == HAL_OK ? len : 0);
}

int _read(int file, char *data, int len)
{
    if (file != STDIN_FILENO)
    {
        errno = EBADF;
        return -1;
    }

    HAL_StatusTypeDef status = HAL_USART_Receive(&pc_usart, (uint8_t *) data, len, 1000);

    return status==HAL_OK?len:0;
}

int _close(int file){return -1;}
int _lseek(int file, int ptr, int dir){return 0;}
int _fstat(int file, struct stat *st){return 0;}

int _isatty(int file)
{
    if ((file == STDOUT_FILENO) ||
        (file == STDIN_FILENO) ||
        (file == STDERR_FILENO))
    {
        return 1;
    }

    errno = EBADF;
    return 0;
}
