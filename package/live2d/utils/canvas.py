from .opengl_functions import create_vao, create_canvas_framebuffer, create_program

import OpenGL.GL as GL
import numpy as np
from typing import Callable


class Canvas:

    def __init__(self):
        self.__canvas_opacity = 1.0

        self._canvas_framebuffer = -1

        self._canvas_texture = -1

        self.__init()

    def __create_program(self):
        vertex_shader = """#version 330 core
        layout(location = 0) in vec2 a_position;
        layout(location = 1) in vec2 a_texCoord;
        out vec2 v_texCoord;
        void main() {
            gl_Position = vec4(a_position, 0.0, 1.0);
            v_texCoord = a_texCoord;
        }
        """
        frag_shader = """#version 330 core
        in vec2 v_texCoord;
        uniform sampler2D canvas;
        uniform float opacity;
        void main() {
            vec4 color = texture(canvas, v_texCoord);
            color *= opacity;
            gl_FragColor =  color;
        }
        """
        self._program = create_program(vertex_shader, frag_shader)
        self._opacity_loc = GL.glGetUniformLocation(self._program, "opacity")

    def __create_vao(self):
        vertices = np.array([
            # 位置
            -1, 1,
            -1, -1,
            1, -1,
            -1, 1,
            1, -1,
            1, 1,
        ], dtype=np.float32)
        uvs = np.array([
            # 纹理坐标
            0, 1,
            0, 0,
            1, 0,
            0, 1,
            1, 0,
            1, 1
        ], dtype=np.float32)
        self._vao = create_vao(vertices, uvs)

    def setSize(self, width, height):
        self._width = width
        self._height = height
        
        self.__create_canvas_framebuffer()

    def __create_canvas_framebuffer(self):
        GL.glDeleteFramebuffers(1, np.array([self._canvas_framebuffer]))
        GL.glDeleteTextures(1, np.array([self._canvas_texture]))
        self._canvas_framebuffer, self._canvas_texture = create_canvas_framebuffer(self._width, self._height)

    def __draw_on_canvas(self, on_draw):
        old_fbo = GL.glGetIntegerv(GL.GL_FRAMEBUFFER_BINDING)
        GL.glBindFramebuffer(GL.GL_FRAMEBUFFER, self._canvas_framebuffer)

        on_draw()

        GL.glBindFramebuffer(GL.GL_FRAMEBUFFER, old_fbo)

    def __init(self):
        self.__create_program()
        self.__create_vao()

    def draw(self, on_draw: Callable[[], None]):
        # 先绘制到 canvas buffer
        # 再设置整个 canvas buffer 的透明度
        # 最后将 canvas buffer 绘制到 qt opengl 窗口上
        self.__draw_on_canvas(on_draw)
        GL.glClearColor(0.0, 0.0, 0.0, 0.0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT)
        GL.glBindVertexArray(self._vao)
        GL.glUseProgram(self._program)
        GL.glProgramUniform1f(self._program, self._opacity_loc, self.__canvas_opacity)
        GL.glActiveTexture(GL.GL_TEXTURE0)
        GL.glBindTexture(GL.GL_TEXTURE_2D, self._canvas_texture)
        GL.glDrawArrays(GL.GL_TRIANGLES, 0, 6)
        GL.glBindVertexArray(0)
    
    def setOutputOpacity(self, value):
        self.__canvas_opacity = value

    
