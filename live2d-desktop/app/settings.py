from app.define import AppMode, Live2DVersion

APP_MODE = AppMode.DEBUG

LIVE2D_VERSION = Live2DVersion.V3

CONFIG_PATH = "./config.json"

if LIVE2D_VERSION == Live2DVersion.V3:
    MODEL_JSON_SUFFIX = ".model3.json"
elif LIVE2D_VERSION == Live2DVersion.V2:
    MODEL_JSON_SUFFIX = ".model.json"
else:
    raise Exception("Unknown live2d version: %s", LIVE2D_VERSION)
