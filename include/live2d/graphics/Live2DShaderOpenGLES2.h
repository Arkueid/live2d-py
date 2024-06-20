
#ifndef __LIVE2D_LIVE2D_SHADER_OPENGLES2_H__
#define __LIVE2D_LIVE2D_SHADER_OPENGLES2_H__

#include "../memory/LDObject.h"
#include "../type/LDVector.h"

#ifdef L2D_TARGET_ANDROID_ES2
#include <jni.h>
#include <errno.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef L2D_TARGET_IPHONE_ES2
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#ifdef L2D_TARGET_WIN_GL
#include <windows.h>
#include <GL/gl.h>
#endif
//#ifdef  L2D_GRAPHICS_OPENGL_ES2
//------------ LIVE2D NAMESPACE ------------
namespace live2d
{
	class DrawParam_OpenGLES2;
	class ClipContext;

	class ShaderSet : public LDObject
	{
	public:
		GLuint shaderProgram;

		GLuint a_position_Loc;
		GLuint a_texCoord_Loc;
		GLint u_matrix_Loc;
		GLint u_clipMatrix_Loc;
		GLint s_texture0_Loc;
		GLint s_texture1_Loc;
		GLint u_baseColor_Loc;
		GLint u_channelFlag_Loc;
	};

	class Live2DShaderOpenGLES2 : public LDObject
	{
	public:

		static Live2DShaderOpenGLES2* getInstance();
		static void disposeInstance();

		void setup(DrawParam_OpenGLES2 *dp, GLuint texture, int textureNo , int vertexCount_, l2d_pointf * vertexArray, l2d_uvmapf * uvArray
			, float opacity, int colorCompositionType, float *color, bool premultipliedAlpha, float *matrix4x4);

		static void setExtShaderMode( bool _extMode , bool _extPAMode );

#ifdef L2D_TARGET_ANDROID_ES2
	public:
		void reloadShader();


	private:
		static bool			extMode ;					// 拡張方式で描画
		static bool			extPAMode ;					// 拡張方式で描画
#endif

	private:
		Live2DShaderOpenGLES2(void);
		~Live2DShaderOpenGLES2(void);

		void disposeShader() ;
		bool compileShader(GLuint *shader , GLenum type , const char *shaderSource ) ;
		bool linkProgram(GLuint prog) ;
		bool validateProgram(GLuint prog) ;
		GLuint loadShaders(const char* vertShaderSrc, const char * fragShaderSrc ) ;

		void initShader();

	private:
		LDVector<ShaderSet*> *shaderSets;


	};
}

#endif	// __LIVE2D_LIVE2D_SHADER_OPENGLES2_H__

