# 测试内存

# import live2d.v2 as live2d
import live2d.v3 as live2d
import threading as t

import glfw


if not glfw.init():
    exit()

window = glfw.create_window(200, 200, "test context", None, None)
if not window:
    glfw.terminate()
    exit()

glfw.make_context_current(window)

live2d.init()

if live2d.LIVE2D_VERSION == 3:
    live2d.glewInit()

sem = t.Semaphore(500)

models = []

def run(num):
    model = live2d.LAppModel()
    models.append(model)
    sem.release()

ls = []

for i in range(1000):
    sem.acquire()
    tx = t.Thread(None, run, str(i), (i,))
    ls.append(tx)
    tx.start()


for i in ls:
    i.join()


for i in models:
    models.remove(i)
    del i

glfw.terminate()

live2d.dispose()