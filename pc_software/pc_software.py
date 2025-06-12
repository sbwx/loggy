import sys
import os
import random
import datetime
import csv
import time
import serial
import serial.tools.list_ports
from PyQt5 import QtWidgets, QtCore
import pyqtgraph as pg
import numpy as np

CHECK_CONN_INTERVAL_MS = 1000
SLIDING_WINDOW_SEC = 60

class SerialEngine:
    def __init__(self):
        self.ser = None

    def open_port(self, port):
        try:
            if self.ser and self.ser.is_open:
                self.close_port()
            self.ser = serial.Serial(port, baudrate=4800, timeout=0.1)
            print(f"[Serial] Opened {port}")
            self.write_serial("y,0,0,0,0,0kkk")  # Connected
            time.sleep(0.05)
        except Exception as e:
            print(f"[Serial] Open error: {e}")

    def close_port(self):
        if self.ser and self.ser.is_open:
            try:

                self.write_serial("z,0,0,0,0,0kkk")  # Disconnected
                time.sleep(0.05)
                self.ser.flush()

            except Exception as e:
                print(f"[Serial] Error sending 'z': {e}")
            self.ser.close()
        self.ser = None


    def write_serial(self, message):
        if self.ser and self.ser.is_open:
            # message += '\n'
            self.ser.write(message.encode('utf-8'))
            time.sleep(0.05)  # 延迟 50ms
            print(f"[Serial] Sent: {message.strip()}")
        else:
            print("[Serial] Port not open")

    def read_data(self):
        if self.ser and self.ser.is_open:
            resp = self.ser.read(100)
            if resp:
                print(f"[Serial] Received: {resp.decode(errors='ignore').strip()}")

    def readline(self):
        if self.ser and self.ser.is_open:
            return self.ser.readline()
        return b""

    def list_ports(self):
        return [p.device for p in serial.tools.list_ports.comports()]

#===============================
# Device Simulator
#===============================
class DeviceSimulator(QtCore.QObject):
    newData = QtCore.pyqtSignal(dict)
    stateChanged = QtCore.pyqtSignal(dict)

    def __init__(self):
        super().__init__()
        self.engine = SerialEngine()
        self.connected = False
        self.sim_enabled = False
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.generate_data)
        self.timer.start(500)
        self.state = self._default_state()

    def _default_state(self):
        return {
            'alarm_high': {f'CH{i}': 5.000 for i in range(1, 9)},
            'alarm_low': {f'CH{i}': 0.000 for i in range(1, 9)},
            'alarm_state': {f'CH{i}': {'high': False, 'low': False} for i in range(1, 9)},
            'alarm_mode': {f'CH{i}': 'Live' for i in range(1, 9)},
            'input_range': {f'CH{i}': '+/- 1V' for i in range(1, 9)},
            'temperature_config': {f'CH{i}': {} for i in range(1, 9)},
            'temp_mode': {f'CH{i}': False for i in range(1, 5)},
            'rtc': datetime.datetime.now(),
            'optical': True,
        }

    def generate_data(self):
        if not self.sim_enabled:
            return
        d = {}
        for i in range(1, 5):
            ch = f'CH{i}'
            d[ch] = random.uniform(20, 30) if self.state['temp_mode'][ch] else random.uniform(-10, 10)
        for i in range(5, 8):
            d[f'CH{i}'] = random.uniform(-10, 10)
        d['CH8'] = random.uniform(20, 30)

        self.state['rtc'] = datetime.datetime.now()
        for ch, val in d.items():
            self.state['alarm_state'][ch]['high'] = val > self.state['alarm_high'][ch]
            self.state['alarm_state'][ch]['low'] = val < self.state['alarm_low'][ch]

        self.newData.emit(d)
        self.stateChanged.emit(self.state)

    def list_ports(self):
        return self.engine.list_ports()

    def open_port(self, port):
        self.engine.open_port(port)
        if self.engine.ser and self.engine.ser.is_open:
            self.connected = True
        self.stateChanged.emit(self.state)

    def close_port(self):
        self.engine.close_port()
        self.connected = False
        self.stateChanged.emit(self.state)

class ChannelConfigWidget(QtWidgets.QWidget):
    def __init__(self, ch_name, parent=None):
        super().__init__(parent)
        self.ch = ch_name
        idx = int(ch_name[2:])
        layout = QtWidgets.QVBoxLayout(self)
        form = QtWidgets.QFormLayout()
        layout.addLayout(form)

        self.highEdit = QtWidgets.QLineEdit()
        self.lowEdit = QtWidgets.QLineEdit()
        hl = QtWidgets.QHBoxLayout()
        hl.addWidget(QtWidgets.QLabel("High:"))
        hl.addWidget(self.highEdit)
        hl.addSpacing(10)
        hl.addWidget(QtWidgets.QLabel("Low:"))
        hl.addWidget(self.lowEdit)
        form.addRow("Alarm Thresholds", hl)

        self.modeCombo = QtWidgets.QComboBox()
        self.modeCombo.addItems(["Disabled", "Live", "Latching"])
        form.addRow("Alarm Mode", self.modeCombo)
        
        if idx <= 4:
            self.rangeCombo = QtWidgets.QComboBox()
            self.rangeCombo.addItems(["+/- 1V", "+/- 10V"])
            form.addRow("Input Range", self.rangeCombo)

            # ✅ 先创建，不连 signal
            self.tempCheck = QtWidgets.QCheckBox("Enable Resistive Temp Mode")
            form.addRow(self.tempCheck)

            self.currentCombo = QtWidgets.QComboBox()
            self.currentCombo.addItems(["10uA", "200uA"])
            form.addRow("Temp Mode Current", self.currentCombo)
            self.currentCombo.setVisible(False)

            # ✅ 控件都建好后，再连接信号，避免初始化时触发回调
            self.tempCheck.stateChanged.connect(self._on_temp_toggled)

        else:
            self.rangeCombo = self.tempCheck = None

        grp = QtWidgets.QGroupBox("Current & Alarm State")
        gl = QtWidgets.QFormLayout(grp)
        self.valueLabel = QtWidgets.QLabel("---")
        self.highAlarmLabel = QtWidgets.QLabel("High Alarm: OFF")
        self.lowAlarmLabel = QtWidgets.QLabel("Low Alarm: OFF")
        self.modeLabel = QtWidgets.QLabel("Alarm Mode: ---")
        self.rangeLabel = QtWidgets.QLabel("Input: ---")
        gl.addRow(self.modeLabel)
        gl.addRow(self.rangeLabel)
        gl.addRow("Value", self.valueLabel)
        gl.addRow(self.highAlarmLabel)
        gl.addRow(self.lowAlarmLabel)
        layout.addWidget(grp)
        layout.addStretch()

    def _on_temp_toggled(self, state):
        enabled = (state == QtCore.Qt.Checked)
        if self.rangeCombo:
            self.rangeCombo.setEnabled(not enabled)
        if self.currentCombo:
            self.currentCombo.setVisible(enabled)


    def load_state(self, state):
        idx = int(self.ch[2:])
        # 初始加载配置用（可选保留）
        if not self.highEdit.text():
            self.highEdit.setText(f"{state['alarm_high'][self.ch]:.3f}")
        if not self.lowEdit.text():
            self.lowEdit.setText(f"{state['alarm_low'][self.ch]:.3f}")
        if self.modeCombo.findText(state['alarm_mode'][self.ch]) != -1:
            self.modeCombo.setCurrentText(state['alarm_mode'][self.ch])

        if idx <= 4 and self.rangeCombo:
            if self.tempCheck.isChecked() != state['temp_mode'][self.ch]:
                self.tempCheck.setChecked(state['temp_mode'][self.ch])
            if not self.rangeCombo.currentText():
                self.rangeCombo.setCurrentText(state['input_range'])

            temp_cfg = state['temperature_config'].get(self.ch, {})
            current_uA = temp_cfg.get('current_uA', 10)
            self.currentCombo.setCurrentText(f"{current_uA}uA")

    def save_state(self, state, serial_engine):
            try:
                hi = float(self.highEdit.text())
                lo = float(self.lowEdit.text())
                if hi < lo:
                    raise ValueError
            except:
                raise ValueError(f"{self.ch}: invalid alarm thresholds")

            idx = int(self.ch[2:])
            ch_letter = chr(ord('A') + idx - 1)  # CH1 -> 'A', CH2 -> 'B', ...

            state['alarm_high'][self.ch] = hi
            state['alarm_low'][self.ch] = lo

            mode_text = self.modeCombo.currentText()
            mode_map = {"Disabled": 1, "Live": 2, "Latching": 3}
            mode = mode_map.get(mode_text, 1)
            state['alarm_mode'][self.ch] = mode_text

            rng = 0
            unit = 0

            if idx <= 4 and self.rangeCombo:
                if self.tempCheck.isChecked():
                    unit = 1
                    state['temp_mode'][self.ch] = True

                    # 获取电流设置（10uA or 200uA）并编码为 1 or 10 传输
                    current_str = self.currentCombo.currentText()
                    current_uA = int(current_str.replace("uA", ""))
                    rng = 1 if current_uA == 10 else 10
                    state['temperature_config'][self.ch]['current_uA'] = current_uA

                else:
                    unit = 0
                    state['temp_mode'][self.ch] = False

                    # 普通电压模式：+/-1V / +/-10V
                    rng_text = self.rangeCombo.currentText()
                    range_map = {"+/- 1V": 1, "+/- 10V": 10}
                    rng = range_map.get(rng_text, 10)
                    state['input_range'][self.ch] = rng_text


            elif idx == 8:
                unit = 1
            elif 5 <= idx <= 7:
                rng = 0
                unit = 0

            # Send updated format: letter,htv,ltv,alarm,range,unit\r\n
            if idx >= 5:
                # CH5~CH8 特殊规则：E,htv,ltv,alarm,0,0
                msg = f"{ch_letter},{int(hi * 1000)},{int(lo * 1000)},{mode},0,0kkk"
            else:
                msg = f"{ch_letter},{int(hi * 1000)},{int(lo * 1000)},{mode},{rng},{unit}kkk"
            
            # main_win = self.window()  # 获取 MainWindow 实例
            # main_win.ack_z = False
            # main_win._pending_msg = msg
            # main_win._pending_engine = serial_engine

            # # 清空旧数据、发送一次，然后启动重发定时器
            # serial_engine.ser.reset_input_buffer()
            serial_engine.write_serial(msg)
            # main_win._send_timer.start()
                    

    def update_current(self, value, alarm_state, device_state):
        idx = int(self.ch[2:])

        # 实时数值单位
        if idx <= 4:
            unit = "°C" if device_state['temp_mode'][self.ch] else "V"
        elif idx <= 7:
            unit = "m/s²"
        else:
            unit = "°C"

        # ✅ 只更新只读显示内容
        self.valueLabel.setText(f"{value:.3f} {unit}")
        self.highAlarmLabel.setText(f"High Alarm: {'ON' if alarm_state['high'] else 'OFF'}")
        self.lowAlarmLabel.setText(f"Low Alarm: {'ON' if alarm_state['low'] else 'OFF'}")

        self.modeLabel.setText(f"Alarm Mode: {device_state['alarm_mode'][self.ch]}")

        if idx <= 4:
            if device_state['temp_mode'][self.ch]:
                current_uA = device_state['temperature_config'][self.ch].get('current_uA', 10)
                self.rangeLabel.setText(f"Temp Current: {current_uA}uA")
            else:
                current_range = device_state['input_range'].get(self.ch, "+/- 10V")
                self.rangeLabel.setText(f"Input: {current_range}")
        else:
            self.rangeLabel.setText("Input: ---")




class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Device Config & Data Viewer")
        self.resize(1200, 900)

        self.ack_z = False
        self._pending_msg = None
        self._pending_engine = None
        self._send_timer = QtCore.QTimer(self)
        self._send_timer.setInterval(100)
        self._send_timer.timeout.connect(self._retry_send)

        self.device = DeviceSimulator()
        self.record_file = None
        self.csv_writer = None
        self.recording = False
        self.fixed_view = False

        self.startTime = datetime.datetime.now()
        self.timeData = []
        self.chData = {f'CH{i}': [] for i in range(1, 9)}
        self.latest = None

        self.fakeTimer = QtCore.QTimer(self)
        self.fakeTimer.timeout.connect(self._gen_fake)

        self._build_ui()

        self.device.newData.connect(self._on_new_data)
        self.device.stateChanged.connect(self._on_state_changed)

        QtCore.QTimer.singleShot(0, self._refresh_ports)
        port_timer = QtCore.QTimer(self)
        port_timer.timeout.connect(self._refresh_ports)
        port_timer.start(CHECK_CONN_INTERVAL_MS)

        self.read_timer = QtCore.QTimer(self)
        self.read_timer.timeout.connect(self._read_serial)
        self.read_timer.start(200)

    def _build_ui(self):
        cw = QtWidgets.QWidget()
        self.setCentralWidget(cw)
        h = QtWidgets.QHBoxLayout(cw)

        scroll = QtWidgets.QScrollArea()
        scroll.setWidgetResizable(True)
        cfg = QtWidgets.QWidget()
        self.cfg_layout = QtWidgets.QVBoxLayout(cfg)
        scroll.setWidget(cfg)
        h.addWidget(scroll, 3)
        self._build_config_panel()

        h.addWidget(self._build_plot_panel(), 1)

        menubar = self.menuBar().addMenu("File")
        replayAction = QtWidgets.QAction("Replay CSV", self)
        replayAction.triggered.connect(self._replay_csv)
        menubar.addAction(replayAction)

    def _build_config_panel(self):
        L = self.cfg_layout

        grp = QtWidgets.QGroupBox("Device Connection")
        ly = QtWidgets.QHBoxLayout(grp)
        self.portCombo = QtWidgets.QComboBox()
        connectBtn = QtWidgets.QPushButton("Connect")
        connectBtn.clicked.connect(self._connect)
        disconnectBtn = QtWidgets.QPushButton("Disconnect")
        disconnectBtn.clicked.connect(self._disconnect)
        ly.addWidget(self.portCombo)
        ly.addWidget(connectBtn)
        ly.addWidget(disconnectBtn)
        L.addWidget(grp)

        self.tabs = QtWidgets.QTabWidget()
        self.ch_widgets = {}
        for i in range(1, 9):
            ch = f"CH{i}"
            w = ChannelConfigWidget(ch)
            w.load_state(self.device.state)
            self.ch_widgets[ch] = w
            self.tabs.addTab(w, ch)
        L.addWidget(self.tabs)

        btns = [
            ("Apply Config", self._apply_config),
            ("Clear Data", self._clear_data),
            ("Toggle 5s View", self._toggle_fixed_view),
            ("Send Plot Data", self._send_plot_data),
            ("Start Displaying", self._start_recording),
            ("Stop Displaying", self._stop_recording),
            ("Sync Time to Device", self._sync_time),
            ("Load SD Card Files", self._load_sd_card_files),
        ]

        for text, fn in btns:
            b = QtWidgets.QPushButton(text)
            b.clicked.connect(fn)
            L.addWidget(b)

        # 保存引用的 SD 卡按钮（默认禁用）
        self.sd_start_btn = QtWidgets.QPushButton("Start SD Card Recording")
        self.sd_start_btn.clicked.connect(self._start_sd_card_recording)
        self.sd_start_btn.setEnabled(True)
        L.addWidget(self.sd_start_btn)

        self.sd_stop_btn = QtWidgets.QPushButton("Stop SD Card Recording")
        self.sd_stop_btn.clicked.connect(self._stop_sd_card_recording)
        self.sd_stop_btn.setEnabled(True)
        L.addWidget(self.sd_stop_btn)

        self.fakeBtn = QtWidgets.QPushButton("Start Fake Data")
        self.fakeBtn.setCheckable(True)
        self.fakeBtn.clicked.connect(self._toggle_fake)
        L.addWidget(self.fakeBtn)

        L.addStretch()
    
    def _build_plot_panel(self):
        p = QtWidgets.QWidget()
        ly = QtWidgets.QVBoxLayout(p)

        self.voltagePlot = pg.PlotWidget(title="Voltage Channels")
        self.voltagePlot.setFixedSize(600, 200)
        self.voltagePlot.addLegend()
        self.voltagePlot.showGrid(True, True)
        colors_v = ['r', 'g', 'b', 'm']
        self.voltageCurves = {}
        for i, col in enumerate(colors_v, start=1):
            ch = f"CH{i}"
            self.voltageCurves[ch] = self.voltagePlot.plot(
                pen=pg.mkPen(col, width=2),
                symbol='x', symbolSize=5,
                name=ch
            )
        ly.addWidget(self.voltagePlot)

        self.accelPlot = pg.PlotWidget(title="Acceleration Channels")
        self.accelPlot.setFixedSize(600, 200)
        self.accelPlot.addLegend()
        self.accelPlot.showGrid(True, True)
        colors_a = ['c', 'y', 'orange']
        self.accelCurves = {}
        for idx, ch_i in enumerate(range(5, 8)):
            ch = f"CH{ch_i}"
            self.accelCurves[ch] = self.accelPlot.plot(
                pen=pg.mkPen(colors_a[idx], width=2),
                symbol='o', symbolSize=5,
                name=ch
            )
        ly.addWidget(self.accelPlot)

        self.tempPlot = pg.PlotWidget(title="Temperature Channels")
        self.tempPlot.setFixedSize(600, 200)
        self.tempPlot.addLegend()
        self.tempPlot.showGrid(True, True)
        colors_t = ['r', 'g', 'b', 'm', 'purple']
        self.tempCurves = {}
        for i in range(1, 5):
            ch = f"CH{i}"
            self.tempCurves[ch] = self.tempPlot.plot(
                pen=pg.mkPen(colors_t[i-1], width=2),
                symbol='t', symbolSize=5,
                name=ch
            )
        self.tempCurves['CH8'] = self.tempPlot.plot(
            pen=pg.mkPen(colors_t[4], width=2),
            symbol='d', symbolSize=5,
            name='CH8'
        )
        ly.addWidget(self.tempPlot)

        self.hoverLabel = QtWidgets.QLabel("Hover over a plot for details")
        ly.addWidget(self.hoverLabel)

        for plt in (self.voltagePlot, self.accelPlot, self.tempPlot):
            plt.scene().sigMouseMoved.connect(self._plot_mouse_moved)

        return p
    def _refresh_ports(self):
        ports = self.device.list_ports()
        curr = self.portCombo.currentText()
        self.portCombo.clear()
        self.portCombo.addItems(ports)
        if curr in ports:
            self.portCombo.setCurrentText(curr)

    def _connect(self):
        port = self.portCombo.currentText()
        if port:
            self.device.open_port(port)

    def _disconnect(self):
        self.device.close_port()


    def _apply_config(self):
        if self.recording:
            return
        current_widget = self.tabs.currentWidget()
        if current_widget:
            current_widget.save_state(self.device.state, self.device.engine)
        self._update_plot_visibility()
        # 强制刷新所有通道的 Alarm State 显示
        for ch, w in self.ch_widgets.items():
            dummy_value = self.latest[ch] if self.latest and ch in self.latest else 0.0
            w.update_current(dummy_value, self.device.state['alarm_state'][ch], self.device.state)


    def _update_plot_visibility(self):
        for i in range(1, 5):
            ch = f"CH{i}"
            is_temp = self.device.state['temp_mode'][ch]
            xs = np.array(self.timeData)
            ys = np.array(self.chData[ch])
            valid = ~np.isnan(ys)

            if is_temp:
                # 从 Voltage 转到 Temp 图
                if ch in self.tempCurves:
                    self.tempCurves[ch].setData(xs[valid], ys[valid])
                if ch in self.voltageCurves:
                    self.voltageCurves[ch].clear()
            else:
                # 从 Temp 转回 Voltage 图
                if ch in self.voltageCurves:
                    self.voltageCurves[ch].setData(xs[valid], ys[valid])
                if ch in self.tempCurves:
                    self.tempCurves[ch].clear()


    def _clear_data(self):
        self.timeData.clear()
        for lst in self.chData.values():
            lst.clear()
        self.startTime = datetime.datetime.now()
        for curves in (self.voltageCurves, self.accelCurves, self.tempCurves):
            for c in curves.values():
                c.clear()

    def _toggle_fixed_view(self):
        self.fixed_view = not self.fixed_view

    def _toggle_fake(self, checked):
        self.device.sim_enabled = checked
        self.fakeBtn.setText("Stop Fake Data" if checked else "Start Fake Data")

    def _gen_fake(self):
        d = {}
        for i in range(1, 5):
            ch = f"CH{i}"
            d[ch] = random.uniform(20, 30) if self.device.state['temp_mode'][ch] else random.uniform(-10, 10)
        for i in range(5, 8):
            ch = f"CH{i}"
            d[ch] = random.uniform(-10, 10)
        d['CH8'] = random.uniform(20, 30)
        self._on_new_data(d)
    
    def _send_plot_data(self):
        if not self.latest:
            return
        for ch, val in self.latest.items():
            self.device.engine.send_command(ch, f"{val:.3f}")
        self.device.engine.read_data()

    def _start_recording(self):
        self.start_rec_time = None
        self.recording = True
        self.device.engine.write_serial("I,0,0,0,0,0kkk")


    def _stop_recording(self):
        self.recording = False
        if self.record_file:
            self.record_file.close()
            self.record_file = None
            self.csv_writer = None
        self.device.engine.write_serial("J,0,0,0,0,0kkk")

    def _start_sd_card_recording(self):
        self.device.engine.write_serial("L,0,0,0,0,0kkk")
        print("[GUI] Sent: Start SD Card Recording")

    def _stop_sd_card_recording(self):
        self.device.engine.write_serial("M,0,0,0,0,0kkk")
        print("[GUI] Sent: Stop SD Card Recording")

    def _load_sd_card_files(self):
        dirpath = QtWidgets.QFileDialog.getExistingDirectory(self, "Select SD Card Directory", "")
        if not dirpath:
            return
        files = sorted(f for f in os.listdir(dirpath) if f.lower().endswith(".csv"))
        if not files:
            QtWidgets.QMessageBox.information(self, "SD Card", "No CSV files found.")
            return
        dlg = QtWidgets.QDialog(self)
        dlg.setWindowTitle("SD Card Files")
        v = QtWidgets.QVBoxLayout(dlg)
        listw = QtWidgets.QListWidget()
        listw.addItems(files)
        v.addWidget(listw)
        btns = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.Close)
        btns.rejected.connect(dlg.reject)
        v.addWidget(btns)

        def on_double(item):
            path = os.path.join(dirpath, item.text())
            dlg.accept()
            self._load_csv(path)

        listw.itemDoubleClicked.connect(on_double)
        dlg.exec_()

    def _retry_send(self):
        if not self.ack_z and self._pending_msg and self._pending_engine:
            # 仍未收到 'z'，重发
            self._pending_engine.ser.reset_input_buffer()
            self._pending_engine.write_serial(self._pending_msg)
        else:
            # 收到 'z' 或无 pending，停止定时器
            self._send_timer.stop()

    def _read_serial(self):
        if self.device.sim_enabled:
            return

        raw = self.device.engine.readline()
        if not raw:
            return

        try:
            decoded = raw.decode(errors='ignore').strip()
            print(f"[Serial] Received: {decoded}")
            parts = decoded.split(',')
            if not parts:
                return

            prefix = parts[0]
            values = parts[1:]

            if prefix == 'A' and len(values) == 8:
                # 实时数据
                d = {f"CH{i+1}": float(values[i]) for i in range(8)}
                self._on_new_data(d)

            elif prefix == 'B' and len(values) == 8:
                # 高报警状态
                for i in range(8):
                    ch = f"CH{i+1}"
                    self.device.state['alarm_state'][ch]['high'] = bool(int(values[i]))

            elif prefix == 'C' and len(values) == 8:
                # 低报警状态
                for i in range(8):
                    ch = f"CH{i+1}"
                    self.device.state['alarm_state'][ch]['low'] = bool(int(values[i]))

            # elif prefix == 'D' and len(values) == 8:
            #     range_map = {"10": "+/- 10V", "1": "+/- 1V"}
            #     for i in range(8):
            #         ch = f"CH{i+1}"
            #         self.device.state['input_range'][ch] = range_map.get(values[i].strip(), "+/- 10V")

            elif prefix == 'D' and len(values) == 8:
                for i in range(8):
                    ch = f"CH{i+1}"
                    val = values[i].strip()

                    # 判断是否为 temperature mode
                    if self.device.state.get('temp_mode', {}).get(ch, False):
                        # 温度模式：1 -> 10uA, 10 -> 200uA
                        current_map = {"1": "10uA", "10": "200uA"}
                        self.device.state['temperature_config'][ch]['current_uA'] = int(current_map.get(val, "200uA").replace("uA", ""))
                    else:
                        # 普通电压模式：1 -> +/-1V, 10 -> +/-10V
                        range_map = {"1": "+/- 1V", "10": "+/- 10V"}
                        self.device.state['input_range'][ch] = range_map.get(values[i].strip(), "+/- 10V")

            elif prefix == 'E' and len(values) == 8:
                # 报警模式
                mode_map = {"1": "Disabled", "2": "Live", "3": "Latching"}
                for i in range(8):
                    ch = f"CH{i+1}"
                    self.device.state['alarm_mode'][ch] = mode_map.get(values[i].strip(), "Live")
            
            elif prefix == 'F'and len(values) == 8:
                # 禁用 SD 卡按钮
                self.sd_start_btn.setEnabled(False)
                self.sd_stop_btn.setEnabled(False)
                print("[Serial] SD Card Buttons Disabled")
            
            elif prefix == 'G'and len(values) == 8:
                # 启用 SD 卡按钮
                self.sd_start_btn.setEnabled(True)
                self.sd_stop_btn.setEnabled(True)
                print("[Serial] SD Card Buttons Enabled")

            elif prefix == 'H'and len(values) == 8:
                ch_num = int(values[0].strip())
                ch = f"CH{ch_num}"
                widget = self.ch_widgets.get(ch)
                if widget:
                    idx = self.tabs.indexOf(widget)
                    if idx != -1:
                        self.tabs.setCurrentIndex(idx)

            if prefix == 'z':
                self.ack_z = True
                if self._send_timer.isActive():
                    self._send_timer.stop()
                return
    
            # 显示刷新（不更新配置面板，只更新显示）
            if prefix != 'A':
                # 用最新数据刷新报警灯等
                fake_data = self.latest if self.latest else {f"CH{i+1}": 0.0 for i in range(8)}
                self._on_new_data(fake_data)

        except Exception as e:
            print(f"[Serial Read Error] {e}")

    def _plot_all(self):
        # Voltage: CH1–CH4
        for i in range(1,5):
            ch = f"CH{i}"
            xs = np.array(self.timeData)
            ys = np.array(self.chData[ch])
            valid = ~np.isnan(ys)
            if np.any(valid):
                self.voltageCurves[ch].setData(xs[valid], ys[valid])
            else:
                self.voltageCurves[ch].clear()

        # Acceleration: CH5–CH7
        for i in range(5,8):
            ch = f"CH{i}"
            xs = np.array(self.timeData)
            ys = np.array(self.chData[ch])
            valid = ~np.isnan(ys)
            if np.any(valid):
                self.accelCurves[ch].setData(xs[valid], ys[valid])
            else:
                self.accelCurves[ch].clear()

        # Temperature: CH1–CH4 (if in temp mode) + CH8
        for i in range(1,5):
            ch = f"CH{i}"
            xs = np.array(self.timeData); ys = np.array(self.chData[ch]); valid = ~np.isnan(ys)
            if self.device.state['temp_mode'][ch] and np.any(valid):
                self.tempCurves[ch].setData(xs[valid], ys[valid])
            else:
                self.tempCurves[ch].clear()
        # CH8 always
        xs8 = np.array(self.timeData); ys8 = np.array(self.chData['CH8']); v8 = ~np.isnan(ys8)
        if np.any(v8):
            self.tempCurves['CH8'].setData(xs8[v8], ys8[v8])
        else:
            self.tempCurves['CH8'].clear()



    def _on_new_data(self, d):
        self.latest = d

        # ✅ 总是更新报警灯等状态显示
        for ch, w in self.ch_widgets.items():
            w.update_current(d[ch], self.device.state['alarm_state'][ch], self.device.state)

        # ✅ 只有录制时才绘图/写入CSV
        if not self.recording:
            return

        now = datetime.datetime.now()
        t = (now - self.startTime).total_seconds()
        self.timeData.append(t)

        for ch, val in d.items():
            self.chData[ch].append(val)
        
        now = datetime.datetime.now()
        if self.recording:
            if self.start_rec_time is None:
                self.start_rec_time = now
                fname = self.start_rec_time.strftime("%Y-%m-%d_%H-%M-%S") + ".csv"
                os.makedirs("records", exist_ok=True)
                self.record_file = open(os.path.join("records", fname), "w", newline="", encoding="utf-8")
                self.csv_writer = csv.writer(self.record_file)
                units = []
                for i in range(1, 9):
                    if i <= 4:
                        units.append("°C" if self.device.state['temp_mode'][f"CH{i}"] else "V")
                    elif i <= 7:
                        units.append("m/s²")
                    else:
                        units.append("°C")
                header = ["Timestamp"] + [f"CH{i} ({units[i-1]})" for i in range(1, 9)]
                self.csv_writer.writerow(header)
            tstr = now.strftime("%Y-%m-%d_%H-%M-%S.%f")[:-3]
            row = [tstr] + [f"{d.get(f'CH{i}', float('nan')):.3f}" for i in range(1, 9)]
            self.csv_writer.writerow(row)

        self.latest = d
        t = (now - self.startTime).total_seconds()
        self.timeData.append(t)
        for ch, val in d.items():
            self.chData[ch].append(val)


        # Voltage: CH1–CH4
        for i in range(1, 5):
            ch = f"CH{i}"
            xs = np.array(self.timeData)
            ys = np.array(self.chData[ch])
            valid = ~np.isnan(ys)
            if not self.device.state['temp_mode'][ch] and np.any(valid):
                self.voltageCurves[ch].setData(xs[valid], ys[valid])
            else:
                self.voltageCurves[ch].clear()

        # Acceleration: CH5–CH7
        for i in range(5, 8):
            ch = f"CH{i}"
            xs = np.array(self.timeData)
            ys = np.array(self.chData[ch])
            valid = ~np.isnan(ys)
            if np.any(valid):
                self.accelCurves[ch].setData(xs[valid], ys[valid])
            else:
                self.accelCurves[ch].clear()

        # Temperature: CH1–CH4 (if in temp mode) + CH8
        for i in range(1, 5):
            ch = f"CH{i}"
            xs = np.array(self.timeData)
            ys = np.array(self.chData[ch])
            valid = ~np.isnan(ys)
            if self.device.state['temp_mode'][ch] and np.any(valid):
                self.tempCurves[ch].setData(xs[valid], ys[valid])
            else:
                self.tempCurves[ch].clear()

        # CH8 temperature always displayed
        ch8_xs = np.array(self.timeData)
        ch8_ys = np.array(self.chData['CH8'])
        ch8_valid = ~np.isnan(ch8_ys)
        if np.any(ch8_valid):
            self.tempCurves['CH8'].setData(ch8_xs[ch8_valid], ch8_ys[ch8_valid])
        else:
            self.tempCurves['CH8'].clear()

        if self.fixed_view:
            lb = max(0, t - 5)
            for plt in (self.voltagePlot, self.accelPlot, self.tempPlot):
                plt.setXRange(lb, t)
        else:
            if t > SLIDING_WINDOW_SEC:
                for plt in (self.voltagePlot, self.accelPlot, self.tempPlot):
                    plt.setXRange(t - SLIDING_WINDOW_SEC, t)

        for ch, w in self.ch_widgets.items():
            w.update_current(d[ch], self.device.state['alarm_state'][ch], self.device.state)

        self._plot_all()


    def _plot_mouse_moved(self, pos):
        for plt in (self.voltagePlot, self.accelPlot, self.tempPlot):
            if plt.sceneBoundingRect().contains(pos):
                mp = plt.getViewBox().mapSceneToView(pos)
                x = mp.x()
                if not self.timeData:
                    return
                idx = min(range(len(self.timeData)), key=lambda i: abs(self.timeData[i] - x))
                text = f"t={self.timeData[idx]:.2f}s: " + ", ".join(
                    f"{ch}:{self.chData[ch][idx]:.3f}" for ch in self.chData
                )
                self.hoverLabel.setText(text)

    def _on_state_changed(self, state):
        for w in self.ch_widgets.values():
            w.load_state(state)

    def _replay_csv(self):
        fn, _ = QtWidgets.QFileDialog.getOpenFileName(self, "Open CSV File", "", "CSV Files (*.csv)")
        if fn:
            self._load_csv(fn)

    def _load_csv(self, filepath):
        self._clear_data()
        with open(filepath, encoding="utf-8") as f:
            reader = csv.reader(f)
            header = next(reader)
            start = None
            for row in reader:
                try:
                    dt = datetime.datetime.strptime(row[0], "%Y-%m-%d_%H-%M-%S.%f")
                except ValueError:
                    print(f"[Error] Failed to parse time: {row[0]}")
                    continue
                if start is None:
                    start = dt
                t = (dt - start).total_seconds()
                self.timeData.append(t)

                for i in range(1, 9):
                    ch = f"CH{i}"
                    try:
                        val = float(row[i]) if i < len(row) and row[i].strip() else float('nan')
                    except ValueError:
                        val = float('nan')
                    self.chData[ch].append(val)

        last = {f"CH{i}": self.chData[f"CH{i}"][-1] if self.chData[f"CH{i}"] else 0.0 for i in range(1, 9)}
        self._on_new_data(last)
        self._plot_all()


    def _sync_time(self):
        now = datetime.datetime.now()
        millisecond = now.microsecond // 1000
        msg = f"T,{now.month},{now.day},{now.hour},{now.minute},{now.second}k"
        # self.device.engine.write_serial(msg)
        self.ack_z = False
        self._pending_msg = msg
        self._pending_engine = self.device.engine

        if self._pending_engine.ser and self._pending_engine.ser.is_open:
            self._pending_engine.ser.reset_input_buffer()
            self._pending_engine.write_serial(msg)
            self._send_timer.start()

    def closeEvent(self, event):
        # 如果模拟器已经打开了真实串口
        eng = self.device.engine
        if eng.ser and eng.ser.is_open:
            # 清空输入缓冲
            eng.ser.reset_input_buffer()
            # 发送断开命令 'z'
            eng.write_serial("zkkk")
            time.sleep(0.05)

        # 一定要调用父类方法，完成关闭
        super().closeEvent(event)

#=======================================
# Main Entry
#=======================================
if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec_())
