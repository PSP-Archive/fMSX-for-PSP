
#ifndef _MAIN_H_
#define _MAIN_H_


int      VSyncTimer();
extern	unsigned int	syorioti_c;
extern	ctrl_data_t ctl;
extern	unsigned int	syorioti;


extern	int		zip_read_size;
extern	char	*zip_read_buff;
int funcUnzipCallback(int nCallbackId,
                      unsigned long ulExtractSize,
		      unsigned long ulCurrentPosition,
                      const void *pData,
                      unsigned long ulDataSize,
                      unsigned long ulUserData);


#endif /* _MAIN_H_ */
