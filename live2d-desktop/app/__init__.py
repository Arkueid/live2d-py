from . import settings, define

if settings.LIVE2D_VERSION == define.Live2DVersion.V3:
    if settings.APP_MODE == define.AppMode.DEBUG:
        import live2d.v3.debug as live2d
    elif settings.APP_MODE == define.AppMode.RELEASE:
        import live2d.v3 as live2d
    else:
        raise Exception("Unknown app mode: %s", settings.APP_MODE)
elif settings.LIVE2D_VERSION == define.Live2DVersion.V2:
    import live2d.v2 as live2d
else:
    raise Exception("Unknown live2d version: %s", settings.LIVE2D_VERSION)

__all__ = ['live2d', 'settings', 'define']