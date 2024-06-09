from ui.components.design.motion_editor_design import MotionEditorDesign
from utils.model3json import MotionGroups, Model3Json


class MotionEditor(MotionEditorDesign):

    def __init__(self, model3Json: Model3Json):
        super().__init__(model3Json)
