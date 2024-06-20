/**
 *  UtDebug.h
 *
 *  [ CONFIDENTIAL ] 
 *  Live2D Inc. All rights reserved.
 */

#ifndef __LIVE2D_UTDEBUG_H__
#define __LIVE2D_UTDEBUG_H__


#include "../Live2D.h"
#include "../memory/LDObject.h"

#ifndef NULL
#  define NULL	0
#endif



//==============================================================
//  ASSERT
//==============================================================
#ifdef L2D_DEBUG

	#define L2D_ASSERT(expr)if( expr ) { } else { live2d::UtDebug::error("assert ::%s %d \n" , __FILE__ , __LINE__ ) ; live2d::UtDebug::debugBreak(); }
//	#define L2D_ASSERT_S(expr,msg)if( expr ) { } else { live2d::UtDebug::error("assert :: %s\n		@%s : %d" , msg , __FILE__ , __LINE__ ) ; live2d::UtDebug::debugBreak();}
	#define L2D_ASSERT_S(expr,format, ... )if( expr ) { } else { live2d::UtDebug::assertF( __FILE__ , __LINE__ , format ,  __VA_ARGS__  ) ; live2d::UtDebug::debugBreak(); }

	#define L2D_DEBUG_MESSAGE( format , ... ) live2d::UtDebug::error( format , __VA_ARGS__ ) 
#else
	#define L2D_ASSERT(expr) 
	#define L2D_ASSERT_S(expr , msg  , ... ) 
//	#define L2D_ASSERT_F(expr , format , ... ) 

	#define L2D_DEBUG_MESSAGE( msg , ... ) 
#endif


#ifdef L2D_DEBUG
	#define TIME_START(x) UtDebug::start( x )
	#define TIME_DUMP(x) UtDebug::dump( x )
#else
	#define TIME_START(x) 
	#define TIME_DUMP(x) 
#endif


//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	
	template <class T> class LDVector ;
	
	

#ifndef __SKIP_DOC__

	
	class DebugTimerObj : public LDObject 
	{
	public:
		const char * key ;
		long long startTimeMs ;
	};

#endif // __SKIP_DOC__
	
	
	
	class UtDebug 
	{
	public:
		const static unsigned int MEMORY_DEBUG_DUMP_ALLOCATOR				= 0x1  ;
		const static unsigned int MEMORY_DEBUG_DUMP_TMP						= 0x2  ;
		const static unsigned int MEMORY_DEBUG_DUMP_FIXED					= 0x4  ;
		const static unsigned int MEMORY_DEBUG_DUMP_UNMANAGED				= 0x8 ;
	
		const static unsigned int MEMORY_DEBUG_MEMORY_INFO_COUNT			= 0x10  ;
		const static unsigned int MEMORY_DEBUG_MEMORY_INFO_DUMP				= 0x20  ;
		const static unsigned int MEMORY_DEBUG_MEMORY_INFO_ALL				= MEMORY_DEBUG_MEMORY_INFO_COUNT | MEMORY_DEBUG_MEMORY_INFO_DUMP  ;
	
		const static unsigned int MEMORY_DEBUG_MEMORY_INFO_KEEP_FREE		= 0x40  ;// 通常はOFF（メモリが永遠に増えていく）
	
		const static unsigned int MEMORY_DEBUG_MEMORY_DUMP_PLACEMENT_NEW	= 0x80  ;
		
	
	
	
		const static unsigned int MEMORY_DEBUG_DUMP_ALL		= MEMORY_DEBUG_DUMP_ALLOCATOR 
															| MEMORY_DEBUG_DUMP_TMP 
															| MEMORY_DEBUG_DUMP_FIXED 
															| MEMORY_DEBUG_MEMORY_INFO_DUMP 
															| MEMORY_DEBUG_DUMP_UNMANAGED ;
	
		const static unsigned int READ_OBJECT_DEBUG_DUMP					= 0x1  ;
	
	
	
	public:
		static void assertF( const char* file , int lineno , const char * format , ... ) ;
	
		static void error( const char * msg , ... ) ;

		static void dumpByte( const char * data , int length ) ;
	
		static void printVectorUShort( char*msg , LDVector<unsigned short> *v , char*unit ) ;
	
		static void print(const char * format , ... ) ;
	
		static void println(const char * format , ... ) ;
		
		static void debugBreak() ;
		
		// メモリ周りの情報をダンプする場合にMEM_DUMP_で始まるフラグを設定する
		// デバッグモード（ダンプは_DEBUGが定義されている場合のみ有効）
		// 	UtDebug::MEM_DUMP_ALLなど
		static void  setMemoryDebugFlags( unsigned int flag ){ memoryDebugFlags = flag ; }

		static void  addMemoryDebugFlags( unsigned int flag ){ memoryDebugFlags |= flag ; }

		static unsigned int  getMemoryDebugFlags( ){ return memoryDebugFlags ; }
	
		static void setReadObjectDebugFlags( unsigned int flag ){ readObjectDebugFlag = flag ; }

		static void addReadObjectDebugFlags( unsigned int flag ){ readObjectDebugFlag |= flag ; }

		static unsigned int getReadObjectDebugFlags( ){ return readObjectDebugFlag ; }
	
	
	#ifdef L2D_DEBUG
		// 
		static void start( const char * key ) ;

		// 
		static long long dump( const char * key , bool print_string=true) ;
	#endif
	
		// リリース用メソッド（Live2D::diposeから呼ばれる)
		static void staticRelease_notForClientCall() ;
	
	private:
		UtDebug() ;					
						
		//Prevention of copy Constructor
		UtDebug( const UtDebug & ) ;				
		UtDebug& operator=( const UtDebug & ) ; 	

		~UtDebug() ;
		
	private:
		static LDVector<DebugTimerObj*>* 	timersList ;
	
		static unsigned int 				memoryDebugFlags ;
		static unsigned int 				readObjectDebugFlag ;
		
	};
} 
//------------ LIVE2D NAMESPACE ------------

#endif		// __LIVE2D_UTDEBUG_H__
