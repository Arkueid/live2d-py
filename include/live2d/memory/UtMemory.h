/**
 *  UtMemory.h
 *
 *  Copyright(c) Live2D Inc. All rights reserved.
 *  [[ CONFIDENTIAL ]]
 */

#ifndef __LIVE2D_UTMEMORY_H__
#define __LIVE2D_UTMEMORY_H__


#ifndef __SKIP_DOC__

#include "../Live2D.h"
#include "LDAllocator.h"

// メモリをアライメントする
#define		L2D_ALLIGN(p , align)				((char *)(((unsigned int)(p) + (align)-1) & ~((align)-1)))
#define		L2D_PTR_TO_ALLOC_HEADER( ptr )		((AllocHeader*)((char*)ptr - sizeof(AllocHeader)))

#define		L2D_MEM_STATIC						live2d::UtMemory::getStaticMemoryParam()
#define		L2D_MEM_TMP							live2d::UtMemory::getTmpMemoryParam()

#define		L2D_UTMEMORY_MALLOC_DUMP

#if defined(L2D_UTMEMORY_MALLOC_DUMP) && defined( L2D_DEBUG )
	//---- DEBUG  ---- //cf. GEMS 2 p.67

	#define		L2D_NEW(memParam)			new ( memParam ,__FILE__,__LINE__)
	#define		L2D_NEW_STATIC					new ( L2D_MEM_STATIC ,__FILE__,__LINE__)

	#define		L2D_DELETE						live2d::UtMemory::setDebugInfo(__FILE__,__LINE__) ,false ? \
													live2d::UtMemory::setDebugInfo("",0) : delete 

	#define		L2D_NEW_TMP						new (L2D_MEM_TMP ,__FILE__,__LINE__ )
	#define		L2D_DELETE_TMP					delete (L2D_MEM_TMP ,__FILE__,__LINE__ )

	#define		L2D_PLACEMENT_NEW(PTR)			new( live2d::UtMemory::placementNew_debug( (PTR)  , __FILE__,__LINE__ ) )

	//--- malloc/free ---
	#define		L2D_MALLOC(memParam,size)	live2d::UtMemory::malloc_debug( memParam , (size) , __FILE__,__LINE__ )
	#define		L2D_MALLOC_TMP(size)			live2d::UtMemory::malloc_debug( L2D_MEM_TMP , (size) , __FILE__,__LINE__ )
	#define		L2D_FREE(ptr)					live2d::UtMemory::free_debug( (ptr) , __FILE__,__LINE__ )


	//--- ummanaged ---
	#define		L2D_NEW_UNMANAGED				new (__FILE__,__LINE__ )
	#define		L2D_DELETE_UNMANAGED			live2d::UtMemory::setDebugInfo(__FILE__,__LINE__) ,false ? \
													live2d::UtMemory::setDebugInfo("",0) : delete   

#else

	#define		L2D_NEW(memParam)			new ( memParam )
	#define		L2D_NEW_STATIC					new ( L2D_MEM_STATIC )

	#define		L2D_DELETE						delete 

	#define		L2D_NEW_TMP						new ( L2D_MEM_TMP )
	#define		L2D_DELETE_TMP					delete (L2D_MEM_TMP )

	#define		L2D_PLACEMENT_NEW(PTR)			new( PTR )

	//--- malloc/free ---
	#define		L2D_MALLOC(memParam,size)	live2d::UtMemory::malloc_exe( memParam , (size) )
	#define		L2D_MALLOC_TMP(size)			live2d::UtMemory::malloc_exe( L2D_MEM_TMP , (size))
	#define		L2D_FREE(ptr)					live2d::UtMemory::free_exe( (ptr) )

	//--- ummanaged ---
	#define		L2D_NEW_UNMANAGED				new 
	#define		L2D_DELETE_UNMANAGED			delete

#endif //defined(L2D_UTMEMORY_MALLOC_DUMP) && defined( L2D_DEBUG )

# define L2D_MALLOC_OBJ_ARRAY(memParam,size)	L2D_MALLOC(memParam,size)		
# define L2D_MALLOC_OBJ_ARRAY_TMP(size)			L2D_MALLOC( L2D_MEM_TMP ,size)	



//--------- LIVE2D NAMESPACE ------------
namespace live2d 
{

	
	class LDAllocator;	// LDAllocatorクラスの宣言
	class MemoryParam ;
	class MemoryInfoSet ;
	class AMemoryHolder ;
	class MemoryHolderTmp ;
	class MemoryHolderSocket ;
	
	
	
	class UtMemory 
	{
	public:
	
	//#if 1
	
		
		static void * allocator_malloc( l2d_size_t size , LDAllocator::Type allocType ) ;
	
	
		
		static void allocator_free( void* ptr , LDAllocator::Type allocType ) ;
	
		
		static void* malloc_exe( MemoryParam* memParam , l2d_size_t size  ) ;
		static void* malloc_debug( MemoryParam* memParam ,l2d_size_t size  , const char* filename , int lineno ) ;
	
		
		static void free_exe( void* ptr ) ;
		static void free_debug( void* ptr , const char* filename , int lineno ) ;
	
		static void* placementNew_debug( void* ptr, const char* filename , int lineno  ) ;
	
	
		static void setDebugInfo( const char* filename , int lineno )
		{
			UtMemory::lastFileName	= filename ;
			UtMemory::lastLine		= lineno ;
		}
	
		static const char* getDebugInfo( int& retLineno )
		{
			const char* ret = UtMemory::lastFileName ;
			retLineno = UtMemory::lastLine ;
	
			UtMemory::lastFileName = NULL ;
			UtMemory::lastLine = 0 ;
	
			return ret ;
		}
	
	#ifdef L2D_DEBUG
		static void dumpMemoryInfo() ;
	
		static MemoryInfoSet* getMemoryInfoLocal(){ return memoryInfoLocal ; }
		static MemoryInfoSet* getMemoryInfoAllocator(){ return memoryInfoAllocator ; }
	#endif
		
		// ポインタをalignでアライメントする。signatureSizeバイト分をポインタの前に挟む。戻り値はsignatureの前に
		static char* alignPtr( char* ptr , int align , int signatureSize )
		{
			return (char *)(((l2d_size_t)ptr + signatureSize + align-1) & ~(align-1)) - signatureSize ;
		}
	
		// Live2D::dispose() まで生存し続けるメモリグループを取得する
		static MemoryParam*  getStaticMemoryParam(){ return staticMemoryParam ; }
	
		// テンポラリに使用するメモリグループを取得する
		static MemoryParam*  getTmpMemoryParam(){ return tmpMemoryParam ; }
	
		static void staticInit_notForClientCall(LDAllocator*  allocator) ;//1.0beta3
		static void staticRelease_notForClientCall() ;//1.0beta3
	
	
	private:
		
		UtMemory();// インスタンス化させない
		
		//Prevention of copy Constructor
		UtMemory( const UtMemory & ) ;
		UtMemory& operator=( const UtMemory & ) ;
		~UtMemory();
	
		static LDAllocator*		allocator ;
		static const char*		lastFileName ;
		static int				lastLine ;
	
		static MemoryParam*			staticMemoryParam ;// Live2D::init()からLive2D::dispose()まで生存
		static MemoryParam*			tmpMemoryParam ;// 自由に確保・開放出来るメモリ（通常のメモリと同じように使える）
	
		static AMemoryHolder*		staticMemoryHolder ;
		static MemoryHolderSocket*	tmpMemoryHolder	;
		static MemoryHolderTmp*		tmpMemoryHolderImpl	;
	
	
	#ifdef L2D_DEBUG
		static bool					useMemoryInfo ;			// 使用中メモリのログを取る場合にtrue 
	
		static MemoryInfoSet*		memoryInfoLocal	;
		static MemoryInfoSet*		memoryInfoAllocator ;
	
	#endif
	
	};

} 
//------------------------- LIVE2D NAMESPACE ------------


#endif // __SKIP_DOC__

#endif // __LIVE2D_UTMEMORY_H__
