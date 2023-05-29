<div align="center">

# Offline Graphic Calculator

![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg?style=for-the-badge)
![Build: Passing](https://img.shields.io/badge/build-passing-brightgreen?style=for-the-badge)
![Language: C++](https://img.shields.io/badge/language-c%2B%2B-blue?style=for-the-badge)

The Offline Graphic Calculator is a tool designed to calculate and display mathematical functions in a fast and accurate manner. It has been developed to cater to both simple and advanced users, providing the user with the opportunity to analyze mathematical functions without the need for an internet connection, even on less powerful computers.

[Motivation](#motivation) •
[Installation](#installation) •
[Launching the application](#launching-the-application) •
[Features](#features) •
[Usage](#usage) •
[Roadmap](#roadmap) •
[License](#license)

</div>

---

## Motivation

Nowadays, there are numerous online and local graphic calculators available. However, it can be challenging to find a supported and modern offline graphic calculator that is user-friendly. The motivation behind the Offline Graphic Calculator is to address this issue by offering a reliable and user-friendly tool for mathematical calculations and function visualisation, even without an internet connection.

## Installation

There are two basic methods to install the Offline Graphic Calculator:

### Via Git

1. Check if [Git](https://git-scm.com) is installed on your system.
2. Open the command prompt (cmd) in the desired installation folder using the file explorer.
3. Execute the following command:

```shell
git clone https://github.com/diliny867/Graphical-Calculator.git
```

### From Releases

1. Download the Graphical.Calculator.zip file from [Releases](https://github.com/diliny867/Graphical-Calculator/releases) and extract it.
2. Execute the Graphical Calculator.exe file.

## Launching the application

There are multiple ways to launch the Offline Graphic Calculator app:

### Using Visual Studio

1. Open the project in [Visual Studio](https://visualstudio.microsoft.com/).
2. Load the solution file [Graphical Calculator.sln](https://github.com/diliny867/Graphical-Calculator/blob/master/Graphical%20Calculator.sln).
3. Set up the local Windows Debugger and debug the `.sin` file.

### Using the .exe file

The user can directly open the program by running the [Graphical Calculator.exe](https://github.com/diliny867/Graphical-Calculator/blob/master/Graphical%20Calculator/Graphical%20Calculator.exe) file located in the "Graphical Calculator" folder.

## Features

<details open> <summary> 1. Basic math operators </summary>

| Operator               | Syntax  |
| ---------------------- | ------- |
| Addition               | `x + y` |
| Subtraction            | `x - y` |
| Multiplication         | `x * y` |
| Division               | `x / y` |
| Exponentiation (Power) | `x ^ y` |

</details>

<details open> 
<summary> 2. Advanced math functions </summary>

| Function                | Syntax                       |
| ----------------------- | ---------------------------- |
| Factorial               | `x!`               |
| Square root             | `sqrt(x)`                    |
| Natural logarithm       | `log(x)`                     |
| Trigonometric functions | `sin(x)`, `cos(x)`, `tan(x)` |
| Absolute value          | `abs(x)`                     |
| Round a number          | `round(x)`                   |
| Round down              | `floor(x)`                   |
| Round up                | `ceil(x)`                    |
| Remainder               | `mod(x, y)`                  |

</details>

<details open> 
<summary> 3. Infinite function zoom and movement capabilities. </summary> 
![Point check](attachments/Graphical_Calculator_Scrolling.webp)

</details>

<details open> 
<summary> 4. The ability to check the value of a function at a specific point. </summary> 
![Point check](attachments/Graphical_Calculator_Check_Point.webp)

</details>

<details open> 
<summary> 5. Support for an *infinite* number of functions. </summary> 
![Different functions](attachments/Graphical_Calculator_Functions.webp)

</details>

## Usage

To get started with the Offline Graphic Calculator, follow these steps:

1. Launch the application using one of the methods mentioned above.
2. Once the calculator is open, you will see the calculator menu on the top right of the screen.
3. The majority of the screen is a canvas where functions can be drawn.
4. The top two rows of the calculator menu control the global zoom.
5. On the third row, you can find the parameter for the dot at the cursor.
6. The buttons on the fourth row allow you to add or remove functions from the canvas.
7. To remove a function, click the "-" button located on the left of the function.
8. The "hide/show" button toggles the visibility of the function on the canvas.
9. Clicking the "Variables" button enables you to change the values of custom variables in the function. For example, in a function like "_x \* a_", you can modify the value of "a".
10. The colour picker button allows you to select a colour for the function.
11. The blue field is where you can input your own function.

## Roadmap

The goal of the Offline Graphic Calculator is to provide a lightweight, portable, and user-friendly tool for analyzing mathematical functions. The following enhancements are planned for future development:

- [x] Improve the interface for better usability.

- [x] Expand the range of functions available.

- [ ] Address any interface issues related to window resizing.

- [ ] Add support for non-function graphs, allowing users to plot more complex mathematical concepts.

## License

This project is licensed under the MIT License. See the [LICENSE](https://github.com/diliny867/Graphical-Calculator/blob/master/LICENSE) file for details
