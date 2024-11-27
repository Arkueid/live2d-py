import turtle

# 网格点列表 (x, y)
points = [(0.081688, 0.656795), (0.053390, 0.657224), (0.082410, 0.624536), (0.053647, 0.625181), ]
triangles  = [(3, 2, 0), (1, 3, 0), ]

# 缩放因子和偏移量
scale = 300
offset_x = -100
offset_y = -100

# 初始化 Turtle
screen = turtle.Screen()
screen.setup(width=800, height=600)
t = turtle.Turtle()
t.speed(0)  # 设置为最快

# 绘制三角形网格
for triangle in triangles:
    t.penup()
    # 获取第一个点
    x1, y1 = points[triangle[0]]
    t.goto(x1 * scale + offset_x, y1 * scale + offset_y)
    t.pendown()

    # 绘制到第二个点
    x2, y2 = points[triangle[1]]
    t.goto(x2 * scale + offset_x, y2 * scale + offset_y)

    # 绘制到第三个点
    x3, y3 = points[triangle[2]]
    t.goto(x3 * scale + offset_x, y3 * scale + offset_y)

    # 闭合回第一个点
    t.goto(x1 * scale + offset_x, y1 * scale + offset_y)

# 完成绘制
t.hideturtle()
screen.mainloop()
