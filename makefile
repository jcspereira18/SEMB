# Variables for source and destination directories
SRC_DIR = src
DST_DIR = /home/pi/Project/src
BUILD_DIR = /home/pi/Project/build
FINAL_DIR = /Project/build

# Raspberry Pi toolchain (cross-compiler or local compiler)
RPI_COMPILER = arm-linux-gnueabihf-gcc  # Adjust this if needed for your toolchain

# Files to copy and compile
FILES = $(wildcard $(SRC_DIR)/*.c)
TARGET = $(BUILD_DIR)/main

# Send the source files to the Raspberry Pi project directory
send:
	@echo "Ensuring target directories exist on Raspberry Pi..."
	ssh pi@192.168.175.94 "mkdir -p $(DST_DIR) $(BUILD_DIR)"
	@echo "Copying files from $(SRC_DIR) to $(DST_DIR)..."
	scp $(FILES) pi@192.168.175.94:/home/pi/Project/src/
	@echo "Files copied successfully."

compile:
	@echo "Compiling source files..." 
	ssh pi@192.168.175.94 "gcc $(DST_DIR)/*.c -o $(BUILD_DIR)/main"
	@echo "Compilation successful."

run:
	@ssh -t pi@192.168.175.94 '.$(FINAL_DIR)/main'
	@echo "Execution finished."

all: send compile run
