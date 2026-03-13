# Quantum Atomic Structure Simulator - Controls

Welcome to the interactive Quantum Atom Simulator. Below is the complete list of keyboard and mouse controls available to interact with the application.

## ⌨️ Keyboard Controls

| Key | Action | Description |
| :--- | :--- | :--- |
| **0 - 9** | Type Atomic Number | Enter the atomic number (Z) of the element you wish to build (e.g., `1` for Hydrogen, `6` for Carbon, `79` for Gold). |
| **Backspace** | Delete Digit | Deletes the last typed digit in the Atomic Number input box. |
| **Enter** | Build Atom | Applies the typed atomic number and instantly generates the corresponding atom and electron shells. |
| **Spacebar** | Toggle Quantum State | Switches the view between the **Classical State** (Bohr model with solid particles) and the **Quantum State** (Schrödinger wave probability cloud). |
| **F11** | Toggle Fullscreen | Switches the application window in and out of borderless fullscreen mode. |
| **Esc** | Exit | Closes the simulator application. |

## 🖱️ Mouse Controls

### Camera & Navigation (3D View)
* **Right-Click & Drag**: Orbit the camera. Hold down the right mouse button and move the mouse to rotate your view 360 degrees around the atomic core.
* **Scroll Wheel**: Zoom in and out. Scroll up to zoom closer to the nucleus, and scroll down to pull back and see the outer electron shells or broader probability cloud.

### UI Interaction (Quantum State Only)
When the simulator is in the **Quantum State** (toggle using `Spacebar`), an interactive HUD appears on the left side of the screen allowing you to manipulate the mathematical wave function.

* **Left-Click & Drag Sliders**: 
    * **`n` (Principal Quantum Number)**: Defines the overall size and energy level of the orbital (Range: 1 to 7).
    * **`l` (Azimuthal Quantum Number)**: Defines the shape/subshell of the orbital ($s, p, d, f$). Automatically constrained to never exceed $n-1$.
    * **`m` (Magnetic Quantum Number)**: Defines the 3D orientation of the orbital's lobes. Automatically constrained between $-l$ and $+l$.
    * **Color Scale**: Adjusts the visual intensity and brightness mapping of the probability cloud.
    * **Clipping Planes (X, Y, Z)**: Drag these sliders to slice through the 3D probability cloud dynamically. Setting them to `0` cuts a perfect quarter-section out of the atom to view the inner core layers.