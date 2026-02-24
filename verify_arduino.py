import subprocess
import os
import sys

def run_command(command):
    print(f"Executing: {' '.join(command)}")
    try:
        result = subprocess.run(command, capture_output=True, text=True, check=True)
        print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error during execution:")
        print(e.stdout)
        print(e.stderr)
        return False

def main():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    arduino_cli = os.path.join(base_dir, "bin", "arduino-cli.exe")
    
    if not os.path.exists(arduino_cli):
        print(f"Error: arduino-cli not found at {arduino_cli}")
        print("Please ensure the 'bin' directory exists and contains arduino-cli.exe")
        sys.exit(1)

    fqbn = "arduino:avr:mega:cpu=atmega2560"
    lib_path = os.path.join(base_dir, "arduino", "lib")
    
    # Initialize and install requirements
    print("\n--- Preparing environment ---")
    
    # 1. Update index
    run_command([arduino_cli, "core", "update-index"])
    
    # 2. Install platform if missing
    run_command([arduino_cli, "core", "install", "arduino:avr"])
    
    # 3. Install registry libraries
    run_command([arduino_cli, "lib", "install", "LiquidCrystal"])

    # 4. Check for submodules
    if not os.path.exists(os.path.join(lib_path, "TM1637", "TM1637Display.h")):
        print("Warning: Submodules seem missing. Please run: git submodule update --init --recursive")

    sketches = [
        os.path.join(base_dir, "arduino", "fresca"),
        os.path.join(base_dir, "test")
    ]

    success = True
    for sketch in sketches:
        print(f"\n--- Compiling {os.path.basename(sketch)} ---")
        cmd = [
            arduino_cli, 
            "compile", 
            "--fqbn", fqbn, 
            "--libraries", lib_path, 
            sketch
        ]
        if not run_command(cmd):
            success = False

    if success:
        print("\nAll builds verified successfully!")
    else:
        print("\nSome builds failed verification.")
        sys.exit(1)

if __name__ == "__main__":
    main()
