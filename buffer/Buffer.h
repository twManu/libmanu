#ifndef	__BUFFER_H__
#define	__BUFFER_H__

class BufferList;

/*!
   @class Buffer
   To hold data
   Usage a: lib allocate buffer
	construct()
	Init(NULL, size)
	destructor()         //buffer delete
   Usage b: user allocate buffer
	construct(false)
	Init(addr, size)
	destructor()
 */
class Buffer {
friend BufferList;        //to use protected function
protected:
	//! @brief whether to alloc/dealloc buffer in Init
	bool                 m_selfAlloc;
	//! @brief data holder
	unsigned char        *m_data;
	//! @brief buffer size
	unsigned int         m_size;
	//! @brief data size<= buffer size
	unsigned int         m_used;
	/*! @detail
	 * to set buffer address and size.
	 * @param - data, it is ignored in case of selfAlloc.
	 * @param - bufSize, the value is set to member variable directly
	 */
	virtual bool         Init(unsigned char *data, unsigned int bufSize);
	void                 SetUsedSize(unsigned int usedSize);

public:
	/* allocate buffer by default */
	Buffer(bool selfAlloc=true);
	virtual ~Buffer();
	//! @brief where the buffer holds the data
	unsigned char        *GetData();
	//! @brief consumer query used size
	unsigned int         GetUsedSize();
	//! @brief return buffer size
	unsigned int         GetSize();
};

#endif	//__BUFFER_H__
