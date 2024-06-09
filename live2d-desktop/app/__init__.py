from . import settings, define

if settings.APP_MODE == define.AppMode.DEBUG.value:
    import live2d.debug as live2d
elif settings.APP_MODE == define.AppMode.RELEASE.value:
    import live2d
else:
    raise Exception("Unknown app mode: %s", settings.APP_MODE)

__all__ = ['live2d', 'settings', 'define']