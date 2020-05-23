# Service_Robot
Brings you stuff and can find its way to you. Example with the VHDPlus Motor &amp; Level Shifter Extension and NIOSDuino

# Compile
1.1. Clone Repository<br>
1.2. Open the Robot.vhdpproj file with the VHDPlus IDE<br>
1.3. Open Libraries/NIOSDuino/NIOSDuino_Core.qsys<br>
1.4. Click on onchip_flash_0 and disable non-default initialization file<br>
1.5. Compile the project<br>

2.1. Open the NIOS II program tool<br>
2.2. Click on browse and select the Software folder in the project folder<br>
2.3. Click on File -> New -> Nios II Application and BSP from Template<br>
2.4. Select the NIOSDuino_Core.sopcinfo file in the project folder<br>
2.5. Name the project e.g. Robot and click on Finish<br>
2.6. Delete the hello_world.c file and copy the files and folders from the Code folder in the Robot project<br>
2.7. Open the Makefile and edit the following lines:

    ALT_INCLUDE_DIRS := arduino arduino/Wire/src arduino/SPI/src
    ALT_CXXFLAGS := -std=gnu++11
    ALT_CPPFLAGS := -D__AVR__ -DARDUINO=185 -ffunction-sections
    ALT_LDFLAGS := -Wl,--gc-sections
2.8. Right click the Robot_bsp project and click on NIOS II -> BSP Editor...<br>
2.9. Set stderr to none and stdin and stdout to uart_0<br>
2.10. Right click the Robot project and click on Build Project<br>
2.11. Right click the Robot project and click on Make Targets -> Build... -> mem_init_generate -> Build<br>

3.1. Go back to the VHDPlus IDE and open the Libraries/NIOSDuino/NIOSDuino_Core.qsys file<br>
3.2. Click on onchip_flash_0, enable non-default initialization file and select the Software/Robot/mem_init/NIOSDuino_Core_onchip_flash_0.hex file as User created hex or mif file<br>
3.3. Compile the project again<br>
3.4. Program the FPGA<br>

Now you can change the code yourself with the NIOS II program tool. You can test the code without compiling by clicking on Run in Eclipse, or Run -> Run as -> NIOS II Hardware.

After compiling you have to click on Robot_bsp -> NIOS II -> Generate BSP. 
To save your code on the FPGA, click on Build Project and Make Targets again. Then you can compile the VHDPlus project.

