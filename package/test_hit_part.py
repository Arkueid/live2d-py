import os

import pygame
from pygame.locals import *

import live2d.v3 as live2d
# import live2d.v2 as live2d

import resources

live2d.setLogEnable(False)

import pytest


@pytest.fixture(scope="module")
def model_instance():
    pygame.init()
    pygame.mixer.init()
    live2d.init()
    
    display = (200, 200)
    pygame.display.set_mode(display, DOUBLEBUF | OPENGL)
    pygame.display.set_caption("pygame window")
    
    if live2d.LIVE2D_VERSION == 3:
        live2d.glewInit()
    
    model = live2d.LAppModel()
    
    if live2d.LIVE2D_VERSION == 3:
        model.LoadModelJson(
            os.path.join(resources.RESOURCES_DIRECTORY, "v3/mianfeimox/llny.model3.json")
        )
    else:
        model.LoadModelJson(
            os.path.join(resources.RESOURCES_DIRECTORY, "v2/kasumi2/kasumi2.model.json")
        )
    
    model.Resize(*display)
    
    # 关闭自动眨眼
    model.SetAutoBlinkEnable(False)
    # 关闭自动呼吸
    model.SetAutoBreathEnable(False)
    model.Update()

    return model

test_points = [(i, j) for j in range(0, 200, 50) for i in range(0, 200, 50)]
top_only = [True, False]


@pytest.mark.parametrize("topOnly", top_only)
@pytest.mark.parametrize("point", test_points)
def test_myfunction(benchmark, model_instance, point, topOnly):
    benchmark(model_instance.HitPart, *point, topOnly)
    assert True

