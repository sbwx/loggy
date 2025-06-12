# loggy

## Build Instructions
### 1. Set up Zephyr SDK:

Ensure Zephyr SDK and toolchain are installed as per Zephyr's Getting Started Guide: https://docs.zephyrproject.org/latest/develop/getting_started/index.html

### 2. Clone the repository:
```
git clone https://source.eait.uq.edu.au/git/engg3800-2025-sem1/engg3800-g6
cd engg3800-g6
```

### 3. Initialise west workspace:
```
west init -l embedded
west update
west zephyr-export
```

### 4. Set up the virtual environment:
```
source zephyr/zephyr-env.sh
```


### 5. Build the project for STM32L433:
```
west build -b nucleo_l433rc_p embedded/apps/loggy/ --pristine
```

### 6. Flash the sensor board:

Connect the JLink programmer and run:
```
west flash -r jlink
```

### 7. Install STM32CubeIDE:

Install the CubeIDE from their website:
https://www.st.com/en/development-tools/stm32cubeide.html

### 8. Import the project:

- Open the CubeIDE
- File > Open Projects from File System
- Select the root folder:
    ```
    engg3800-g6/embedded/control
    ```

### 9. Build the project:
- Select the project in Project Explorer
- Click the Build icon

### 10. Flash the control board:
- Connect the ST-Link programmer
- Click Run > Run As > STM32 Cortex-M C/C++ Application
