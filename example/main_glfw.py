import glfw
import live2d

# 窗口初始化
def init_window(width, height, title):
    if not glfw.init():
        return None

    window = glfw.create_window(width, height, title, None, None)
    if not window:
        glfw.terminate()
        return None

    glfw.make_context_current(window)
    return window


# 渲染循环
def main():
    window = init_window(800, 600, "Simple GLFW Window")
    if not window:
        print("Failed to create GLFW window")
        return
    
    live2d.InitializeCubism()
    live2d.InitializeGlew()
    live2d.SetGLProperties()

    model = live2d.LAppModel()

    model.LoadAssets('./Resources/Haru', "Haru.model3.json")
    
    while not glfw.window_should_close(window):
        glfw.poll_events()

        live2d.ClearBuffer()

        model.Update(800, 600)
        
        glfw.swap_buffers(window)
    
    glfw.terminate()

    live2d.ReleaseCubism()


if __name__ == "__main__":
    main()
