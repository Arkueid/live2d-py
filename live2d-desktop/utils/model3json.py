import json
import os.path


class Key:
    FILE_REFERENCES = "FileReferences"
    MOTIONS = "Motions"
    FILE = "File"
    SOUND = "Sound"
    TEXT = "Text"


class Motion:
    __meta: dict

    def __init__(self, d: dict):
        self.__meta = d
        if Key.FILE not in self.__meta:
            self.__meta[Key.FILE] = ""
        if Key.SOUND not in self.__meta:
            self.__meta[Key.SOUND] = ""
        if Key.TEXT not in self.__meta:
            self.__meta[Key.TEXT] = ""

    def file(self):
        return self.__meta[Key.FILE]

    def sound(self):
        return self.__meta[Key.SOUND]

    def text(self):
        return self.__meta[Key.TEXT]

    def set_file(self, value: str):
        self.__meta[Key.FILE] = value

    def set_sound(self, value: str):
        self.__meta[Key.SOUND] = value

    def set_text(self, value: str):
        self.__meta[Key.TEXT] = value

    def meta(self):
        return self.__meta


class MotionGroup:
    __meta: list

    def __init__(self, ls=None):
        if ls is None:
            ls = []
        self.__meta = ls

    def __iter__(self):
        for i in self.__meta:
            yield Motion(i)

    def motion(self, nr: int):
        return Motion(self.__meta[nr])

    def add(self, motion: Motion):
        self.__meta.append(motion.meta())

    def remove(self, motion: Motion):
        self.__meta.remove(motion.meta())

    def meta(self):
        return self.__meta


class MotionGroups:
    __meta: dict

    def __init__(self, d: dict):
        self.__meta = d

    def __iter__(self):
        for key, value in self.__meta.items():
            yield key, MotionGroup(value)

    def group(self, name: str) -> MotionGroup:
        return MotionGroup(self.__meta[name])

    def add(self, name: str, motionGroup: MotionGroup):
        self.__meta[name] = motionGroup.meta()

    def remove(self, name: str):
        self.__meta.pop(name)

    def meta(self):
        return self.__meta
    
    def set_meta(self, meta):
        self.__meta = meta


# todo
class Model3Json:
    __meta: dict
    __src_file: str
    __src_dir: str

    def __init__(self, d=None):
        if d is None:
            d = {}
        self.__meta = d

    def motion_groups(self) -> MotionGroups:
        return MotionGroups(self.__meta[Key.FILE_REFERENCES][Key.MOTIONS])

    def src_dir(self):
        return self.__src_dir

    def load(self, fileName):
        d = None
        with open(fileName, 'r', encoding='utf-8') as f:
            self.__meta = json.loads(f.read())
        self.__src_file = fileName
        self.__src_dir = os.path.split(fileName)[0]
        self.backup()

    def __write_to(self, filename):
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(json.dumps(self.__meta, ensure_ascii=False, indent=4))

    def backup(self):
        if os.path.exists(self.__src_file + ".bak"):
            return
        self.__write_to(self.__src_file + ".bak")

    def save(self):
        self.__write_to(self.__src_file)


if __name__ == '__main__':
    m = Model3Json()
    m.load("../Resources/Haru/Haru.model3.json")
    mg = MotionGroup()
    mg.add(Motion({
        "File": "123.motion3.json"
    }))
    m.motion_groups().add("123", mg)


