import time

import serial
import serial.tools.list_ports
import numpy as np
import math
import matlab.engine

eng = matlab.engine.start_matlab()
eng.cd(r"D:/UPB/Licenta/MatlabV2", nargout=0)

motor_limits = {
    "Motor 1": (-500000, 500000),
    "Motor 2": (-400000, 800000),
    "Motor 3": (-200000, 400000),
    "Motor 4": (-150000, 150000),
    "Motor 5": (-100000, 100000)
}

motor_pos = {
    "Motor 1": 0,
    "Motor 2": 0,
    "Motor 3": 0,
    "Motor 4": 0,
    "Motor 5": 0
}
gripper_state = 1
lastSaved = 0


def plus_value(motor, step_id, id, canvas, posX, posY, posZ, par = 0):
    min_val, max_val = motor_limits[motor]
    new_value = int(canvas.itemcget(id, "text")) + int(step_id.get())
    if new_value <= max_val:
        canvas.itemconfig(id, text=str(new_value))
        motor_pos[motor] = new_value
        x_actual, y_actual = canvas.coords(id)
        if par == 1:
            x_actual = 350 - 10 * (len(str(new_value)) + (7-len(str(new_value)))//2)
        else:
            x_actual = 125 - 10 * (len(str(new_value)) + (7-len(str(new_value)))//2)
        canvas.coords(id, x_actual, y_actual)
        send_data('C')
        update_coord_pos(posX, posY, posZ, canvas)


def minus_value(motor, step_id, id, canvas, posX, posY, posZ, par = 0):
    min_val, max_val = motor_limits[motor]
    new_value = int(canvas.itemcget(id, "text")) - int(step_id.get())
    if min_val <= new_value:
        canvas.itemconfig(id, text=str(new_value))
        motor_pos[motor] = new_value
        x_actual, y_actual = canvas.coords(id)
        if par == 1:
            x_actual = 350 - 10 * (len(str(new_value)) + (7 - len(str(new_value))) // 2)
        else:
            x_actual = 125 - 10 * (len(str(new_value)) + (7 - len(str(new_value))) // 2)
        canvas.coords(id, x_actual, y_actual)
        send_data('C')
        update_coord_pos(posX, posY, posZ, canvas)


def control_gripper(btn_id, btn_d, btn_c):
    global gripper_state
    gripper_state ^= 1

    if gripper_state == 1:
        btn_img = btn_c
        data = '0' + '\n'
    else:
        btn_img = btn_d
        data = '1' + '\n'

    if arduino:
        arduino.write(data.encode())

    btn_id.config(image=btn_img)
    print(gripper_state)


def save_cfg(canvas, cPos, sPos):
    global lastSaved
    pos = ", ".join([str(round(int(canvas.itemcget(cPos[0], "text")) / 256, 2)), str(round(int(canvas.itemcget(cPos[1], "text")) / 260, 2)),
                     str(round(int(canvas.itemcget(cPos[2], "text")) / 260, 2)), str(round(int(canvas.itemcget(cPos[3], "text")) / 166, 2)),
                     str(round(int(canvas.itemcget(cPos[4], "text")) / 256, 2))])
    sPos[lastSaved].config(text=pos)
    lastSaved = (lastSaved + 1) % 5


def update_pos(btn, values, canvas, posX, posY, posZ):
    pos = btn["text"].split(", ")
    if len(pos) == 1:
        return

    motor_pos["Motor 1"] = float(pos[0]) * 256
    motor_pos["Motor 2"] = float(pos[1]) * 260
    motor_pos["Motor 3"] = float(pos[2]) * 260
    motor_pos["Motor 4"] = float(pos[3]) * 166
    motor_pos["Motor 5"] = float(pos[4]) * 256

    for i in range(len(values)):
        pos[i] = str(int(motor_pos["Motor " + str(i + 1)]))
        canvas.itemconfig(values[i], text=pos[i])
        x_actual, y_actual = canvas.coords(values[i])
        if i % 2 == 1:
            x_actual = 350 - 10 * (len(pos[i]) + (7 - len(pos[i])) // 2)
        else:
            x_actual = 125 - 10 * (len(pos[i]) + (7 - len(pos[i])) // 2)
        canvas.coords(values[i], x_actual, y_actual)
    send_data('A')

    update_coord_pos(posX, posY, posZ, canvas)


def move_cart(btn, values, canvas, posX, posY, posZ):
    pos = btn["text"].split(", ")
    if len(pos) == 1:
        return

    motor_pos["Motor 1"] = float(pos[0]) * 256
    motor_pos["Motor 2"] = float(pos[1]) * 260
    motor_pos["Motor 3"] = float(pos[2]) * 260
    motor_pos["Motor 4"] = float(pos[3]) * 166
    motor_pos["Motor 5"] = float(pos[4]) * 256

    for i in range(len(values)):
        pos[i] = str(int(motor_pos["Motor " + str(i + 1)]))
        canvas.itemconfig(values[i], text=pos[i])
        x_actual, y_actual = canvas.coords(values[i])
        if i % 2 == 1:
            x_actual = 350 - 10 * (len(pos[i]) + (7 - len(pos[i])) // 2)
        else:
            x_actual = 125 - 10 * (len(pos[i]) + (7 - len(pos[i])) // 2)
        canvas.coords(values[i], x_actual, y_actual)

    p1 = [int(canvas.itemcget(posX, "text")), int(canvas.itemcget(posY, "text")), int(canvas.itemcget(posZ, "text"))]
    update_coord_pos(posX, posY, posZ, canvas)
    p2 = [int(canvas.itemcget(posX, "text")), int(canvas.itemcget(posY, "text")), int(canvas.itemcget(posZ, "text"))]

    if p1 == p2:
        return

    eng.workspace['p1'] = matlab.double(p1)  # [350, 100, 100]
    eng.workspace['p2'] = matlab.double(p2)  # [350, 100, 200]
    eng.workspace['limitare_viteza'] = matlab.double(40)

    eng.run("main", nargout=0)

    rezultat = eng.workspace['sirValori']
    print(p1)
    print(p2)
    print(rezultat)
    for poz in rezultat:
        pos = poz.split(",")
        motor_pos["Motor 1"] = int(pos[0])
        motor_pos["Motor 2"] = int(pos[1])
        motor_pos["Motor 3"] = int(pos[2])
        motor_pos["Motor 4"] = int(pos[3])
        motor_pos["Motor 5"] = int(pos[4])
        send_data('C')
        time.sleep(0.03)


def update_coord_pos(posX, posY, posZ, canvas):
    px, py, pz = CD_Scorbot(motor_pos["Motor 1"] / (180 / np.pi) / 256, (motor_pos["Motor 2"] / 260 + 90) / (180/ np.pi), motor_pos["Motor 3"] / (180 / np.pi) / 260)
    canvas.itemconfig(posX, text=str(px))
    canvas.itemconfig(posY, text=str(py))
    canvas.itemconfig(posZ, text=str(pz))


def CD_Scorbot(th1, th2, th3):
    # Parametrii robotului (în mm)
    a1 = 50
    a2 = 300
    a3 = 350
    d1 = 358.5
    d5_tool = 251
    alfa4 = -np.pi / 2

    # 1. Coordonate în plan vertical (fără tool offset)
    var_PR = a2 * np.cos(th2) + a3 * np.cos(th2 - th3)
    var_PZ = a2 * np.sin(th2) + a3 * np.sin(th2 - th3)

    # 2. Tool offset (efect al orientării terminalului)
    r = a1 + var_PR + d5_tool * np.cos(alfa4)
    pz = d1 + var_PZ + d5_tool * np.sin(alfa4)

    # 3. Conversie din polar în cartezian
    px = r * np.sin(th1)
    py = r * np.cos(th1)

    return math.ceil(px), math.ceil(py), math.ceil(pz)


def get_available_ports():
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]


def connect_arduino(port_var, button):
    global arduino
    selected_port = port_var.get()
    try:
        arduino = serial.Serial(selected_port, 250000, timeout=1)
        button.config(text="Conectat")
    except:
        arduino = None


def get_current():
    values = []
    c1 = []
    c2 = []
    c3 = []
    c4 = []
    c5 = []
    if arduino:
        data = 'X' + '\n'
        arduino.write(data.encode())

        buffer_size = 56
        print("Aștept date de la Arduino...")

        while len(c1) < buffer_size:
            line = arduino.readline().decode('utf-8').strip()
            if not line:
                continue
            try:
                values = [float(x) for x in line.split(',')]
                if len(values) == 5:
                    c1.append(values[0])
                    c2.append(values[1])
                    c3.append(values[2])
                    c4.append(values[3])
                    c5.append(values[4])
            except ValueError:
                print("Eroare la conversie:", line)
    else:
        print('Arduino not connected')
    return [c1, c2, c3, c4, c5]


def send_data(type):
    data = ','.join([str(math.ceil(value)) for value in motor_pos.values()]) + '\n'
    if type == 'A':
        data = 'A:' + data
    elif type == 'C':
        data = 'C:' + data
    if arduino:
        arduino.write(data.encode())
        print(f"Sent: {data}")
    else:
        print("Arduino not connected")