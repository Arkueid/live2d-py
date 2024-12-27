#pragma once

// 动作优先级
#define MOTION_PRIORITY_NONE 0
#define MOTION_PRIORITY_IDLE 1
#define MOTION_PRIORITY_NORMAL 2
#define MOTION_PRIORITY_FORCE 3

// default motion groups
#define MOTION_GROUP_IDLE "Idle"
#define MOTION_GROUP_TAP_HEAD "TapHead"

// hit area must be as same as its related motion group name
#define HIT_AREA_HEAD MOTION_GROUP_TAP_HEAD

// model files
#define MODEL_MOC_SUFFIX ".moc3"
#define MODEL_JSON_SUFFIX ".model3.json"

